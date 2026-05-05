// File: src/core/reviewed_page_text.hpp
// Purpose: Load and save reviewed page text files with stale-write protection.
// Architectural context: HLA-TEXT with HLA-WORK, HLA-APP, HLA-SAFE.
// Requirement references: FR-011, FR-016, FR-022, FR-032, NFR-007, NFR-012.
// Test references: TC-FR-011, TC-FR-016, TC-FR-022, TC-FR-032, TC-NFR-007, TC-NFR-012.
// Constraint: Save/load command messages must not include reviewed page text contents.

#pragma once

#include "core/file_change_detector.hpp"

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Result of loading one reviewed page text file.
 */
struct ReviewedPageLoadResult {
    bool success = false;
    std::string text;
    FileSnapshot snapshot;
    std::string safeMessage;
};

/**
 * @brief Save policy for reviewed page text writes.
 */
struct ReviewedPageSavePolicy {
    FileSnapshot expectedSnapshot;
    bool overwriteExternalChange = false;
};

/**
 * @brief Result of saving one reviewed page text file.
 */
struct ReviewedPageSaveResult {
    bool success = false;
    bool conflict = false;
    FileSnapshot savedSnapshot;
    std::string safeMessage;
};

/**
 * @brief Owns plain reviewed page text file load/save behavior.
 *
 * This service reads and writes only `pages/NNNN.txt` style text artifacts supplied by the caller.
 * It does not parse metadata, update review state, or copy raw OCR/embedded candidates.
 */
class ReviewedPageTextService {
public:
    /**
     * @brief Loads reviewed page text and captures a stale-write snapshot.
     *
     * @param path Local reviewed page text path.
     * @return Text plus snapshot on success, or a safe failure message.
     * @post Failure messages omit file contents.
     * Side effects: Reads local page text file bytes.
     * Determinism: Deterministic for a stable file.
     */
    ReviewedPageLoadResult load(const std::filesystem::path& path) const;

    /**
     * @brief Saves reviewed page text using stale-write conflict protection.
     *
     * @param path Local reviewed page text path.
     * @param text Plain reviewed page text to persist.
     * @param policy Expected snapshot and explicit overwrite choice.
     * @return Save result with conflict status and safe message.
     * @pre text is reviewed page text intended to remain local unless explicitly approved.
     * @post Saved file contains text only, with no metadata front matter added by this service.
     * Side effects: May write the local reviewed page text file atomically.
     * Determinism: Deterministic for stable input and filesystem state.
     */
    ReviewedPageSaveResult save(const std::filesystem::path& path,
                                const std::string& text,
                                const ReviewedPageSavePolicy& policy) const;
};

} // namespace pte::core
