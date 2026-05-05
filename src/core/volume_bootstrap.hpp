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

    /**
     * When true, if volume.json already matches this PDF (volumeId, filename, page count), skip
     * re-initializing the work folder and skip candidate extraction unless reextractWhenResuming
     * is true. Used by the shell so reopening a PDF does not wipe reviewed page text or re-run
     * pdftotext. CLI/bootstrap defaults leave this false for explicit full runs.
     */
    bool resumeWhenWorkFolderMatches = false;
    /**
     * When resumeWhenWorkFolderMatches applies and the folder matches, force extractAllPages even
     * though extraction would otherwise be skipped on resume (user-requested re-extraction).
     */
    bool reextractWhenResuming = false;
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
