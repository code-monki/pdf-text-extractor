// File: src/core/work_folder_initializer.cpp
// Purpose: Implement deterministic local work-folder initialization.
// Architectural context: HLA-WORK, HLA-META, HLA-REVIEW, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-004, FR-005, FR-009, FR-011, FR-017, FR-024, NFR-011.
// Test references: TC-FR-004, TC-FR-005, TC-FR-009, TC-FR-011, TC-FR-017, TC-NFR-011.
// Assumption: Source PDF identity and page count are supplied by inventory or tests.
// Constraint: Initialization writes local artifacts only and must never modify source PDFs.

#include "core/work_folder_initializer.hpp"

#include "core/page_id.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace pte::core {

namespace {

/**
 * @brief Escapes a string for JSON artifact output.
 *
 * @param value UTF-8 string value to encode.
 * @return JSON string literal including surrounding quotes.
 */
std::string jsonString(const std::string& value) {
    std::string output = "\"";
    for (const char c : value) {
        switch (c) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b"; break;
        case '\f': output += "\\f"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                output += "\\u00";
                const char* hex = "0123456789abcdef";
                output.push_back(hex[(static_cast<unsigned char>(c) >> 4) & 0x0f]);
                output.push_back(hex[static_cast<unsigned char>(c) & 0x0f]);
            } else {
                output.push_back(c);
            }
            break;
        }
    }
    output += "\"";
    return output;
}

/**
 * @brief Writes text through a sibling temporary file and rename.
 *
 * @param path Final artifact path.
 * @param content Content to write.
 * @return true when the write and rename complete.
 * Side effects: Creates parent directories, writes a temp file, and renames it into place.
 */
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
 * @brief Builds the initial volume.json document.
 *
 * @param request Initialization request.
 * @return JSON document string.
 */
std::string buildVolumeJson(const WorkFolderInitRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schemaVersion\": 1,\n";
    out << "  \"volumeId\": " << jsonString(request.volumeId) << ",\n";
    out << "  \"title\": " << jsonString(request.title) << ",\n";
    out << "  \"subtitle\": " << jsonString(request.subtitle) << ",\n";
    out << "  \"sortTitle\": " << jsonString(request.sortTitle) << ",\n";
    out << "  \"group\": " << jsonString(request.group) << ",\n";
    out << "  \"bibliographic\": {\n";
    out << "    \"publisher\": \"\",\n";
    out << "    \"publicationYear\": null,\n";
    out << "    \"originalPublicationYear\": null,\n";
    out << "    \"edition\": \"\",\n";
    out << "    \"language\": \"\",\n";
    out << "    \"isbn\": \"\",\n";
    out << "    \"productCode\": \"\",\n";
    out << "    \"creators\": [],\n";
    out << "    \"series\": \"\",\n";
    out << "    \"seriesNumber\": \"\",\n";
    out << "    \"description\": \"\",\n";
    out << "    \"subjects\": [],\n";
    out << "    \"rights\": \"\"\n";
    out << "  },\n";
    out << "  \"sourcePdf\": {\n";
    out << "    \"path\": " << jsonString(request.sourcePdf.path.string()) << ",\n";
    out << "    \"filename\": " << jsonString(request.sourcePdf.filename) << ",\n";
    out << "    \"pageCount\": " << request.sourcePdf.pageCount << "\n";
    out << "  },\n";
    out << "  \"indexing\": {\n";
    out << "    \"includeByDefault\": true\n";
    out << "  },\n";
    out << "  \"collection\": {},\n";
    out << "  \"cover\": {\n";
    out << "    \"pageId\": null,\n";
    out << "    \"pdfPage\": null\n";
    out << "  },\n";
    out << "  \"pageMap\": {\n";
    for (int pdfPage = 1; pdfPage <= request.sourcePdf.pageCount; ++pdfPage) {
        const auto pageId = pageIdForPdfPage(pdfPage);
        out << "    " << jsonString(pageId) << ": {\n";
        out << "      \"pageId\": " << jsonString(pageId) << ",\n";
        out << "      \"pdfPage\": " << pdfPage << ",\n";
        out << "      \"pageTextPath\": " << jsonString("pages/" + pageId + ".txt") << ",\n";
        out << "      \"printedPageLabel\": \"\",\n";
        out << "      \"includeByDefault\": true\n";
        out << "    }";
        if (pdfPage != request.sourcePdf.pageCount) {
            out << ",";
        }
        out << "\n";
    }
    out << "  },\n";
    out << "  \"notes\": \"\"\n";
    out << "}\n";
    return out.str();
}

/**
 * @brief Builds the initial review-state.json document.
 *
 * @param request Initialization request.
 * @return JSON document string.
 */
