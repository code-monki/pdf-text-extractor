// File: src/core/page_review_sync.hpp
// Purpose: Load per-page review + volume label fields for UI sync (NFR-006).
// Architectural context: HLA-REVIEW, HLA-META, HLA-WORK.
// Requirement references: FR-017, FR-019, FR-020, NFR-005, NFR-006.
// Test references: TC-NFR-006.
// Constraint: Does not read reviewed page text files; JSON metadata only.

#pragma once

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Non-text metadata for one page used to keep review UI aligned with artifacts.
 */
struct PageReviewSyncSnapshot {
    bool loaded = false;
    std::string safeMessage;
    std::string reviewStatus;
    std::string selectedSource;
    std::string dirtyState;
    std::string printedPageLabel;
    std::string volumeTitle;
};

/**
 * @brief Reads review-state.json and volume.json for one page ID under a work folder.
 *
 * @param workFolder Work-folder root (volume.json, review-state.json).
 * @param pageId Zero-padded page identifier (e.g. "0003").
 */
PageReviewSyncSnapshot loadPageReviewSyncSnapshot(const std::filesystem::path& workFolder,
                                                  const std::string& pageId);

} // namespace pte::core
