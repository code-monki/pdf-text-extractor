// File: src/core/file_digest.hpp
// Purpose: Stable SHA-256 digest for local files (source PDF integrity / NFR-003).
// Architectural context: HLA-INV, HLA-SAFE.
// Requirement references: NFR-003, NFR-004.
// Test references: TC-NFR-003.

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief Computes lowercase hexadecimal SHA-256 for an entire local file.
 *
 * @return Digest string on success; empty when the file cannot be read.
 */
std::optional<std::string> sha256HexFile(const std::filesystem::path& path);

} // namespace pte::core
