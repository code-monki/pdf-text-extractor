// File: src/core/json.cpp
// Purpose: Implement safe JSON parsing for local extraction artifact validation.
// Architectural context: HLA-VALID, HLA-WORK.
// Requirement references: FR-025, NFR-011, NFR-012.
// Test references: TC-FR-025, TC-NFR-011, TC-NFR-012.
// Assumption: JSON artifacts are UTF-8 text; semantic UTF-8 validation is handled by later
// text services.
// Constraint: Parse errors identify structure and offset only, never surrounding document text.

#include "core/json.hpp"

#include <cctype>
#include <charconv>
#include <sstream>
#include <utility>

namespace pte::core {

/** @brief Constructs a null JSON value for absent or explicit null fields. */
JsonValue::JsonValue() = default;

/** @brief Constructs a null JSON value from nullptr. */
JsonValue::JsonValue(std::nullptr_t) {}

/** @brief Constructs a JSON boolean value. */
JsonValue::JsonValue(bool value) : type_(Type::Bool), boolValue_(value) {}

/** @brief Constructs a JSON number value. */
JsonValue::JsonValue(double value) : type_(Type::Number), numberValue_(value) {}

/** @brief Constructs a JSON string value by taking ownership of value. */
JsonValue::JsonValue(std::string value)
    : type_(Type::String), stringValue_(std::move(value)) {}

/** @brief Constructs a JSON array value by taking ownership of value. */
JsonValue::JsonValue(Array value) : type_(Type::Array), arrayValue_(std::move(value)) {}

/** @brief Constructs a JSON object value by taking ownership of value. */
JsonValue::JsonValue(Object value) : type_(Type::Object), objectValue_(std::move(value)) {}

/** @brief Returns true when the stored value is JSON null. */
bool JsonValue::isNull() const { return type_ == Type::Null; }

/** @brief Returns true when the stored value is a JSON boolean. */
bool JsonValue::isBool() const { return type_ == Type::Bool; }

/** @brief Returns true when the stored value is a JSON number. */
bool JsonValue::isNumber() const { return type_ == Type::Number; }

/** @brief Returns true when the stored value is a JSON string. */
bool JsonValue::isString() const { return type_ == Type::String; }

/** @brief Returns true when the stored value is a JSON array. */
bool JsonValue::isArray() const { return type_ == Type::Array; }

/** @brief Returns true when the stored value is a JSON object. */
bool JsonValue::isObject() const { return type_ == Type::Object; }

/** @brief Returns a string pointer when the stored type is string, otherwise nullptr. */
const std::string* JsonValue::asString() const {
    return isString() ? &stringValue_ : nullptr;
}

/** @brief Returns a number pointer when the stored type is number, otherwise nullptr. */
const double* JsonValue::asNumber() const {
    return isNumber() ? &numberValue_ : nullptr;
}

/** @brief Returns a boolean pointer when the stored type is boolean, otherwise nullptr. */
const bool* JsonValue::asBool() const {
    return isBool() ? &boolValue_ : nullptr;
}

/** @brief Returns an array pointer when the stored type is array, otherwise nullptr. */
const JsonValue::Array* JsonValue::asArray() const {
    return isArray() ? &arrayValue_ : nullptr;
}

/** @brief Returns an object pointer when the stored type is object, otherwise nullptr. */
const JsonValue::Object* JsonValue::asObject() const {
    return isObject() ? &objectValue_ : nullptr;
}

/**
 * @brief Finds an object member without throwing or creating entries.
 *
 * @return Pointer to the existing member, or nullptr when the receiver is not an object or key
 * is absent.
 */
const JsonValue* JsonValue::find(std::string_view key) const {
    const auto* object = asObject();
    if (!object) {
        return nullptr;
    }
    const auto found = object->find(std::string(key));
    if (found == object->end()) {
        return nullptr;
    }
    return &found->second;
}

JsonValue::Object* JsonValue::asObjectForUpdate() {
    return isObject() ? &objectValue_ : nullptr;
}

namespace {

/**
 * @brief Recursive-descent parser for the repository's JSON artifact subset.
 *
 * The parser records only structural error messages and byte offsets. It deliberately avoids
 * including input excerpts in errors so restricted page text cannot leak through diagnostics.
 */
class Parser {
public:
    /**
     * @brief Creates a parser over an immutable input view.
     *
     * @param input JSON bytes supplied by a local artifact reader.
     */
    explicit Parser(std::string_view input) : input_(input) {}

