// File: src/core/work_folder_initializer.hpp
// Purpose: Initialize local extractor work-folder artifacts for one PDF volume.
// Architectural context: HLA-WORK, HLA-META, HLA-REVIEW, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-004, FR-005, FR-009, FR-011, FR-017, FR-024, NFR-011.
// Test references: TC-FR-004, TC-FR-005, TC-FR-009, TC-FR-011, TC-FR-017, TC-NFR-011.
// Constraint: Initialization writes local artifacts only and must never modify source PDFs.

#pragma once

#include <filesystem>
#include <string>

namespace pte::core {

/**
 * @brief Source PDF metadata needed to seed work-folder artifacts.
 *
 * This structure carries identity metadata supplied by inventory or a caller-controlled
 * synthetic test path. It does not imply source PDF ownership or mutation rights.
 */
struct SourcePdfSeed {
    std::filesystem::path path;
    std::string filename;
    int pageCount = 0;
};

/**
 * @brief Request model for deterministic work-folder initialization.
 */
struct WorkFolderInitRequest {
    std::filesystem::path workFolder;
    std::string volumeId;
    std::string title;
    std::string subtitle;
    std::string sortTitle;
    std::string group;
    SourcePdfSeed sourcePdf;
};

/**
 * @brief Result returned by work-folder initialization.
 */
struct WorkFolderInitResult {
    bool success = false;
    std::string safeMessage;
};

/**
 * @brief Creates the initial local artifact layout for one volume.
 *
 * The initializer creates `volume.json`, `page-quality.json`, `review-state.json`,
 * `pages/NNNN.txt`, and raw candidate directories using synthetic-safe metadata and empty
 * reviewed page text files. It does not read or modify source PDF bytes.
 */
class WorkFolderInitializer {
public:
    /**
     * @brief Initializes a work folder from caller-supplied source metadata.
     *
     * @param request Initialization request containing output root, stable volume ID, metadata,
     * and source PDF page count.
     * @return Success or failure with a safe message that does not include page text.
     * @pre request.sourcePdf.pageCount must be positive.
     * @post On success, every PDF page has a reviewed text file; existing files are not replaced.
     * Side effects: Creates directories and writes JSON/text artifacts under request.workFolder.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    WorkFolderInitResult initialize(const WorkFolderInitRequest& request) const;
};

} // namespace pte::core
