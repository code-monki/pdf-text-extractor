// File: src/core/file_change_detector.cpp
// Purpose: Implement safe file snapshot comparison for stale-write detection.
// Architectural context: HLA-WORK with HLA-APP, HLA-SAFE.
// Requirement references: FR-032, NFR-007, NFR-012.
// Test references: TC-FR-032, TC-NFR-007, TC-NFR-012.
// Assumption: This detects accidental single-user external edits, not collaborative locking.
// Constraint: Detection reports metadata and hashes only; it must not expose file contents.

#include "core/file_change_detector.hpp"

#include <fstream>

namespace pte::core {

namespace {

/**
 * @brief FNV-1a offset basis used for deterministic local content fingerprints.
 */
constexpr std::uint64_t fnvOffset = 14695981039346656037ull;

/**
 * @brief FNV-1a prime used for deterministic local content fingerprints.
 */
constexpr std::uint64_t fnvPrime = 1099511628211ull;

/**
 * @brief Converts filesystem time to a stable integer token for equality checks.
 *
 * @param time Filesystem last-write time.
 * @return Implementation-defined duration count suitable for same-process comparisons.
 */
std::int64_t modifiedToken(std::filesystem::file_time_type time) {
    return time.time_since_epoch().count();
}

/**
 * @brief Computes a safe content hash without returning file contents.
 *
 * @param path File path to hash.
 * @param ok Set true when the file was fully read.
 * @return FNV-1a hash for file bytes, or zero when unreadable.
 * Side effects: Reads local file bytes.
 */
std::uint64_t hashFile(const std::filesystem::path& path, bool& ok) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        ok = false;
        return 0;
    }

    std::uint64_t hash = fnvOffset;
    char buffer[4096];
    while (input) {
        input.read(buffer, sizeof(buffer));
        const auto readCount = input.gcount();
        for (std::streamsize index = 0; index < readCount; ++index) {
            hash ^= static_cast<unsigned char>(buffer[index]);
            hash *= fnvPrime;
        }
    }

    ok = input.eof();
    return ok ? hash : 0;
}

} // namespace

/**
 * @brief Reports whether the current file state matches the expected snapshot.
 */
bool FileChangeResult::canSaveWithoutConflict() const {
    return status == FileChangeStatus::Unchanged;
}

/**
 * @brief Captures a safe file snapshot for later stale-write detection.
 */
FileSnapshot FileChangeDetector::snapshot(const std::filesystem::path& path) const {
    FileSnapshot result;
    std::error_code error;

    result.exists = std::filesystem::exists(path, error);
    if (error || !result.exists) {
        return result;
    }

    result.size = std::filesystem::file_size(path, error);
    if (error) {
        result.exists = false;
        return result;
    }

    const auto writeTime = std::filesystem::last_write_time(path, error);
    if (error) {
        result.exists = false;
        return result;
    }
    result.modifiedToken = modifiedToken(writeTime);

    bool hashOk = false;
    result.contentHash = hashFile(path, hashOk);
    if (!hashOk) {
        result.exists = false;
        result.size = 0;
        result.modifiedToken = 0;
        result.contentHash = 0;
    }

    return result;
}

/**
 * @brief Compares current file state to a prior safe snapshot.
 */
FileChangeResult FileChangeDetector::compare(const std::filesystem::path& path,
                                             const FileSnapshot& expected) const {
    FileChangeResult result;
    result.current = snapshot(path);

    if (!result.current.exists) {
        result.status = FileChangeStatus::Missing;
        result.safeMessage = "file is missing or unreadable";
        return result;
    }

    if (!expected.exists) {
        result.status = FileChangeStatus::Changed;
        result.safeMessage = "file now exists but no prior snapshot exists";
        return result;
    }

    if (result.current.size == expected.size
        && result.current.modifiedToken == expected.modifiedToken
        && result.current.contentHash == expected.contentHash) {
        result.status = FileChangeStatus::Unchanged;
        result.safeMessage = "file is unchanged";
        return result;
    }

    result.status = FileChangeStatus::Changed;
    result.safeMessage = "file changed since it was loaded";
    return result;
}

/**
 * @brief Converts file change status to a stable lowercase diagnostic name.
 */
std::string fileChangeStatusName(FileChangeStatus status) {
    switch (status) {
    case FileChangeStatus::Unchanged: return "unchanged";
    case FileChangeStatus::Changed: return "changed";
    case FileChangeStatus::Missing: return "missing";
    case FileChangeStatus::Unreadable: return "unreadable";
    }
    return "unknown";
}

} // namespace pte::core