    /**
     * @brief Parses one complete JSON document.
     *
     * @return Parsed value, or the first safe structural failure.
     * @post Trailing non-whitespace input is reported as an error.
     */
    JsonParseResult parse() {
        skipWhitespace();
        auto value = parseValue();
        if (!value) {
            return failure_;
        }
        skipWhitespace();
        if (pos_ != input_.size()) {
            return fail("unexpected trailing content");
        }
        JsonParseResult result;
        result.value = std::move(value);
        return result;
    }

private:
    /**
     * @brief Parses any JSON value at the current position.
     *
     * @return Parsed value, or std::nullopt after failure_ is populated.
     */
    std::optional<JsonValue> parseValue() {
        skipWhitespace();
        if (pos_ >= input_.size()) {
            fail("unexpected end of input");
            return std::nullopt;
        }
        const char c = input_[pos_];
        if (c == '{') {
            return parseObject();
        }
        if (c == '[') {
            return parseArray();
        }
        if (c == '"') {
            auto value = parseString();
            if (!value) {
                return std::nullopt;
            }
            return JsonValue(std::move(*value));
        }
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            return parseNumber();
        }
        if (consumeLiteral("true")) {
            return JsonValue(true);
        }
        if (consumeLiteral("false")) {
            return JsonValue(false);
        }
        if (consumeLiteral("null")) {
            return JsonValue(nullptr);
        }
        fail("unexpected token");
        return std::nullopt;
    }

    /**
     * @brief Parses a JSON object and its member values.
     *
     * @return Object value, or std::nullopt after a structural failure.
     * @post Duplicate keys keep the first parsed value because Object::emplace does not overwrite.
     */
    std::optional<JsonValue> parseObject() {
        ++pos_;
        JsonValue::Object object;
        skipWhitespace();
        if (consume('}')) {
            return JsonValue(std::move(object));
        }
        while (pos_ < input_.size()) {
            skipWhitespace();
            auto key = parseString();
            if (!key) {
                return std::nullopt;
            }
            skipWhitespace();
            if (!consume(':')) {
                fail("expected object separator");
                return std::nullopt;
            }
            auto value = parseValue();
            if (!value) {
                return std::nullopt;
            }
            object.emplace(std::move(*key), std::move(*value));
            skipWhitespace();
            if (consume('}')) {
                return JsonValue(std::move(object));
            }
            if (!consume(',')) {
                fail("expected object comma or close");
                return std::nullopt;
            }
        }
        fail("unterminated object");
        return std::nullopt;
    }

    /**
     * @brief Parses a JSON array and its ordered values.
     *
     * @return Array value, or std::nullopt after a structural failure.
     */
    std::optional<JsonValue> parseArray() {
        ++pos_;
        JsonValue::Array array;
        skipWhitespace();
        if (consume(']')) {
            return JsonValue(std::move(array));
        }
        while (pos_ < input_.size()) {
            auto value = parseValue();
            if (!value) {
                return std::nullopt;
            }
            array.push_back(std::move(*value));
            skipWhitespace();
            if (consume(']')) {
                return JsonValue(std::move(array));
            }
            if (!consume(',')) {
                fail("expected array comma or close");
                return std::nullopt;
            }
        }
        fail("unterminated array");
        return std::nullopt;
    }

    /**
     * @brief Parses a JSON string.
     *
     * Unicode escapes are preserved in escaped form for this implementation slice because current
     * validation needs structural metadata, not semantic Unicode normalization.
     *
     * @return Parsed string, or std::nullopt after a structural failure.
     */
    std::optional<std::string> parseString() {
        if (!consume('"')) {
            fail("expected string");
            return std::nullopt;
        }
        std::string output;
        while (pos_ < input_.size()) {
            const char c = input_[pos_++];
            if (c == '"') {
                return output;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                fail("unescaped control character in string");
                return std::nullopt;
            }
            if (c != '\\') {
                output.push_back(c);
                continue;
            }
            if (pos_ >= input_.size()) {
                fail("unterminated escape sequence");
                return std::nullopt;
            }
            const char escaped = input_[pos_++];
            switch (escaped) {
            case '"': output.push_back('"'); break;
            case '\\': output.push_back('\\'); break;
            case '/': output.push_back('/'); break;
            case 'b': output.push_back('\b'); break;
            case 'f': output.push_back('\f'); break;
            case 'n': output.push_back('\n'); break;
            case 'r': output.push_back('\r'); break;
            case 't': output.push_back('\t'); break;
            case 'u':
                if (pos_ + 4 > input_.size()) {
                    fail("invalid unicode escape");
                    return std::nullopt;
                }
                output.append("\\u");
                output.append(input_.substr(pos_, 4));
                pos_ += 4;
                break;
            default:
                fail("invalid escape sequence");
                return std::nullopt;
            }
        }
        fail("unterminated string");
        return std::nullopt;
    }

