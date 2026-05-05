// File: src/core/candidate_text.hpp
// Purpose: Manage raw embedded/OCR candidate text artifacts and explicit restore actions.
// Architectural context: HLA-WORK, HLA-EXT, HLA-OCR, HLA-TEXT, HLA-SAFE.
// Requirement references: FR-012, FR-013, FR-015, FR-016, FR-022, FR-024, NFR-012.
// Test references: TC-FR-012, TC-FR-013, TC-FR-015, TC-FR-016, TC-FR-022,
// TC-FR-024, TC-NFR-012, TC-BND-OCR-002, TC-BND-OCR-005.
// Constraint: Raw candidates are local restricted artifacts and must not self-authorize review.

#pragma once

#include "core/file_change_detector.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Supported raw candidate text sources.
 *
 * Embedded and OCR candidates are diagnostic inputs. They are not review statuses and cannot
 * directly make a page release eligible.
 */
enum class CandidateSource {
    Embedded,
    Ocr
};

/**
 * @brief Save policy for explicitly restoring reviewed text from a raw candidate.
 */
struct CandidateRestorePolicy {
    FileSnapshot expectedSnapshot;
    bool allowOverwrite = false;
};

/**
 * @brief Result for raw candidate load/save operations.
 */
struct CandidateTextResult {
    bool success = false;
    std::string text;
    std::size_t charCount = 0;
    std::string safeMessage;
};

/**
 * @brief Availability and size metadata for one candidate source.
 */
struct CandidateSourceSummary {
    CandidateSource source = CandidateSource::Embedded;
    bool available = false;
    std::size_t charCount = 0;
};

/**
 * @brief Metadata-only comparison of reviewed text and raw candidate files.
 */
struct CandidateComparison {
    bool success = false;
    std::string pageId;
    std::size_t reviewedCharCount = 0;
    CandidateSourceSummary embedded;
    CandidateSourceSummary ocr;
    double embeddedOcrDeltaRatio = 0.0;
    std::vector<std::string> flags;
    std::string safeMessage;
};

/**
 * @brief Result for explicit restore-from-candidate workflow actions.
 */
struct CandidateRestoreResult {
    bool success = false;
    bool conflict = false;
    std::size_t restoredCharCount = 0;
    std::string safeMessage;
};

/**
 * @brief Service for raw candidate storage, metadata comparison, and explicit restore.
 *
 * The service owns no PDF or OCR tooling. It only handles candidate text after an adapter has
 * produced it and keeps candidate files separate from `pages/NNNN.txt` until restore is requested.
 */
class CandidateTextService {
public:
    /**
     * @brief Stores a raw candidate file under `raw/embedded` or `raw/ocr`.
     *
     * @param workFolder Root work-folder path.
     * @param pageId Stable page ID such as `0001`.
     * @param source Candidate source to store.
     * @param text Raw candidate text to write.
     * @return Success result with character count and safe diagnostic message.
     * @pre pageId must be a valid page ID.
     * @post Reviewed page text and review status are unchanged.
     * Side effects: Writes one local raw candidate text file atomically.
     * Determinism: Deterministic for stable inputs and filesystem state.
     */
    CandidateTextResult storeCandidate(const std::filesystem::path& workFolder,
                                       const std::string& pageId,
                                       CandidateSource source,
                                       const std::string& text) const;

    /**
     * @brief Loads a raw candidate file from local work-folder storage.
     *
     * @param workFolder Root work-folder path.
     * @param pageId Stable page ID such as `0001`.
     * @param source Candidate source to load.
     * @return Candidate text, character count, and safe diagnostic status.
     * @pre pageId must be a valid page ID.
     * @post Failure messages omit candidate text.
     * Side effects: Reads one local raw candidate text file.
     * Determinism: Deterministic for stable filesystem state.
     */
    CandidateTextResult loadCandidate(const std::filesystem::path& workFolder,
                                      const std::string& pageId,
                                      CandidateSource source) const;

    /**
     * @brief Builds a metadata-only comparison for reviewed, embedded, and OCR text.
     *
     * @param workFolder Root work-folder path.
     * @param pageId Stable page ID such as `0001`.
     * @return Character counts, availability, delta ratio, and safe flags.
     * @post Result does not include reviewed or candidate text content.
     * Side effects: Reads local reviewed and raw candidate files.
     * Determinism: Deterministic for stable filesystem state.
     */
    CandidateComparison comparePage(const std::filesystem::path& workFolder,
                                    const std::string& pageId) const;

    /**
     * @brief Explicitly replaces reviewed page text with a raw candidate.
     *
     * @param workFolder Root work-folder path.
     * @param pageId Stable page ID such as `0001`.
     * @param source Candidate source to restore from.
     * @param policy Stale-write policy for the reviewed page file.
     * @return Restore result with conflict status and safe diagnostic message.
     * @pre Candidate file must already exist.
     * @post Review status remains unchanged; caller must update metadata through review workflow.
     * Side effects: Reads one raw candidate file and may write `pages/NNNN.txt`.
     * Determinism: Deterministic for stable inputs and filesystem state.
     */
    CandidateRestoreResult restoreReviewedTextFromCandidate(
        const std::filesystem::path& workFolder,
        const std::string& pageId,
        CandidateSource source,
        const CandidateRestorePolicy& policy) const;
};

/**
 * @brief Converts a candidate source to its stable lowercase artifact name.
 *
 * @param source Candidate source enum.
 * @return `embedded` or `ocr`.
 */
std::string candidateSourceName(CandidateSource source);

} // namespace pte::core
