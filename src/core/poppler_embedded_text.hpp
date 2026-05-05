// File: src/core/poppler_embedded_text.hpp
// Purpose: Embedded text extraction via Poppler pdftotext (HLA-EXT).
// Architectural context: HLA-EXT, HLA-DEP.
// Requirement references: FR-012, FR-015, FR-024, NFR-004, NFR-010.
// Constraint: Produces raw candidate text only; does not modify review state.

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief One-page embedded text extraction outcome from pdftotext.
 */
struct EmbeddedTextExtractOutcome {
    bool success = false;
    std::string text;
    std::size_t charCount = 0;
    std::string toolId = "pdftotext";
    std::string safeMessage;
    int exitCode = -1;
};

/**
 * @brief Runs pdftotext for a single PDF page to stdout (no files written except temp none).
 */
class PopplerEmbeddedTextExtractor {
public:
    struct Options {
        std::optional<std::filesystem::path> pdftotextExecutable;
    };

    /**
     * @brief Extracts embedded text for one page using pdftotext -f N -l N file.pdf -.
     *
     * @param pdfPath Local PDF path.
     * @param pdfPageOneBased One-based physical page index.
     */
    EmbeddedTextExtractOutcome extractPage(const std::filesystem::path& pdfPath,
                                           int pdfPageOneBased,
                                           const Options& options) const;
};

} // namespace pte::core