    /**
     * @brief Parses a JSON number into double precision storage.
     *
     * @return Numeric JSON value, or std::nullopt when lexical or conversion validation fails.
     */
    std::optional<JsonValue> parseNumber() {
        const std::size_t start = pos_;
        if (input_[pos_] == '-') {
            ++pos_;
        }
        if (pos_ >= input_.size()) {
            fail("invalid number");
            return std::nullopt;
        }
        if (input_[pos_] == '0') {
            ++pos_;
        } else if (std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        } else {
            fail("invalid number");
            return std::nullopt;
        }
        if (pos_ < input_.size() && input_[pos_] == '.') {
            ++pos_;
            if (pos_ >= input_.size() || !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                fail("invalid fractional number");
                return std::nullopt;
            }
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        }
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) {
                ++pos_;
            }
            if (pos_ >= input_.size() || !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                fail("invalid exponent");
                return std::nullopt;
            }
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        }

        const auto token = input_.substr(start, pos_ - start);
        double parsed = 0.0;
        const auto* first = token.data();
        const auto* last = token.data() + token.size();
        const auto result = std::from_chars(first, last, parsed);
        if (result.ec != std::errc() || result.ptr != last) {
            fail("invalid number conversion");
            return std::nullopt;
        }
        return JsonValue(parsed);
    }

    /** @brief Advances past JSON whitespace characters without side effects. */
    void skipWhitespace() {
        while (pos_ < input_.size()) {
            const char c = input_[pos_];
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                return;
            }
            ++pos_;
        }
    }

    /**
     * @brief Consumes one expected character when it appears at the current position.
     *
     * @return true when the character was consumed.
     */
    bool consume(char expected) {
        if (pos_ < input_.size() && input_[pos_] == expected) {
            ++pos_;
            return true;
        }
        return false;
    }

    /**
     * @brief Consumes an expected literal token at the current position.
     *
     * @return true when the full literal was consumed.
     */
    bool consumeLiteral(std::string_view literal) {
        if (input_.substr(pos_, literal.size()) == literal) {
            pos_ += literal.size();
            return true;
        }
        return false;
    }

    /**
     * @brief Records the first parser failure using a safe structural message.
     *
     * Later failures are ignored so callers receive the earliest actionable offset.
     *
     * @return Current failure state.
     */
    JsonParseResult fail(std::string message) {
        if (failure_.error.empty()) {
            failure_.error = std::move(message);
            failure_.offset = pos_;
        }
        return failure_;
    }

    std::string_view input_;
    std::size_t pos_ = 0;
    JsonParseResult failure_;
};

} // namespace

namespace {

void appendEscapedString(std::string& out, std::string_view value) {
    out.push_back('"');
    for (const char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                out += "\\u00";
                const char* hex = "0123456789abcdef";
                out.push_back(hex[(static_cast<unsigned char>(c) >> 4) & 0x0f]);
                out.push_back(hex[static_cast<unsigned char>(c) & 0x0f]);
            } else {
                out.push_back(c);
            }
            break;
        }
    }
    out.push_back('"');
}

void appendNumber(std::string& out, double n) {
    std::ostringstream stream;
    stream.setf(std::ios::fmtflags(0), std::ios::floatfield);
    stream.precision(17);
    stream << n;
    out += stream.str();
}

void appendJsonValue(std::string& out, const JsonValue& value) {
    if (value.isNull()) {
        out += "null";
        return;
    }
    if (value.isBool()) {
        out += *value.asBool() ? "true" : "false";
        return;
    }
    if (value.isNumber()) {
        appendNumber(out, *value.asNumber());
        return;
    }
    if (value.isString()) {
        appendEscapedString(out, *value.asString());
        return;
    }
    if (value.isArray()) {
        out.push_back('[');
        const auto* array = value.asArray();
        bool first = true;
        for (const JsonValue& element : *array) {
            if (!first) {
                out.push_back(',');
            }
            first = false;
            appendJsonValue(out, element);
        }
        out.push_back(']');
        return;
    }
    if (value.isObject()) {
        out.push_back('{');
        const auto* object = value.asObject();
        bool first = true;
        for (const auto& member : *object) {
            if (!first) {
                out.push_back(',');
            }
            first = false;
            appendEscapedString(out, member.first);
            out.push_back(':');
            appendJsonValue(out, member.second);
        }
        out.push_back('}');
        return;
    }
}

} // namespace

/**
 * @brief Parses JSON artifact text and returns a safe result object.
 *
 * @param input JSON document bytes.
 * @return Parsed value, or structural error metadata on failure.
 */
JsonParseResult parseJson(std::string_view input) {
    return Parser(input).parse();
}

std::string formatJsonCompact(const JsonValue& value) {
    std::string out;
    appendJsonValue(out, value);
    return out;
}

} // namespace pte::core
