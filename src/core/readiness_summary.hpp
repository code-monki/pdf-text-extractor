// File: src/core/readiness_summary.hpp
// Purpose: Summarize safe volume readiness counts from local work-folder metadata.
// Architectural context: HLA-DIAG with HLA-VALID, HLA-REVIEW, HLA-SAFE.
// Requirement references: FR-024, FR-027, FR-025, NFR-005, NFR-010, NFR-012.
// Test references: TC-FR-024, TC-FR-027, TC-FR-025, TC-NFR-005, TC-NFR-010, TC-NFR-012.
// Constraint: Readiness summaries must not read or expose reviewed page text contents.

#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace pte::core {

/**
 * @brief Safe readiness counts for one work-folder volume.
 */
struct ReadinessSummary {
    bool success = false;
    std::string safeMessage;
    std::map<std::string, int> statusCounts;
    std::map<std::string, int> selectedSourceCounts;
    std::map<std::string, int> flagCounts;
    int ocrUsedCount = 0;
    int validationErrorCount = 0;
};

/**
 * @brief Builds safe readiness summaries from metadata artifacts.
 */
class ReadinessSummaryService {
public:
    /**
     * @brief Summarizes review and diagnostic readiness for one work folder.
     *
     * @param workFolder Work-folder root containing review-state.json and page-quality.json.
     * @return Safe readiness summary with counts and validation error count.
     * @post Summary does not include reviewed page text or source PDF text.
     * Side effects: Reads JSON metadata artifacts and filesystem metadata for validation only.
     * Determinism: Deterministic for a stable work-folder snapshot.
     */
    ReadinessSummary summarizeWorkFolder(const std::filesystem::path& workFolder) const;
};

} // namespace pte::core
