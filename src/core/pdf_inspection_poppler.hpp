// File: src/core/pdf_inspection_poppler.hpp
// Purpose: Populate PdfInspectionEvidence using Poppler pdfinfo (ADR-0005).
// Architectural context: HLA-PDF inspection path, HLA-INV, HLA-DEP.
// Requirement references: FR-002, NFR-004, NFR-010.
// Constraint: Non-mutating subprocess inspection only; argv execution (no shell).

#pragma once

#include "core/source_inventory.hpp"

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Parses pdfinfo stdout into structural evidence (deterministic; test seam).
 *
 * @param stdoutText Raw characters captured from pdfinfo standard output.
 * @return Evidence derived from key/value lines (Pages, Encrypted).
 * Side effects: None.
 */
PdfInspectionEvidence parsePdfinfoStdout(const std::string& stdoutText);

/**
 * @brief Runs pdfinfo on a local PDF path and returns merged inspection evidence.
 */
class PopplerPdfInspectionService {
public:
    struct Options {
        /** When empty, uses PATH resolution for the executable name `pdfinfo`. */
        std::filesystem::path pdfinfoExecutable;
    };

    /**
     * @brief Invokes pdfinfo without modifying the source PDF.
     *
     * @param pdfPath Absolute or normalized path to an existing file.
     * @param options Optional explicit pdfinfo binary path.
     * @return Evidence plus a safe operator-facing message on failure.
     * @post Source PDF bytes are unchanged.
     */
    struct InspectResult {
        PdfInspectionEvidence evidence;
        std::string safeMessage;
        int exitCode = -1;
    };

    InspectResult inspect(const std::filesystem::path& pdfPath, const Options& options) const;
};

} // namespace pte::core
