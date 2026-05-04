// File: src/core/local_path_intent.hpp
// Purpose: Detect path strings that imply remote/hosted resources (NFR-001).
// Architectural context: HLA-CONFIG, HLA-SAFE.
// Requirement references: FR-029, NFR-001.
// Test references: TC-NFR-001.
// Constraint: Heuristic only; OS-mounted network drives use normal paths and are not detected here.

#pragma once

#include <string_view>

namespace pte::core {

/**
 * @brief Returns true when @p pathText looks like a URI with a scheme (e.g. https://, s3://).
 *
 * `file://` is treated as local intent and returns false so native file URLs can be normalized
 * elsewhere. Paths without a `scheme://` prefix (including UNC/SMB-style paths) return false.
 */
bool pathTextLooksLikeHostedUriScheme(std::string_view pathText);

} // namespace pte::core
