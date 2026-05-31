// File: src/core/pdf_enrichment.hpp
// Purpose: Gate 8 enrichment boundary for derived-PDF outline/link workflows.
// Architectural context: HLA-PDF, HLA-UI, HLA-PKG, HLA-SAFE.
// Requirement references: FR-034, FR-035, FR-036, NFR-003, NFR-012.
// Constraint: Never mutates source PDF; writes derived artifacts only.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pte::core {

struct PdfEnrichmentRequest {
    std::filesystem::path sourcePdfPath;
    std::filesystem::path derivedPdfPath;
    std::optional<std::filesystem::path> outlineMapPath;
    std::optional<std::filesystem::path> linkMapPath;
    std::optional<std::filesystem::path> reportPath;
    bool enableAnnotationInjection = false;
    bool lintOnly = false;
    std::optional<std::filesystem::path> pythonExecutable;
    std::optional<std::filesystem::path> pythonScriptPath;
};

struct PdfEnrichmentResult {
    bool success = false;
    std::string safeMessage;
    bool derivedPdfWritten = false;
    int outlineNodeCount = 0;
    int linkCount = 0;
    int manualOverrideCount = 0;
    std::vector<std::string> warnings;
};

/** @brief One link rectangle from `link-map.json` for shell preview overlays (FR-035/036). */
struct EnrichmentLinkPreviewEntry {
    int pageIndex = 0;
    /** PDF user-space rectangle: llx, lly, urx, ury (bottom-left origin). */
    double rect[4]{};
    bool manual = false;
    std::string targetType;
};

struct EnrichmentLinkMapPreviewLoadResult {
    bool success = false;
    std::string safeMessage;
    std::vector<EnrichmentLinkPreviewEntry> links;
};

/**
 * @brief Loads link-map geometry for preview overlays without full enrichment validation.
 *
 * @details Skips outline cross-checks and target payload rules enforced by `PdfEnrichmentService`.
 *          Does not read PDF content.
 */
EnrichmentLinkMapPreviewLoadResult loadLinkMapForPreview(const std::filesystem::path& path);

/**
 * @brief Validates enrichment sidecars and emits a derived PDF prototype artifact.
 *
 * Prototype scope (Gate 8 slice):
 * - validates sidecar JSON structure for outline/link maps,
 * - copies source PDF to derived output path without source mutation,
 * - writes a compact safe report JSON.
 *
 * Full bookmark/link annotation injection is intentionally deferred to a follow-up slice.
 */
class PdfEnrichmentService {
public:
    PdfEnrichmentResult run(const PdfEnrichmentRequest& request) const;
};

} // namespace pte::core

