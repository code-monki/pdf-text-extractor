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

