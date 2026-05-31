// File: src/core/link_map_store.cpp

#include "core/link_map_store.hpp"

#include "core/json.hpp"

#include <fstream>
#include <sstream>

namespace pte::core {
namespace {

std::optional<std::string> readUtf8File(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool writeUtf8File(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << text;
    return static_cast<bool>(out);
}

bool parseRect(const JsonValue* rect, double out[4]) {
    if (rect == nullptr || !rect->isArray() || rect->asArray()->size() != 4) {
        return false;
    }
    for (std::size_t i = 0; i < 4; ++i) {
        const JsonValue& coord = rect->asArray()->at(i);
        if (!coord.isNumber()) {
            return false;
        }
        out[i] = *coord.asNumber();
    }
    return true;
}

JsonValue::Object makeIntraTarget(const LinkMapEntry& entry) {
    JsonValue::Object target;
    target["type"] = JsonValue(std::string("intra"));
    if (!entry.destinationId.empty()) {
        target["destinationId"] = JsonValue(entry.destinationId);
        return target;
    }
    JsonValue::Object destination;
    destination["pageIndex"] = JsonValue(static_cast<double>(entry.destinationPageIndex));
    destination["yPt"] = JsonValue(entry.destinationYPt);
    target["destination"] = JsonValue(std::move(destination));
    return target;
}

JsonValue::Object makeUrlTarget(const std::string& url) {
    JsonValue::Object target;
    target["type"] = JsonValue(std::string("url"));
    target["url"] = JsonValue(url);
    return target;
}

} // namespace

std::optional<std::filesystem::path> resolveLinkMapPath(
    const std::filesystem::path& workFolder) {
    std::error_code error;
    const std::filesystem::path primary = workFolder / "link-map.json";
    if (std::filesystem::is_regular_file(primary, error) && !error) {
        return primary;
    }
    const std::filesystem::path nested = workFolder / "enrichment" / "link-map.json";
    if (std::filesystem::is_regular_file(nested, error) && !error) {
        return nested;
    }
    return workFolder / "link-map.json";
}

LinkMapStoreResult loadLinkMapDocument(const std::filesystem::path& path) {
    LinkMapStoreResult result;
    if (path.empty()) {
        result.safeMessage = "link-map path is empty";
        return result;
    }
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        result.success = true;
        result.safeMessage = "link-map will be created on save";
        return result;
    }
    const auto text = readUtf8File(path);
    if (!text.has_value()) {
        result.safeMessage = "link-map not readable";
        return result;
    }
    const JsonParseResult parsed = parseJson(*text);
    if (!parsed.value.has_value()) {
        result.safeMessage = "link-map JSON parse failed";
        return result;
    }

    const JsonValue* schemaVersion = parsed.value->find("schemaVersion");
    if (schemaVersion == nullptr || !schemaVersion->isNumber()
        || static_cast<int>(*schemaVersion->asNumber()) != 1) {
        result.safeMessage = "link-map schemaVersion must be 1";
        return result;
    }

    const JsonValue* links = parsed.value->find("links");
    if (links == nullptr || !links->isArray()) {
        result.safeMessage = "link-map missing links array";
        return result;
    }

    for (const JsonValue& link : *links->asArray()) {
        if (link.asObject() == nullptr) {
            result.safeMessage = "link-map link must be object";
            return result;
        }
        LinkMapEntry entry;
        const JsonValue* pageIndex = link.find("pageIndex");
        const JsonValue* rect = link.find("rect");
        const JsonValue* target = link.find("target");
        if (pageIndex == nullptr || !pageIndex->isNumber() || !parseRect(rect, entry.rect)
            || target == nullptr || !target->isObject()) {
            result.safeMessage = "link-map link missing required fields";
            return result;
        }
        entry.pageIndex = static_cast<int>(*pageIndex->asNumber());
        const JsonValue* type = target->find("type");
        if (type == nullptr || !type->isString()) {
            result.safeMessage = "link-map target missing type";
            return result;
        }
        entry.targetType = *type->asString();
        if (entry.targetType == "intra") {
            const JsonValue* destinationId = target->find("destinationId");
            if (destinationId != nullptr && destinationId->isString()) {
                entry.destinationId = *destinationId->asString();
            }
            const JsonValue* destination = target->find("destination");
            if (destination != nullptr && destination->isObject()) {
                const JsonValue* destPage = destination->find("pageIndex");
                const JsonValue* destY = destination->find("yPt");
                if (destPage != nullptr && destPage->isNumber()) {
                    entry.destinationPageIndex = static_cast<int>(*destPage->asNumber());
                }
                if (destY != nullptr && destY->isNumber()) {
                    entry.destinationYPt = *destY->asNumber();
                }
            }
        } else if (entry.targetType == "url") {
            const JsonValue* url = target->find("url");
            if (url != nullptr && url->isString()) {
                entry.url = *url->asString();
            }
        }
        const JsonValue* manual = link.find("manual");
        if (manual != nullptr && manual->isBool()) {
            entry.manual = *manual->asBool();
        }
        result.document.links.push_back(entry);
    }

    result.success = true;
    result.safeMessage = "link-map loaded";
    return result;
}

LinkMapStoreResult saveLinkMapDocument(const std::filesystem::path& path,
                                       const LinkMapDocument& document) {
    LinkMapStoreResult result;
    if (path.empty()) {
        result.safeMessage = "link-map path is empty";
        return result;
    }

    JsonValue::Object root;
    root["schemaVersion"] = JsonValue(1.0);
    JsonValue::Array links;
    for (const LinkMapEntry& entry : document.links) {
        JsonValue::Object link;
        link["pageIndex"] = JsonValue(static_cast<double>(entry.pageIndex));
        JsonValue::Array rect;
        for (double value : entry.rect) {
            rect.emplace_back(JsonValue(value));
        }
        link["rect"] = JsonValue(std::move(rect));
        link["manual"] = JsonValue(entry.manual);
        if (entry.targetType == "url") {
            link["target"] = JsonValue(makeUrlTarget(entry.url));
        } else {
            link["target"] = JsonValue(makeIntraTarget(entry));
        }
        links.emplace_back(JsonValue(std::move(link)));
    }
    root["links"] = JsonValue(std::move(links));

    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (!writeUtf8File(path, formatJsonCompact(JsonValue(std::move(root))))) {
        result.safeMessage = "failed to write link-map";
        return result;
    }

    result.success = true;
    result.document = document;
    result.safeMessage = "link-map saved";
    return result;
}

} // namespace pte::core
