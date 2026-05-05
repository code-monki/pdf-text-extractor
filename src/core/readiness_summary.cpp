// File: src/core/readiness_summary.cpp
// Purpose: Implement safe readiness aggregation from review and diagnostic metadata.
// Architectural context: HLA-DIAG with HLA-VALID, HLA-REVIEW, HLA-SAFE.
// Requirement references: FR-024, FR-027, FR-025, NFR-005, NFR-010, NFR-012.
// Test references: TC-FR-024, TC-FR-027, TC-FR-025, TC-NFR-005, TC-NFR-010, TC-NFR-012.
// Assumption: Full report export formatting belongs to a later HLA-DIAG slice.
// Constraint: Readiness summaries must not read or expose reviewed page text contents.

#include "core/readiness_summary.hpp"

#include "core/json.hpp"
#include "core/work_folder_validator.hpp"

#include <fstream>
#include <sstream>

namespace pte::core {

namespace {

/**
 * @brief Reads a local metadata artifact into memory.
 *
 * @param path JSON artifact path.
 * @param output Destination content.
 * @return true when the file is readable.
 * Side effects: Reads local JSON metadata file bytes.
 */
bool readText(const std::filesystem::path& path, std::string& output) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    output = buffer.str();
    return true;
}

/**
 * @brief Parses a JSON metadata object.
 *
 * @param path JSON artifact path.
 * @param output Parsed root object.
 * @return true when the artifact exists and parses to an object.
 * Side effects: Reads local JSON metadata file bytes.
 */
bool parseObjectFile(const std::filesystem::path& path, JsonValue& output) {
    std::string content;
    if (!readText(path, content)) {
        return false;
    }
    const auto parsed = parseJson(content);
    if (!parsed.value || !parsed.value->isObject()) {
        return false;
    }
    output = *parsed.value;
    return true;
}

/**
 * @brief Returns a string field from an object value.
 *
 * @param value Object value.
 * @param key Field key.
 * @return String value, or empty string when absent/wrong type.
 */
std::string stringField(const JsonValue& value, std::string_view key) {
    const auto* field = value.find(key);
    if (!field) {
        return {};
    }
    const auto* text = field->asString();
    return text ? *text : std::string();
}

/**
 * @brief Counts review statuses from review-state pages.
 *
 * @param reviewRoot Parsed review-state root object.
 * @param summary Summary receiving counts.
 */
void countReviewStatuses(const JsonValue& reviewRoot, ReadinessSummary& summary) {
    const auto* pagesValue = reviewRoot.find("pages");
    const auto* pages = pagesValue ? pagesValue->asObject() : nullptr;
    if (!pages) {
        return;
    }
    for (const auto& [pageId, entry] : *pages) {
        (void)pageId;
        const auto status = stringField(entry, "status");
        if (!status.empty()) {
            ++summary.statusCounts[status];
        }
    }
}

/**
 * @brief Counts selected sources and quality flags from page-quality pages.
 *
 * @param qualityRoot Parsed page-quality root object.
 * @param summary Summary receiving counts.
 */
void countQualityDiagnostics(const JsonValue& qualityRoot, ReadinessSummary& summary) {
    const auto* pagesValue = qualityRoot.find("pages");
    const auto* pages = pagesValue ? pagesValue->asObject() : nullptr;
    if (!pages) {
        return;
    }

    for (const auto& [pageId, entry] : *pages) {
        (void)pageId;
        const auto selectedSource = stringField(entry, "selectedSource");
        if (!selectedSource.empty()) {
            ++summary.selectedSourceCounts[selectedSource];
        }

        const auto* flagsValue = entry.find("flags");
        const auto* flags = flagsValue ? flagsValue->asArray() : nullptr;
        if (!flags) {
            continue;
        }
        for (const auto& flagValue : *flags) {
            const auto* flag = flagValue.asString();
            if (!flag || flag->empty()) {
                continue;
            }
            ++summary.flagCounts[*flag];
            if (*flag == "ocr-used") {
                ++summary.ocrUsedCount;
            }
        }
    }
}

} // namespace

/**
 * @brief Summarizes safe readiness counts from review-state and page-quality artifacts.
 */
ReadinessSummary ReadinessSummaryService::summarizeWorkFolder(
    const std::filesystem::path& workFolder) const {
    ReadinessSummary summary;
    summary.validationErrorCount = WorkFolderValidator().validate(workFolder).errorCount();

    JsonValue reviewRoot;
    JsonValue qualityRoot;
    if (!parseObjectFile(workFolder / "review-state.json", reviewRoot)) {
        summary.safeMessage = "review-state.json could not be summarized";
        return summary;
    }
    if (!parseObjectFile(workFolder / "page-quality.json", qualityRoot)) {
        summary.safeMessage = "page-quality.json could not be summarized";
        return summary;
    }

    countReviewStatuses(reviewRoot, summary);
    countQualityDiagnostics(qualityRoot, summary);
    summary.success = true;
    summary.safeMessage = "readiness summary created";
    return summary;
}

} // namespace pte::core
