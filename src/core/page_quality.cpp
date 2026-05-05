// File: src/core/page_quality.cpp
// Purpose: Implement serialization of safe page-quality diagnostics.
// Architectural context: HLA-DIAG, HLA-EXT, HLA-OCR, HLA-WORK, HLA-SAFE.
// Requirement references: FR-012, FR-013, FR-014, FR-024, NFR-004, NFR-005,
// NFR-010, NFR-012.
// Test references: TC-FR-012, TC-FR-013, TC-FR-014, TC-FR-024, TC-NFR-004,
// TC-NFR-005, TC-NFR-010, TC-NFR-012, TC-BND-OCR-004, TC-BND-OCR-005.
// Constraint: Serialization omits substantive candidate and reviewed page text.

#include "core/page_quality.hpp"

#include "core/page_id.hpp"

#include <filesystem>
#include <fstream>
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
 * @brief Writes one string array.
 */
void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values) {
    out << "[";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            out << ", ";
        }
        out << jsonString(values[index]);
    }
    out << "]";
}

/**
 * @brief Writes one candidate diagnostics object.
 */
void writeCandidate(std::ostringstream& out, const CandidateDiagnostics& diagnostics) {
    out << "{\n";
    out << "        \"available\": " << (diagnostics.available ? "true" : "false") << ",\n";
    out << "        \"charCount\": " << diagnostics.charCount << ",\n";
    out << "        \"qualityScore\": null,\n";
    if (diagnostics.toolInfo.empty()) {
        out << "        \"toolInfo\": null,\n";
    } else {
        out << "        \"toolInfo\": " << jsonString(diagnostics.toolInfo) << ",\n";
    }
    out << "        \"modePolicy\": " << jsonString(diagnostics.modePolicy) << "\n";
    out << "      }";
}

/**
 * @brief Writes text atomically through a sibling temporary file.
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

} // namespace

/**
 * @brief Serializes a complete page-quality.json document.
 */
PageQualityWriteResult PageQualityService::writePageQuality(
    const std::filesystem::path& workFolder,
    const std::string& volumeId,
    const std::vector<PageQualityEntry>& entries) const {
    if (workFolder.empty()) {
        return {false, "work folder path is required"};
    }
    if (volumeId.empty()) {
        return {false, "volumeId is required"};
    }

    for (const auto& entry : entries) {
        if (!isValidPageId(entry.pageId) || entry.pdfPage <= 0) {
            return {false, "page-quality entry has invalid page identity"};
        }
    }

    std::ostringstream out;
    out << "{\n";
    out << "  \"schemaVersion\": 1,\n";
    out << "  \"volumeId\": " << jsonString(volumeId) << ",\n";
    out << "  \"pages\": {\n";
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        out << "    " << jsonString(entry.pageId) << ": {\n";
        out << "      \"pdfPage\": " << entry.pdfPage << ",\n";
        out << "      \"selectedSource\": " << jsonString(entry.selectedSource) << ",\n";
        out << "      \"embedded\": ";
        writeCandidate(out, entry.embedded);
        out << ",\n";
        out << "      \"ocr\": ";
        writeCandidate(out, entry.ocr);
        out << ",\n";
        out << "      \"flags\": ";
        writeStringArray(out, entry.flags);
        out << ",\n";
        out << "      \"errors\": ";
        writeStringArray(out, entry.errors);
        out << "\n";
        out << "    }";
        if (index + 1 != entries.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  }\n";
    out << "}\n";

    try {
        if (!writeTextAtomic(workFolder / "page-quality.json", out.str())) {
            return {false, "unable to write page-quality.json"};
        }
    } catch (const std::filesystem::filesystem_error&) {
        return {false, "filesystem error while writing page-quality.json"};
    }
    return {true, "page-quality diagnostics written"};
}

} // namespace pte::core
