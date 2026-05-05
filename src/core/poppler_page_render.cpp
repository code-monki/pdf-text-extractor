// File: src/core/poppler_page_render.cpp
// Purpose: pdftoppm single-page PNG render.

#include "core/poppler_page_render.hpp"

#include "core/process_runner.hpp"

#include <filesystem>
#include <vector>

namespace pte::core {

PageRenderOutcome PopplerPageRenderer::renderPagePng(const std::filesystem::path& pdfPath,
                                                     int pdfPageOneBased,
                                                     const std::filesystem::path& outputDirectory,
                                                     const std::string& outputBaseName,
                                                     const Options& options) const {
    PageRenderOutcome outcome;
    std::error_code error;
    if (pdfPath.empty() || !std::filesystem::is_regular_file(pdfPath, error) || error) {
        outcome.safeMessage = "pdftoppm source is not a readable file";
        return outcome;
    }
    if (pdfPageOneBased < 1) {
        outcome.safeMessage = "PDF page number must be positive";
        return outcome;
    }
    if (outputBaseName.empty()) {
        outcome.safeMessage = "render output base name is required";
        return outcome;
    }
    if (!std::filesystem::is_directory(outputDirectory, error) || error) {
        outcome.safeMessage = "render output directory is unavailable";
        return outcome;
    }

    const std::string pageText = std::to_string(pdfPageOneBased);
    const std::string dpiText = std::to_string(options.dpi);
    const std::filesystem::path outputPrefix = outputDirectory / outputBaseName;

    std::vector<std::string> argv = {
        "pdftoppm",
        "-png",
        "-singlefile",
        "-rx",
        dpiText,
        "-ry",
        dpiText,
        "-f",
        pageText,
        "-l",
        pageText,
        pdfPath.string(),
        outputPrefix.string(),
    };

    const auto run = runProcessArgv(options.pdftoppmExecutable, argv, {});
    outcome.exitCode = run.exitCode;

    if (run.spawnFailed) {
        outcome.safeMessage = "pdftoppm could not be started";
        return outcome;
    }
    if (run.exitCode != 0) {
        outcome.safeMessage = "pdftoppm exited with non-zero status";
        return outcome;
    }

    const std::filesystem::path pngPath = outputPrefix.string() + ".png";
    if (!std::filesystem::is_regular_file(pngPath, error) || error) {
        outcome.safeMessage = "pdftoppm did not produce the expected PNG file";
        return outcome;
    }

    outcome.imagePath = pngPath;
    outcome.success = true;
    outcome.safeMessage = "page rendered to PNG";
    return outcome;
}

} // namespace pte::core
