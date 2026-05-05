// File: src/core/page_quality.hpp
// Purpose: Persist machine-maintained page quality diagnostics.
// Architectural context: HLA-DIAG, HLA-EXT, HLA-OCR, HLA-WORK, HLA-SAFE.
// Requirement references: FR-012, FR-013, FR-014, FR-024, NFR-004, NFR-005,
// NFR-010, NFR-012.
// Test references: TC-FR-012, TC-FR-013, TC-FR-014, TC-FR-024, TC-NFR-004,
// TC-NFR-005, TC-NFR-010, TC-NFR-012, TC-BND-OCR-004, TC-BND-OCR-005.
// Constraint: Diagnostics persist counts, flags, and tool metadata, not page text contents.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Candidate diagnostics for one source on one page.
 */
struct CandidateDiagnostics {
    bool available = false;
    std::size_t charCount = 0;
    std::string toolInfo;
    std::string modePolicy = "not-run";
};

/**
 * @brief Machine-maintained quality diagnostics for one page.
 */
struct PageQualityEntry {
    std::string pageId;
    int pdfPage = 0;
    std::string selectedSource = "empty";
    CandidateDiagnostics embedded;
    CandidateDiagnostics ocr;
    std::vector<std::string> flags;
    std::vector<std::string> errors;
};

/**
 * @brief Result for writing page-quality diagnostics.
 */
struct PageQualityWriteResult {
    bool success = false;
    std::string safeMessage;
};

/**
 * @brief Service for serializing page-quality.json diagnostics.
 */
class PageQualityService {
public:
    /**
     * @brief Writes a complete page-quality.json artifact.
     *
     * @param workFolder Root work-folder path.
     * @param volumeId Volume identifier.
     * @param entries Page diagnostics to serialize.
     * @return Success or safe failure message.
     * @post Serialized diagnostics contain counts, flags, errors, and tool metadata only.
     * Side effects: Writes `page-quality.json` atomically below workFolder.
     * Determinism: Deterministic for stable inputs and filesystem state.
     */
    PageQualityWriteResult writePageQuality(const std::filesystem::path& workFolder,
                                            const std::string& volumeId,
                                            const std::vector<PageQualityEntry>& entries) const;
};

} // namespace pte::core
