// File: src/core/volume_extraction_pipeline.hpp
// Purpose: Run per-page Poppler/Tesseract extraction and persist page-quality diagnostics.
// Architectural context: HLA-APP coordination, HLA-WORK, HLA-DIAG, HLA-EXT, HLA-OCR.
// Requirement references: FR-012, FR-013, FR-014, FR-024.
// Constraint: Does not change review-state.json or reviewed page text; raw candidates only.

#pragma once

#include "core/candidate_generation_poppler.hpp"

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Volume-wide candidate extraction and diagnostic persistence request.
 */
struct VolumeCandidateExtractRequest {
    std::filesystem::path workFolder;
    std::filesystem::path sourcePdfPath;
    std::string volumeId;
    int pageCount = 0;
    CandidateGenerationOptions generation;
    PopplerTesseractToolPaths tools;
};

/**
 * @brief Result of extraction and page-quality rewrite.
 */
struct VolumeCandidateExtractResult {
    bool success = false;
    int pagesProcessed = 0;
    std::string safeMessage;
};

/**
 * @brief Generates raw candidates for every page and rewrites page-quality.json from outcomes.
 */
class VolumeCandidateExtractService {
public:
    /**
     * @brief Processes pages 1..pageCount inclusive.
     *
     * @post On success, raw embedded/OCR artifacts exist where tools succeeded and page-quality.json
     * reflects counts, tool metadata, comparison flags, and extraction errors (no page text).
     */
    VolumeCandidateExtractResult extractAllPages(const VolumeCandidateExtractRequest& request) const;
};

} // namespace pte::core
