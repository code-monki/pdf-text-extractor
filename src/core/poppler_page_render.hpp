// File: src/core/poppler_page_render.hpp
// Purpose: Rasterize one PDF page to PNG via Poppler pdftoppm (HLA-PDF render path).
// Architectural context: HLA-PDF, HLA-OCR.
// Requirement references: FR-014, FR-019, NFR-004.
// Constraint: Non-mutating read of PDF bytes; output is a local image path for OCR input.

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief Result of rendering one page to a PNG file via pdftoppm -singlefile.
 */
struct PageRenderOutcome {
    bool success = false;
    std::filesystem::path imagePath;
    std::string safeMessage;
    int exitCode = -1;
};

/**
 * @brief Renders one PDF page to a PNG file in an existing directory.
 */
class PopplerPageRenderer {
public:
    struct Options {
        std::optional<std::filesystem::path> pdftoppmExecutable;
        /** Horizontal and vertical resolution passed to pdftoppm -rx / -ry. */
        int dpi = 200;
    };

    /**
     * @brief Writes exactly one PNG using pdftoppm -png -singlefile -f N -l N.
     *
     * @param pdfPath Source PDF.
     * @param pdfPageOneBased Physical page number (1-based).
     * @param outputDirectory Existing directory; receives `<outputBaseName>.png`.
     * @param outputBaseName File name without extension (safe ASCII recommended).
     */
    PageRenderOutcome renderPagePng(const std::filesystem::path& pdfPath,
                                    int pdfPageOneBased,
                                    const std::filesystem::path& outputDirectory,
                                    const std::string& outputBaseName,
                                    const Options& options) const;
};

} // namespace pte::core
