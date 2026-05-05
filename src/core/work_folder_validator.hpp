// File: src/core/work_folder_validator.hpp
// Purpose: Validate extractor work-folder structural contracts.
// Architectural context: HLA-VALID with HLA-WORK, HLA-META, HLA-REVIEW, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-011, FR-017, FR-018, FR-025, NFR-011, NFR-012.
// Test references: TC-FR-011, TC-FR-017, TC-FR-018, TC-FR-025, TC-NFR-011, TC-NFR-012.
// Constraint: Validation findings must be safe metadata and must not include page text contents.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Severity levels for safe structural validation findings.
 *
 * Blocker and Error findings make a report not OK. Info and Warning findings remain non-blocking.
 */
enum class Severity {
    Info,
    Warning,
    Error,
    Blocker
};

/**
 * @brief Safe validation finding for a local work-folder artifact.
 *
 * Fields intentionally carry structural metadata only: finding code, artifact path/name, page ID,
 * safe message, and repairability. Findings must not include source PDF text or page text contents.
 */
struct ValidationFinding {
    Severity severity = Severity::Info;
    std::string code;
    std::string artifact;
    std::string pageId;
    std::string safeMessage;
    bool repairable = false;
};

/**
 * @brief Accumulates structural validation findings for one work-folder validation run.
 */
struct ValidationReport {
    std::vector<ValidationFinding> findings;

    /**
     * @brief Reports whether validation completed without Error or Blocker findings.
     *
     * @return true when errorCount() is zero.
     * Side effects: None.
     * Determinism: Deterministic for the stored findings.
     */
    bool ok() const;

    /**
     * @brief Counts blocking validation severities.
     *
     * @return Number of findings with Error or Blocker severity.
     * Side effects: None.
     * Determinism: Deterministic for the stored findings.
     */
    int errorCount() const;
};

/**
 * @brief Validates the structural contract of a local extractor work folder.
 *
 * Validation inspects expected JSON artifacts, schema versions, volume identity consistency,
 * page coverage, review statuses, and reviewed page-file presence. It does not read or report
 * substantive page text.
 */
class WorkFolderValidator {
public:
    /**
     * @brief Runs safe structural validation for a work folder.
     *
     * @param workFolder Root directory containing volume.json, page-quality.json,
     * review-state.json, and pages/.
     * @return Validation report containing safe metadata-only findings.
     * @pre workFolder points to a local filesystem path controlled by the operator.
     * @post Report findings omit source PDF text and reviewed page text contents.
     * Side effects: Reads local artifact files and filesystem metadata only.
     * Determinism: Deterministic for a stable filesystem snapshot.
     */
    ValidationReport validate(const std::filesystem::path& workFolder) const;
};

/**
 * @brief Converts a severity enum to its stable lowercase report name.
 *
 * @param severity Severity value to convert.
 * @return Stable textual severity name.
 * Side effects: None.
 * Determinism: Deterministic for a given severity value.
 */
std::string severityName(Severity severity);

} // namespace pte::core
