// File: src/core/review_state_update.hpp
// Purpose: Patch review-state.json after reviewed page text saves (selected source, dirty flags).
// Architectural context: HLA-REVIEW, HLA-WORK.
// Requirement references: FR-015, FR-017.
// Test references: TC-FR-015.
// Constraint: Safe messages and rewritten JSON must not embed substantive page text.

#pragma once

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Fields to apply after persisting pages/NNNN.txt for one page.
 */
struct ReviewStatePageSavePatch {
    /** Tracks FR-015 provenance: embedded, manual, ocr, empty. */
    std::string selectedSource;
    /** When true and current status is raw, transitions to editing after first substantive save. */
    bool advanceRawToEditing = true;
};

struct ReviewStateUpdateResult {
    bool success = false;
    std::string safeMessage;
};

/**
 * @brief Updates selectedSource/dirtyState (and optionally raw→editing) for one page entry.
 *
 * @post Does not modify reviewed page text files or source PDFs.
 */
ReviewStateUpdateResult applyReviewStateAfterPageTextSave(
    const std::filesystem::path& workFolder,
    const std::string& pageId,
    const ReviewStatePageSavePatch& patch);

} // namespace pte::core
