// File: src/core/candidate_text.cpp
// Purpose: Implement raw candidate artifact storage and explicit reviewed-text restoration.
// Architectural context: HLA-WORK, HLA-EXT, HLA-OCR, HLA-TEXT, HLA-SAFE.
// Requirement references: FR-012, FR-013, FR-015, FR-016, FR-022, FR-024, NFR-012.
// Test references: TC-FR-012, TC-FR-013, TC-FR-015, TC-FR-016, TC-FR-022,
// TC-FR-024, TC-NFR-012, TC-BND-OCR-002, TC-BND-OCR-005.
// Constraint: Candidate operations report metadata and safe messages without exposing text.

#include "core/candidate_text.hpp"

#include "core/page_id.hpp"
#include "core/reviewed_page_text.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace pte::core {

namespace {

constexpr double suspiciousDeltaThreshold = 0.05;

/**
 * @brief Resolves the raw candidate path for a page/source pair.
 *
 * @param workFolder Root work-folder path.
 * @param pageId Stable page ID.
 * @param source Candidate source.
 * @return Local candidate artifact path.
 */
std::filesystem::path candidatePath(const std::filesystem::path& workFolder,
                                    const std::string& pageId,
                                    CandidateSource source) {
    return workFolder / "raw" / candidateSourceName(source) / (pageId + ".txt");
}

/**
 * @brief Resolves the reviewed page text path for a page.
 *
 * @param workFolder Root work-folder path.
 * @param pageId Stable page ID.
 * @return Local reviewed text artifact path.
 */
std::filesystem::path reviewedPath(const std::filesystem::path& workFolder,
                                   const std::string& pageId) {
    return workFolder / "pages" / (pageId + ".txt");
}

/**
 * @brief Reads a local text file into memory.
 *
 * @param path Local artifact path.
 * @param output Destination string.
 * @return true when the file can be read.
 * Side effects: Reads local file bytes.
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
 * @brief Writes text through a temporary sibling file and rename.
 *
 * @param path Final artifact path.
 * @param text Text content to write.
 * @return true when the write and rename complete.
 * Side effects: Creates directories, writes temp file, and renames it into place.
 */
bool writeTextAtomic(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    const auto temp = path.string() + ".tmp";
    {
        std::ofstream output(temp, std::ios::binary | std::ios::trunc);
        if (!output) {
            return false;
        }
        output << text;
        if (!output) {
            return false;
        }
    }
    std::filesystem::rename(temp, path);
    return true;
}

/**
 * @brief Counts UTF-8 character starts without validating or reporting text content.
 *
 * @param text Local candidate or reviewed text.
 * @return Approximate Unicode character count for valid UTF-8; byte count fallback for ASCII.
 */
std::size_t textCharCount(const std::string& text) {
    std::size_t count = 0;
    for (const unsigned char c : text) {
        if ((c & 0xc0) != 0x80) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Computes the absolute proportional difference between two counts.
 *
 * @param first First character count.
 * @param second Second character count.
 * @return Delta ratio relative to the larger nonzero count.
 */
double deltaRatio(std::size_t first, std::size_t second) {
    const auto larger = std::max(first, second);
    if (larger == 0) {
        return 0.0;
    }
    const auto smaller = std::min(first, second);
    return static_cast<double>(larger - smaller) / static_cast<double>(larger);
}

} // namespace

/**
 * @brief Converts a candidate source enum to its storage directory name.
 */
std::string candidateSourceName(CandidateSource source) {
    switch (source) {
    case CandidateSource::Embedded: return "embedded";
    case CandidateSource::Ocr: return "ocr";
    }
    return "unknown";
}

/**
 * @brief Stores a raw candidate while leaving reviewed text untouched.
 */
CandidateTextResult CandidateTextService::storeCandidate(
    const std::filesystem::path& workFolder,
    const std::string& pageId,
    CandidateSource source,
    const std::string& text) const {
    if (!isValidPageId(pageId)) {
        return {false, "", 0, "candidate pageId is invalid"};
    }

    try {
        if (!writeTextAtomic(candidatePath(workFolder, pageId, source), text)) {
            return {false, "", 0, "unable to write raw candidate"};
        }
    } catch (const std::filesystem::filesystem_error&) {
        return {false, "", 0, "filesystem error while writing raw candidate"};
    }

    return {true, "", textCharCount(text), "raw candidate stored"};
}

/**
 * @brief Loads a raw candidate text file from the work folder.
 */
CandidateTextResult CandidateTextService::loadCandidate(
    const std::filesystem::path& workFolder,
    const std::string& pageId,
    CandidateSource source) const {
    if (!isValidPageId(pageId)) {
        return {false, "", 0, "candidate pageId is invalid"};
    }

    std::string text;
    if (!readText(candidatePath(workFolder, pageId, source), text)) {
        return {false, "", 0, "raw candidate is unavailable"};
    }
    return {true, text, textCharCount(text), "raw candidate loaded"};
}

/**
 * @brief Compares reviewed text and raw candidates using metadata only.
 */
CandidateComparison CandidateTextService::comparePage(
    const std::filesystem::path& workFolder,
    const std::string& pageId) const {
    CandidateComparison comparison;
    comparison.pageId = pageId;
    comparison.embedded.source = CandidateSource::Embedded;
    comparison.ocr.source = CandidateSource::Ocr;

    if (!isValidPageId(pageId)) {
        comparison.safeMessage = "candidate pageId is invalid";
        return comparison;
    }

    std::string reviewed;
    if (!readText(reviewedPath(workFolder, pageId), reviewed)) {
        comparison.safeMessage = "reviewed page text is unavailable";
        return comparison;
    }
    comparison.reviewedCharCount = textCharCount(reviewed);

    const auto embedded = loadCandidate(workFolder, pageId, CandidateSource::Embedded);
    comparison.embedded.available = embedded.success;
    comparison.embedded.charCount = embedded.charCount;

    const auto ocr = loadCandidate(workFolder, pageId, CandidateSource::Ocr);
    comparison.ocr.available = ocr.success;
    comparison.ocr.charCount = ocr.charCount;

    if (comparison.embedded.available && comparison.ocr.available) {
        comparison.embeddedOcrDeltaRatio = deltaRatio(comparison.embedded.charCount,
                                                      comparison.ocr.charCount);
        if (comparison.embeddedOcrDeltaRatio > suspiciousDeltaThreshold) {
            comparison.flags.push_back("embedded-ocr-delta-suspicious");
        }
    }

    comparison.success = true;
    comparison.safeMessage = "candidate comparison created";
    return comparison;
}

/**
 * @brief Explicitly restores reviewed text from a selected raw candidate.
 */
CandidateRestoreResult CandidateTextService::restoreReviewedTextFromCandidate(
    const std::filesystem::path& workFolder,
    const std::string& pageId,
    CandidateSource source,
    const CandidateRestorePolicy& policy) const {
    const auto candidate = loadCandidate(workFolder, pageId, source);
    if (!candidate.success) {
        return {false, false, 0, candidate.safeMessage};
    }

    const ReviewedPageSavePolicy savePolicy{policy.expectedSnapshot, policy.allowOverwrite};
    const auto saved = ReviewedPageTextService().save(reviewedPath(workFolder, pageId),
                                                      candidate.text,
                                                      savePolicy);
    if (!saved.success) {
        return {false, saved.conflict, 0, saved.safeMessage};
    }

    return {true, false, candidate.charCount, "reviewed text restored from raw candidate"};
}

} // namespace pte::core