std::string buildReviewStateJson(const WorkFolderInitRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schemaVersion\": 1,\n";
    out << "  \"volumeId\": " << jsonString(request.volumeId) << ",\n";
    out << "  \"pages\": {\n";
    for (int pdfPage = 1; pdfPage <= request.sourcePdf.pageCount; ++pdfPage) {
        const auto pageId = pageIdForPdfPage(pdfPage);
        out << "    " << jsonString(pageId) << ": {\n";
        out << "      \"pdfPage\": " << pdfPage << ",\n";
        out << "      \"status\": \"raw\",\n";
        out << "      \"selectedSource\": \"empty\",\n";
        out << "      \"reviewedAt\": null,\n";
        out << "      \"reviewedBy\": null,\n";
        out << "      \"notes\": \"\",\n";
        out << "      \"dirtyState\": \"clean\"\n";
        out << "    }";
        if (pdfPage != request.sourcePdf.pageCount) {
            out << ",";
        }
        out << "\n";
    }
    out << "  }\n";
    out << "}\n";
    return out.str();
}

/**
 * @brief Builds the initial page-quality.json document.
 *
 * @param request Initialization request.
 * @return JSON document string.
 */
std::string buildPageQualityJson(const WorkFolderInitRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schemaVersion\": 1,\n";
    out << "  \"volumeId\": " << jsonString(request.volumeId) << ",\n";
    out << "  \"pages\": {\n";
    for (int pdfPage = 1; pdfPage <= request.sourcePdf.pageCount; ++pdfPage) {
        const auto pageId = pageIdForPdfPage(pdfPage);
        out << "    " << jsonString(pageId) << ": {\n";
        out << "      \"pdfPage\": " << pdfPage << ",\n";
        out << "      \"selectedSource\": \"empty\",\n";
        out << "      \"embedded\": {\n";
        out << "        \"available\": false,\n";
        out << "        \"charCount\": 0,\n";
        out << "        \"qualityScore\": null,\n";
        out << "        \"toolInfo\": null,\n";
        out << "        \"modePolicy\": \"not-run\"\n";
        out << "      },\n";
        out << "      \"ocr\": {\n";
        out << "        \"available\": false,\n";
        out << "        \"charCount\": 0,\n";
        out << "        \"qualityScore\": null,\n";
        out << "        \"toolInfo\": null,\n";
        out << "        \"modePolicy\": \"not-run\"\n";
        out << "      },\n";
        out << "      \"flags\": [\"empty\"],\n";
        out << "      \"errors\": []\n";
        out << "    }";
        if (pdfPage != request.sourcePdf.pageCount) {
            out << ",";
        }
        out << "\n";
    }
    out << "  }\n";
    out << "}\n";
    return out.str();
}

} // namespace

/**
 * @brief Initializes the expected local work-folder layout and empty reviewed page files.
 */
WorkFolderInitResult WorkFolderInitializer::initialize(
    const WorkFolderInitRequest& request) const {
    if (request.workFolder.empty()) {
        return {false, "work folder path is required"};
    }
    if (request.volumeId.empty()) {
        return {false, "volumeId is required"};
    }
    if (request.sourcePdf.pageCount <= 0) {
        return {false, "sourcePdf.pageCount must be positive"};
    }

    try {
        std::filesystem::create_directories(request.workFolder / "pages");
        std::filesystem::create_directories(request.workFolder / "raw" / "embedded");
        std::filesystem::create_directories(request.workFolder / "raw" / "ocr");

        for (int pdfPage = 1; pdfPage <= request.sourcePdf.pageCount; ++pdfPage) {
            const auto pageId = pageIdForPdfPage(pdfPage);
            const auto pagePath = request.workFolder / "pages" / (pageId + ".txt");
            std::error_code existsError;
            if (std::filesystem::exists(pagePath, existsError) && !existsError) {
                continue;
            }
            if (!writeTextAtomic(pagePath, "")) {
                return {false, "unable to write reviewed page text file"};
            }
        }

        if (!writeTextAtomic(request.workFolder / "volume.json", buildVolumeJson(request))) {
            return {false, "unable to write volume.json"};
        }
        if (!writeTextAtomic(request.workFolder / "review-state.json",
                             buildReviewStateJson(request))) {
            return {false, "unable to write review-state.json"};
        }
        if (!writeTextAtomic(request.workFolder / "page-quality.json",
                             buildPageQualityJson(request))) {
            return {false, "unable to write page-quality.json"};
        }
    } catch (const std::filesystem::filesystem_error&) {
        return {false, "filesystem error while initializing work folder"};
    }

    return {true, "work folder initialized"};
}

} // namespace pte::core
