// File: src/core/candidate_generation_poppler.hpp
// Purpose: Orchestrate Poppler + Tesseract raw candidate generation for one PDF page.
// Architectural context: HLA-PDF (render), HLA-EXT, HLA-OCR, HLA-WORK.
// Requirement references: FR-012, FR-013, FR-014, FR-015, FR-024.
// Constraint: Stores only raw candidates; never mutates review-state.json.

#pragma once

#include "core/candidate_text.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief Executable overrides mirroring dependency capability configuration.
 */
struct PopplerTesseractToolPaths {
    std::optional<std::filesystem::path> pdfinfoExecutable;
    std::optional<std::filesystem::path> pdftotextExecutable;
    std::optional<std::filesystem::path> pdftoppmExecutable;
    std::optional<std::filesystem::path> tesseractExecutable;
};

/**
 * @brief Generation controls for one page.
 */
struct CandidateGenerationOptions {
    bool extractEmbedded = true;
    bool extractOcr = true;
    int renderDpi = 200;
    std::string ocrLanguage = "eng";
};

/**
 * @brief Summary of stored raw artifacts for one page.
 */
struct CandidateGenerationSummary {
    bool embeddedAttempted = false;
    bool embeddedStored = false;
    bool ocrAttempted = false;
    bool ocrStored = false;
    std::string embeddedMessage;
    std::string ocrMessage;
};

/**
 * @brief Generates raw embedded and OCR candidates for one PDF page into the work folder.
 */
class PopplerTesseractCandidateGenerator {
public:
    /**
     * @brief Runs configured extraction stages and writes `raw/embedded` and `raw/ocr` text files.
     *
     * @pre pageId must equal pageIdForPdfPage(pdfPageOneBased).
     */
    CandidateGenerationSummary generateForPage(const std::filesystem::path& pdfPath,
                                               const std::filesystem::path& workFolder,
                                               int pdfPageOneBased,
                                               const std::string& pageId,
                                               const PopplerTesseractToolPaths& tools,
                                               const CandidateGenerationOptions& options) const;
};

} // namespace pte::core
