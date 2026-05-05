// File: src/core/page_id.cpp
// Purpose: Implement stable zero-padded page IDs.
// Architectural context: HLA-META, HLA-WORK, HLA-VALID.
// Requirement references: FR-009, FR-011, FR-025.
// Test references: TC-FR-009, TC-FR-011, TC-FR-025.
// Assumption: Current artifacts use four-digit IDs; larger volumes can extend through later
// schema migration if needed.

#include "core/page_id.hpp"

#include <iomanip>
#include <sstream>

namespace pte::core {

/**
 * @brief Formats a one-based PDF page number into the current four-digit page ID.
 *
 * The function is intentionally deterministic and does not inspect PDF content or printed page
 * labels.
 */
std::string pageIdForPdfPage(int pdfPage) {
    std::ostringstream out;
    out << std::setw(4) << std::setfill('0') << pdfPage;
    return out.str();
}

/**
 * @brief Validates that a candidate page ID matches the current artifact schema.
 *
 * The validator rejects non-digits, non-four-character values, and the non-page sentinel "0000".
 */
bool isValidPageId(const std::string& pageId) {
    if (pageId.size() != 4) {
        return false;
    }
    for (const char c : pageId) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return pageId != "0000";
}

} // namespace pte::core
