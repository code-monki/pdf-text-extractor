// File: src/core/volume_metadata.cpp
// Purpose: Implement deterministic volume metadata serialization and edit helpers.
// Architectural context: HLA-META, HLA-WORK, HLA-SAFE.
// Requirement references: FR-005, FR-006, FR-007, FR-008, FR-009, FR-010,
// NFR-005, NFR-008, NFR-011, NFR-012.
// Test references: TC-FR-005, TC-FR-006, TC-FR-007, TC-FR-008, TC-FR-009,
// TC-FR-010, TC-NFR-005, TC-NFR-008, TC-NFR-011, TC-NFR-012.
// Constraint: Metadata serialization must not embed reviewed page text contents.

#include "core/volume_metadata.hpp"

#include "core/json.hpp"
#include "core/page_id.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace pte::core {

namespace {

/**
 * @brief Escapes a string for JSON output.
 */
std::string jsonString(const std::string& value) {
    std::string output = "\"";
    for (const char c : value) {
        switch (c) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default: output.push_back(c); break;
        }
    }
    output += "\"";
    return output;
}

/**
 * @brief Writes text atomically through a sibling temporary file.
 */
std::optional<std::string> readFileUtf8(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string stringField(const JsonValue& object, std::string_view key) {
    const auto* field = object.find(key);
    if (!field || field->isNull()) {
        return {};
    }
    const auto* text = field->asString();
    return text ? *text : std::string{};
}

std::optional<int> intField(const JsonValue& object, std::string_view key) {
    const auto* field = object.find(key);
    if (!field || !field->isNumber()) {
        return std::nullopt;
    }
    const auto* num = field->asNumber();
    if (!num) {
        return std::nullopt;
    }
    return static_cast<int>(*num);
}

bool boolFieldDefault(const JsonValue& object, std::string_view key, bool defaultValue) {
    const auto* field = object.find(key);
    if (!field) {
        return defaultValue;
    }
    const auto* b = field->asBool();
    return b ? *b : defaultValue;
}

bool writeTextAtomic(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    const auto temp = path.string() + ".tmp";
    {
        std::ofstream output(temp, std::ios::binary | std::ios::trunc);
        if (!output) {
            return false;
        }
        output << content;
        if (!output) {
            return false;
        }
    }
    std::filesystem::rename(temp, path);
    return true;
}

/**
 * @brief Finds a page-map entry by page ID.
 */
std::vector<PageMapEntry>::const_iterator findPage(const VolumeMetadata& metadata,
                                                   const std::string& pageId) {
    for (auto iter = metadata.pageMap.begin(); iter != metadata.pageMap.end(); ++iter) {
        if (iter->pageId == pageId) {
            return iter;
        }
    }
    return metadata.pageMap.end();
}

/**
 * @brief Validates minimal metadata invariants before serialization.
 */
std::string validateMetadata(const VolumeMetadata& metadata) {
    if (metadata.volumeId.empty()) {
        return "volumeId is required";
    }
    if (metadata.sourcePageCount <= 0) {
        return "source page count must be positive";
    }
    if (metadata.pageMap.empty()) {
        return "page map is required";
    }
    for (const auto& entry : metadata.pageMap) {
        if (!isValidPageId(entry.pageId) || entry.pdfPage <= 0 || entry.pageTextPath.empty()) {
            return "page map entry is invalid";
        }
    }
    return {};
}

/**
 * @brief Fills a VolumeMetadata model from a parsed volume.json root object.
 *
 * @return Empty string on success, otherwise a safe diagnostic message.
 */
std::string parseVolumeMetadataDocument(const JsonValue& root, VolumeMetadata& out) {
    if (!root.isObject()) {
        return "volume.json root must be a JSON object";
    }

    out = VolumeMetadata{};

    const auto schemaVersion = intField(root, "schemaVersion");
    if (!schemaVersion || *schemaVersion != 1) {
        return "unsupported volume.json schemaVersion (expected 1)";
    }

    out.volumeId = stringField(root, "volumeId");
    out.title = stringField(root, "title");
    out.subtitle = stringField(root, "subtitle");
    out.sortTitle = stringField(root, "sortTitle");
    out.group = stringField(root, "group");
    out.notes = stringField(root, "notes");

    const auto* bibRoot = root.find("bibliographic");
    if (bibRoot != nullptr && !bibRoot->isNull()) {
        const auto* bibObj = bibRoot->asObject();
        if (!bibObj) {
            return "bibliographic must be an object";
        }
        for (const auto& [key, value] : *bibObj) {
            const auto* str = value.asString();
            if (str != nullptr) {
                out.bibliographic[key] = *str;
            }
        }
    }

    const auto* sourceField = root.find("sourcePdf");
    if (sourceField == nullptr) {
        return "volume.json missing sourcePdf";
    }
    if (!sourceField->isObject()) {
        return "sourcePdf must be an object";
    }
    out.sourcePath = stringField(*sourceField, "path");
    out.sourceFilename = stringField(*sourceField, "filename");
    const auto pageCount = intField(*sourceField, "pageCount");
    if (!pageCount || *pageCount <= 0) {
        return "sourcePdf.pageCount must be a positive integer";
    }
    out.sourcePageCount = *pageCount;

    const auto* indexingField = root.find("indexing");
    if (indexingField != nullptr && !indexingField->isNull()) {
        out.includeByDefault = boolFieldDefault(*indexingField, "includeByDefault", true);
    }

    const auto* coverField = root.find("cover");
    if (coverField != nullptr && !coverField->isNull()) {
        const auto* coverObj = coverField->asObject();
        if (!coverObj) {
            return "cover must be an object";
        }
        const std::string coverPageId = stringField(*coverField, "pageId");
        if (!coverPageId.empty()) {
            out.cover.pageId = coverPageId;
        }
        const auto coverPdfPage = intField(*coverField, "pdfPage");
        if (coverPdfPage.has_value()) {
            out.cover.pdfPage = *coverPdfPage;
        }
    }

    const auto* pageMapField = root.find("pageMap");
    if (pageMapField == nullptr) {
        return "volume.json missing pageMap";
    }
    const auto* pageMapObj = pageMapField->asObject();
    if (!pageMapObj) {
        return "pageMap must be an object";
    }

    for (const auto& [pageId, entryValue] : *pageMapObj) {
        if (!isValidPageId(pageId)) {
            return "pageMap contains an invalid page ID";
        }
        if (!entryValue.isObject()) {
            return "pageMap entry must be an object";
        }
        PageMapEntry entry;
        entry.pageId = pageId;
        const auto pdfPage = intField(entryValue, "pdfPage");
        if (!pdfPage || *pdfPage <= 0) {
            return "pageMap pdfPage must be a positive integer";
        }
        entry.pdfPage = *pdfPage;

        std::string textRel = stringField(entryValue, "pageTextPath");
        if (textRel.empty()) {
            textRel = stringField(entryValue, "textPath");
        }
        if (textRel.empty()) {
            return "pageMap entry missing pageTextPath";
        }
        entry.pageTextPath = std::move(textRel);
        entry.printedPageLabel = stringField(entryValue, "printedPageLabel");
        entry.includeByDefault = boolFieldDefault(entryValue, "includeByDefault", true);
        out.pageMap.push_back(std::move(entry));
    }

    std::sort(out.pageMap.begin(), out.pageMap.end(), [](const PageMapEntry& a, const PageMapEntry& b) {
        if (a.pdfPage != b.pdfPage) {
            return a.pdfPage < b.pdfPage;
        }
        return a.pageId < b.pageId;
    });

    return {};
}

/**
 * @brief Serializes the metadata model as volume.json.
 */
std::string buildVolumeJson(const VolumeMetadata& metadata) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schemaVersion\": 1,\n";
    out << "  \"volumeId\": " << jsonString(metadata.volumeId) << ",\n";
    out << "  \"title\": " << jsonString(metadata.title) << ",\n";
    out << "  \"subtitle\": " << jsonString(metadata.subtitle) << ",\n";
    out << "  \"sortTitle\": " << jsonString(metadata.sortTitle) << ",\n";
    out << "  \"group\": " << jsonString(metadata.group) << ",\n";
    out << "  \"bibliographic\": {\n";
    std::size_t bibIndex = 0;
    for (const auto& [key, value] : metadata.bibliographic) {
        out << "    " << jsonString(key) << ": " << jsonString(value);
        if (++bibIndex != metadata.bibliographic.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  },\n";
    out << "  \"sourcePdf\": {\n";
    out << "    \"path\": " << jsonString(metadata.sourcePath) << ",\n";
    out << "    \"filename\": " << jsonString(metadata.sourceFilename) << ",\n";
    out << "    \"pageCount\": " << metadata.sourcePageCount << "\n";
    out << "  },\n";
    out << "  \"indexing\": {\n";
    out << "    \"includeByDefault\": "
        << (metadata.includeByDefault ? "true" : "false") << "\n";
    out << "  },\n";
    out << "  \"cover\": {\n";
    out << "    \"pageId\": ";
    if (metadata.cover.pageId) {
        out << jsonString(*metadata.cover.pageId);
    } else {
        out << "null";
    }
    out << ",\n";
    out << "    \"pdfPage\": ";
    if (metadata.cover.pdfPage) {
        out << *metadata.cover.pdfPage;
    } else {
        out << "null";
    }
    out << "\n";
    out << "  },\n";
    out << "  \"pageMap\": {\n";
    for (std::size_t index = 0; index < metadata.pageMap.size(); ++index) {
        const auto& entry = metadata.pageMap[index];
        out << "    " << jsonString(entry.pageId) << ": {\n";
        out << "      \"pageId\": " << jsonString(entry.pageId) << ",\n";
        out << "      \"pdfPage\": " << entry.pdfPage << ",\n";
        out << "      \"pageTextPath\": " << jsonString(entry.pageTextPath) << ",\n";
        out << "      \"printedPageLabel\": " << jsonString(entry.printedPageLabel) << ",\n";
        out << "      \"includeByDefault\": "
            << (entry.includeByDefault ? "true" : "false") << "\n";
        out << "    }";
        if (index + 1 != metadata.pageMap.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  },\n";
    out << "  \"notes\": " << jsonString(metadata.notes) << "\n";
    out << "}\n";
    return out.str();
}

} // namespace

/**
 * @brief Writes volume metadata after validating stable identity and page-map fields.
 */
VolumeMetadataResult VolumeMetadataService::writeVolumeMetadata(
    const std::filesystem::path& workFolder,
    const VolumeMetadata& metadata) const {
    VolumeMetadataResult result;
    result.metadata = metadata;
    const auto validation = validateMetadata(metadata);
    if (!validation.empty()) {
        result.safeMessage = validation;
        return result;
    }

    try {
        if (!writeTextAtomic(workFolder / "volume.json", buildVolumeJson(metadata))) {
            result.safeMessage = "unable to write volume.json";
            return result;
        }
    } catch (const std::filesystem::filesystem_error&) {
        result.safeMessage = "filesystem error while writing volume.json";
        return result;
    }

    result.success = true;
    result.safeMessage = "volume metadata written";
    return result;
}

VolumeMetadataResult VolumeMetadataService::loadVolumeMetadata(
    const std::filesystem::path& workFolder) const {
    VolumeMetadataResult result;
    const auto path = workFolder / "volume.json";
    const auto fileBytes = readFileUtf8(path);
    if (!fileBytes) {
        result.safeMessage = "volume.json is missing or unreadable";
        return result;
    }

    const auto parsed = parseJson(*fileBytes);
    if (!parsed.value) {
        result.safeMessage = "volume.json is not valid JSON";
        return result;
    }

    VolumeMetadata metadata;
    const std::string parseError = parseVolumeMetadataDocument(*parsed.value, metadata);
    if (!parseError.empty()) {
        result.safeMessage = parseError;
        return result;
    }

    const std::string validation = validateMetadata(metadata);
    if (!validation.empty()) {
        result.safeMessage = validation;
        return result;
    }

    result.success = true;
    result.metadata = std::move(metadata);
    result.safeMessage = "volume metadata loaded";
    return result;
}

/**
 * @brief Updates a printed page label in memory.
 */
VolumeMetadataResult VolumeMetadataService::setPrintedPageLabel(
    const VolumeMetadata& metadata,
    const std::string& pageId,
    const std::string& label) const {
    VolumeMetadataResult result;
    result.metadata = metadata;
    auto page = result.metadata.pageMap.end();
    for (auto iter = result.metadata.pageMap.begin(); iter != result.metadata.pageMap.end();
         ++iter) {
        if (iter->pageId == pageId) {
            page = iter;
            break;
        }
    }
    if (page == result.metadata.pageMap.end()) {
        result.safeMessage = "page map entry not found";
        return result;
    }
    page->printedPageLabel = label;
    result.success = true;
    result.safeMessage = "printed page label updated";
    return result;
}

/**
 * @brief Updates or clears cover page metadata in memory.
 */
VolumeMetadataResult VolumeMetadataService::setCoverPage(
    const VolumeMetadata& metadata,
    const std::optional<std::string>& pageId) const {
    VolumeMetadataResult result;
    result.metadata = metadata;
    if (!pageId) {
        result.metadata.cover = {};
        result.success = true;
        result.safeMessage = "cover page cleared";
        return result;
    }

    const auto page = findPage(metadata, *pageId);
    if (page == metadata.pageMap.end()) {
        result.safeMessage = "cover page is not in page map";
        return result;
    }

    result.metadata.cover.pageId = *pageId;
    result.metadata.cover.pdfPage = page->pdfPage;
    result.success = true;
    result.safeMessage = "cover page updated";
    return result;
}

} // namespace pte::core
