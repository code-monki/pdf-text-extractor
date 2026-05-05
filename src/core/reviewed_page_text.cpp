// File: src/core/reviewed_page_text.cpp
// Purpose: Implement reviewed page text load/save behavior.
// Architectural context: HLA-TEXT with HLA-WORK, HLA-APP, HLA-SAFE.
// Requirement references: FR-011, FR-016, FR-022, FR-032, NFR-007, NFR-012.
// Test references: TC-FR-011, TC-FR-016, TC-FR-022, TC-FR-032, TC-NFR-007, TC-NFR-012.
// Assumption: Text normalization profiles will be added in a later HLA-TEXT slice.
// Constraint: Save/load command messages must not include reviewed page text contents.

#include "core/reviewed_page_text.hpp"

#include <filesystem>
#include <fstream>

namespace pte::core {

namespace {

/**
 * @brief Reads an entire local text file.
 *
 * @param path File path to read.
 * @param output Destination text.
 * @return true when the file is fully read.
 * Side effects: Reads local file bytes.
 */
bool readAllText(const std::filesystem::path& path, std::string& output) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }

    input.seekg(0, std::ios::end);
    const auto size = input.tellg();
    if (size < 0) {
        return false;
    }
    output.resize(static_cast<std::size_t>(size));
    input.seekg(0, std::ios::beg);
    input.read(output.data(), static_cast<std::streamsize>(output.size()));
    return input.good() || input.eof();
}

/**
 * @brief Writes text through a sibling temporary file and rename.
 *
 * @param path Final file path.
 * @param text Plain reviewed page text to write.
 * @return true when write and rename complete.
 * Side effects: Creates parent directories, writes a temp file, and renames it into place.
 */
bool writeAllTextAtomic(const std::filesystem::path& path, const std::string& text) {
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

} // namespace

/**
 * @brief Loads reviewed page text and captures its current file snapshot.
 */
ReviewedPageLoadResult ReviewedPageTextService::load(
    const std::filesystem::path& path) const {
    ReviewedPageLoadResult result;

    if (!readAllText(path, result.text)) {
        result.safeMessage = "reviewed page text file could not be read";
        return result;
    }

    result.snapshot = FileChangeDetector().snapshot(path);
    if (!result.snapshot.exists) {
        result.safeMessage = "reviewed page text snapshot could not be captured";
        result.text.clear();
        return result;
    }

    result.success = true;
    result.safeMessage = "reviewed page text loaded";
    return result;
}

/**
 * @brief Saves reviewed page text after checking for stale writes.
 */
ReviewedPageSaveResult ReviewedPageTextService::save(
    const std::filesystem::path& path,
    const std::string& text,
    const ReviewedPageSavePolicy& policy) const {
    ReviewedPageSaveResult result;
    const FileChangeDetector detector;
    const auto change = detector.compare(path, policy.expectedSnapshot);

    if (!change.canSaveWithoutConflict() && !policy.overwriteExternalChange) {
        result.conflict = true;
        result.safeMessage = "reviewed page text changed before save";
        return result;
    }

    try {
        if (!writeAllTextAtomic(path, text)) {
            result.safeMessage = "reviewed page text could not be saved";
            return result;
        }
    } catch (const std::filesystem::filesystem_error&) {
        result.safeMessage = "filesystem error while saving reviewed page text";
        return result;
    }

    result.savedSnapshot = detector.snapshot(path);
    if (!result.savedSnapshot.exists) {
        result.safeMessage = "reviewed page text saved but snapshot failed";
        return result;
    }

    result.success = true;
    result.safeMessage = "reviewed page text saved";
    return result;
}

} // namespace pte::core
