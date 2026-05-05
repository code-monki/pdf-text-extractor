// File: src/core/file_change_detector.hpp
// Purpose: Detect external modification of local work-folder files before overwrite.
// Architectural context: HLA-WORK with HLA-APP, HLA-SAFE.
// Requirement references: FR-032, NFR-007, NFR-012.
// Test references: TC-FR-032, TC-NFR-007, TC-NFR-012.
// Constraint: Detection reports metadata and hashes only; it must not expose file contents.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Safe snapshot of one file at load/save time.
 *
 * The snapshot is intended as a stale-write token for reviewed text and metadata files. It carries
 * only file metadata and a content hash, never file contents.
 */
struct FileSnapshot {
    bool exists = false;
    std::uintmax_t size = 0;
    std::int64_t modifiedToken = 0;
    std::uint64_t contentHash = 0;
};

/**
 * @brief External modification comparison status.
 */
enum class FileChangeStatus {
    Unchanged,
    Changed,
    Missing,
    Unreadable
};

/**
 * @brief Result of comparing a previously loaded file snapshot to current filesystem state.
 */
struct FileChangeResult {
    FileChangeStatus status = FileChangeStatus::Unreadable;
    FileSnapshot current;
    std::string safeMessage;

    /**
     * @brief Reports whether a save can proceed without conflict handling.
     *
     * @return true when status is Unchanged.
     * Side effects: None.
     * Determinism: Deterministic for this result value.
     */
    bool canSaveWithoutConflict() const;
};

/**
 * @brief Captures and compares safe file snapshots for accidental overwrite protection.
 */
class FileChangeDetector {
public:
    /**
     * @brief Captures a safe snapshot for one file.
     *
     * @param path Local file path to snapshot.
     * @return Snapshot containing existence, size, modified token, and content hash.
     * @post The returned snapshot does not contain file contents.
     * Side effects: Reads local file bytes only to compute the content hash.
     * Determinism: Deterministic for a stable file.
     */
    FileSnapshot snapshot(const std::filesystem::path& path) const;

    /**
     * @brief Compares current file state against a prior snapshot.
     *
     * @param path Local file path to compare.
     * @param expected Snapshot captured when the application loaded or last saved the file.
     * @return Safe comparison result for conflict handling.
     * @post Result messages do not include file contents.
     * Side effects: Reads local file metadata and bytes only for hashing.
     * Determinism: Deterministic for stable input and filesystem state.
     */
    FileChangeResult compare(const std::filesystem::path& path,
                             const FileSnapshot& expected) const;
};

/**
 * @brief Converts a file change status to a stable lowercase diagnostic name.
 *
 * @param status Status to convert.
 * @return Stable status name.
 * Side effects: None.
 * Determinism: Deterministic for a given status.
 */
std::string fileChangeStatusName(FileChangeStatus status);

} // namespace pte::core
