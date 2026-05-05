// File: src/core/artifact_cleanup.hpp
// Purpose: Plan and execute user-controlled cleanup of local diagnostic artifacts.
// Architectural context: HLA-WORK, HLA-CONFIG, HLA-SAFE.
// Requirement references: FR-026, FR-030, NFR-002, NFR-007, NFR-012.
// Test references: TC-FR-026, TC-FR-030, TC-NFR-002, TC-NFR-007, TC-NFR-012.
// Constraint: Cleanup never removes reviewed page text unless explicitly authorized.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Cleanup scope for local diagnostic artifacts.
 */
enum class CleanupScope {
    RawCandidates,
    Diagnostics,
    ReviewedText
};

/**
 * @brief User-controlled cleanup request.
 */
struct CleanupRequest {
    std::filesystem::path workFolder;
    std::vector<CleanupScope> scopes;
    bool confirmed = false;
    bool allowReviewedTextRemoval = false;
    bool dryRun = true;
};

/**
 * @brief One cleanup action against a local artifact path.
 */
struct CleanupAction {
    CleanupScope scope = CleanupScope::RawCandidates;
    std::filesystem::path artifactPath;
    bool destructive = true;
};

/**
 * @brief Safe cleanup report.
 */
struct CleanupReport {
    bool success = false;
    bool dryRun = true;
    bool confirmationRequired = false;
    std::vector<CleanupAction> actions;
    std::string safeMessage;
};

/**
 * @brief Plans and optionally executes cleanup of retained local artifacts.
 */
class ArtifactCleanupService {
public:
    /**
     * @brief Plans or executes cleanup for requested artifact scopes.
     *
     * @param request Work folder, cleanup scopes, confirmation, and dry-run options.
     * @return Safe cleanup report with planned or removed artifact paths.
     * @post Reviewed text is not removed unless explicitly authorized and confirmed.
     * Side effects: Removes local diagnostic artifacts only when dryRun is false and confirmed.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    CleanupReport cleanup(const CleanupRequest& request) const;
};

/**
 * @brief Converts cleanup scope to its stable lowercase name.
 *
 * @param scope Cleanup scope.
 * @return Stable scope name.
 */
std::string cleanupScopeName(CleanupScope scope);

} // namespace pte::core
