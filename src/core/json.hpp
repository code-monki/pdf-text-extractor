// File: src/core/json.hpp
// Purpose: Provide a small JSON parser for approved work-folder validation.
// Architectural context: HLA-VALID, HLA-WORK.
// Requirement references: FR-025, NFR-011, NFR-012.
// Test references: TC-FR-025, TC-NFR-011, TC-NFR-012.
// Assumption: This parser supports JSON needed for extractor artifacts and reports parse
// errors safely.
// Constraint: Error messages must not include source PDF text or page text contents.

#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace pte::core {

class JsonValue {
public:
    /** @brief JSON array storage used for artifact fields that contain ordered values. */
    using Array = std::vector<JsonValue>;

    /** @brief JSON object storage used for schema-managed extractor artifacts. */
    using Object = std::map<std::string, JsonValue>;

    /**
     * @brief Constructs a null JSON value.
     *
     * @post isNull() returns true.
     */
    JsonValue();

    /**
     * @brief Constructs a null JSON value from nullptr.
     *
     * @post isNull() returns true.
     */
    explicit JsonValue(std::nullptr_t);

    /**
     * @brief Constructs a boolean JSON value.
     *
     * @param value Boolean value to store.
     * @post isBool() returns true.
     */
    explicit JsonValue(bool value);

    /**
     * @brief Constructs a numeric JSON value.
     *
     * @param value Parsed JSON number stored as double precision.
     * @post isNumber() returns true.
     */
    explicit JsonValue(double value);

    /**
     * @brief Constructs a string JSON value.
     *
     * @param value String content to own.
     * @post isString() returns true.
     * @note Callers must avoid exposing source PDF or page text through validation errors.
     */
    explicit JsonValue(std::string value);

    /**
     * @brief Constructs an array JSON value.
     *
     * @param value Array content to own.
     * @post isArray() returns true.
     */
    explicit JsonValue(Array value);

    /**
     * @brief Constructs an object JSON value.
     *
     * @param value Object content to own.
     * @post isObject() returns true.
     */
    explicit JsonValue(Object value);

    /** @brief Returns true when this value is JSON null. */
    bool isNull() const;

    /** @brief Returns true when this value is a JSON boolean. */
    bool isBool() const;

    /** @brief Returns true when this value is a JSON number. */
    bool isNumber() const;

    /** @brief Returns true when this value is a JSON string. */
    bool isString() const;

    /** @brief Returns true when this value is a JSON array. */
    bool isArray() const;

    /** @brief Returns true when this value is a JSON object. */
    bool isObject() const;

    /**
     * @brief Returns the stored string when this value is a JSON string.
     *
     * @return Pointer to the string, or nullptr when the stored type differs.
     */
    const std::string* asString() const;

    /**
     * @brief Returns the stored number when this value is a JSON number.
     *
     * @return Pointer to the number, or nullptr when the stored type differs.
     */
    const double* asNumber() const;

    /**
     * @brief Returns the stored boolean when this value is a JSON boolean.
     *
     * @return Pointer to the boolean, or nullptr when the stored type differs.
     */
    const bool* asBool() const;

    /**
     * @brief Returns the stored array when this value is a JSON array.
     *
     * @return Pointer to the array, or nullptr when the stored type differs.
     */
    const Array* asArray() const;

    /**
     * @brief Returns the stored object when this value is a JSON object.
     *
     * @return Pointer to the object, or nullptr when the stored type differs.
     */
    const Object* asObject() const;

    /**
     * @brief Looks up a key on an object value without throwing.
     *
     * @param key Object key to find.
     * @return Pointer to the matching value, or nullptr when this is not an object or the key
     * is absent.
     * Side effects: None.
     */
    const JsonValue* find(std::string_view key) const;

    /** @brief Mutable object payload for rewriting local JSON artifacts. */
    Object* asObjectForUpdate();

private:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    Type type_ = Type::Null;
    bool boolValue_ = false;
    double numberValue_ = 0.0;
    std::string stringValue_;
    Array arrayValue_;
    Object objectValue_;
};

struct JsonParseResult {
    /** @brief Parsed JSON value when parsing succeeds. */
    std::optional<JsonValue> value;

    /** @brief Safe structural error message that omits surrounding source text. */
    std::string error;

    /** @brief Byte offset where the first parse failure was detected. */
    std::size_t offset = 0;
};

/**
 * @brief Parses JSON text used by local extractor artifacts.
 *
 * @param input JSON document bytes.
 * @return Parsed value on success, or a safe structural error and byte offset on failure.
 * @pre input is expected to be local artifact text, not source PDF page content.
 * @post Failure output does not quote substantive input text.
 * Side effects: None.
 * Determinism: Deterministic for a given input byte sequence.
 */
JsonParseResult parseJson(std::string_view input);

/**
 * @brief Serializes a JSON value to compact UTF-8 text (artifact rewrite helpers).
 *
 * @post Output does not embed substantive page text beyond what is already in the value tree.
 */
std::string formatJsonCompact(const JsonValue& value);

} // namespace pte::core
