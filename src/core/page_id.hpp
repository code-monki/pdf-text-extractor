// File: src/core/page_id.hpp
// Purpose: Define stable PDF page file identifiers for work-folder artifacts.
// Architectural context: HLA-META, HLA-WORK, HLA-VALID.
// Requirement references: FR-009, FR-011, FR-025.
// Test references: TC-FR-009, TC-FR-011, TC-FR-025.
// Constraint: Page IDs are derived from one-based PDF page numbers and are not printed page labels.

#pragma once

#include <string>

namespace pte::core {

/**
 * @brief Formats a one-based PDF page number as the stable work-folder page ID.
 *
 * @param pdfPage One-based source PDF page number.
 * @return Four-digit zero-padded page ID for current artifact schema.
 * @pre pdfPage should be positive for valid extractor artifacts.
 * Side effects: None.
 * Determinism: Deterministic for a given page number.
 */
std::string pageIdForPdfPage(int pdfPage);

/**
 * @brief Validates a page ID against the current four-digit artifact contract.
 *
 * @param pageId Candidate page ID.
 * @return true when pageId is four decimal digits and not "0000".
 * Side effects: None.
 * Determinism: Deterministic for a given page ID string.
 */
bool isValidPageId(const std::string& pageId);

} // namespace pte::core
