// File: src/core/artifact_cleanup.cpp
// Purpose: Implement dry-run and confirmed cleanup of local diagnostic artifacts.
// Architectural context: HLA-WORK, HLA-CONFIG, HLA-SAFE.
// Requirement references: FR-026, FR-030, NFR-002, NFR-007, NFR-012.
// Test references: TC-FR-026, TC-FR-030, TC-NFR-002, TC-NFR-007, TC-NFR-012.
// Constraint: Reports include paths and scopes only, never artifact file contents.

#include "core/artifact_cleanup.hpp"

#include <filesystem>

namespace pte::core {

namespace {

/**
 * @brief Adds existing files/directories under a path to a cleanup report.
 */
void addExisting(CleanupReport& report,
                 CleanupScope scope,
                 const std::filesystem::path& path) {
    std::error_code error;
    if (std::filesystem::exists(path, error) && !error) {
        report.actions.push_back(CleanupAction{scope, path, true});
    }
}

/**
 * @brief Adds cleanup actions for a requested scope.
 */
void addScopeActions(CleanupReport& report,
                     const std::filesystem::path& workFolder,
                     CleanupScope scope) {
    switch (scope) {
    case CleanupScope::RawCandidates:
        addExisting(report, scope, workFolder / "raw" / "embedded");
        addExisting(report, scope, workFolder / "raw" / "ocr");
        break;
    case CleanupScope::Diagnostics:
        addExisting(report, scope, workFolder / "diagnostics");
        break;
    case CleanupScope::ReviewedText:
        addExisting(report, scope, workFolder / "pages");
        break;
    }
}

} // namespace

/**
 * @brief Converts cleanup scope to a stable diagnostic name.
 */
std::string cleanupScopeName(CleanupScope scope) {
    switch (scope) {
    case CleanupScope::RawCandidates: return "raw-candidates";
    case CleanupScope::Diagnostics: return "diagnostics";
    case CleanupScope::ReviewedText: return "reviewed-text";
    }
    return "unknown";
}

/**
 * @brief Plans or executes cleanup according to confirmation and dry-run policy.
 */
CleanupReport ArtifactCleanupService::cleanup(const CleanupRequest& request) const {
    CleanupReport report;
    report.dryRun = request.dryRun;
    if (request.workFolder.empty()) {
        report.safeMessage = "work folder path is required";
        return report;
    }

    bool reviewedTextRequested = false;
    for (const auto scope : request.scopes) {
        addScopeActions(report, request.workFolder, scope);
        if (scope == CleanupScope::ReviewedText) {
            reviewedTextRequested = true;
        }
    }

    if (reviewedTextRequested && !request.allowReviewedTextRemoval) {
        report.confirmationRequired = true;
        report.safeMessage = "reviewed text cleanup requires explicit authorization";
        return report;
    }

    if (!request.dryRun && !request.confirmed) {
        report.confirmationRequired = true;
        report.safeMessage = "cleanup requires explicit confirmation";
        return report;
    }

    if (request.dryRun) {
        report.success = true;
        report.safeMessage = "cleanup planned";
        return report;
    }

    for (const auto& action : report.actions) {
        std::error_code error;
        std::filesystem::remove_all(action.artifactPath, error);
        if (error) {
            report.safeMessage = "cleanup failed while removing artifact";
            return report;
        }
    }

    report.success = true;
    report.safeMessage = "cleanup completed";
    return report;
}

} // namespace pte::core
