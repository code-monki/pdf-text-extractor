// File: src/core/volume_bootstrap.hpp
// Purpose: Headless application façade — inventory, work-folder init, optional extraction.
// Architectural context: HLA-APP, HLA-INV, HLA-WORK, HLA-EXT, HLA-OCR.
// Requirement references: FR-001, FR-002, FR-004, FR-012–FR-014, FR-024, NFR-001.
// Constraint: Never modifies source PDF bytes; orchestration only.

#pragma once

#include "core/candidate_generation_poppler.hpp"
#include "core/source_inventory.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief Single-shot bootstrap from a selected source PDF into a new work folder.
 */
struct VolumeBootstrapRequest {
    std::filesystem::path corpusRoot;
    std::filesystem::path sourcePdfPath;
    std::filesystem::path workFolder;
    std::string volumeId;
    std::string title;
    std::string subtitle;
    std::string sortTitle;
    std::string group;

    PopplerPdfInspectionOptions pdfInspection;
    PopplerTesseractToolPaths extractionTools;
    CandidateGenerationOptions generation;

    /** When false, stops after work-folder initialization (empty candidates / placeholder diagnostics). */
    bool runCandidateExtraction = true;
};

/**
 * @brief Outcome of the bootstrap pipeline (safe summaries only).
 */
struct VolumeBootstrapResult {
    bool success = false;
    std::string safeMessage;

    bool inventoryRecorded = false;
    bool workFolderInitialized = false;
    bool extractionCompleted = false;

    std::optional<int> pageCount;
};

/**
 * @brief Runs pdfinfo-backed inventory, initializes the work folder using inspected page count,
 * then optionally runs volume-wide candidate extraction.
 */
class VolumeBootstrapService {
public:
    VolumeBootstrapResult run(const VolumeBootstrapRequest& request) const;
};

} // namespace pte::core
