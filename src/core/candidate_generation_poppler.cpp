// File: src/core/candidate_generation_poppler.cpp
// Purpose: Wire embedded extraction, page render, OCR, and candidate storage.

#include "core/candidate_generation_poppler.hpp"

#include "core/page_id.hpp"
#include "core/poppler_embedded_text.hpp"
#include "core/poppler_page_render.hpp"
#include "core/tesseract_ocr.hpp"

#include <chrono>
#include <filesystem>

namespace pte::core {

CandidateGenerationSummary PopplerTesseractCandidateGenerator::generateForPage(
    const std::filesystem::path& pdfPath,
    const std::filesystem::path& workFolder,
    int pdfPageOneBased,
    const std::string& pageId,
    const PopplerTesseractToolPaths& tools,
    const CandidateGenerationOptions& options) const {
    CandidateGenerationSummary summary;
    if (!isValidPageId(pageId) || pageIdForPdfPage(pdfPageOneBased) != pageId) {
        summary.embeddedMessage = "page ID does not match PDF page index";
        summary.ocrMessage = summary.embeddedMessage;
        return summary;
    }

    const CandidateTextService candidates;

    if (options.extractEmbedded) {
        summary.embeddedAttempted = true;
        PopplerEmbeddedTextExtractor embeddedExtractor;
        PopplerEmbeddedTextExtractor::Options embeddedOptions;
        embeddedOptions.pdftotextExecutable = tools.pdftotextExecutable;

        const auto embeddedOutcome =
            embeddedExtractor.extractPage(pdfPath, pdfPageOneBased, embeddedOptions);
        if (!embeddedOutcome.success) {
            summary.embeddedMessage = embeddedOutcome.safeMessage;
        } else {
            const auto stored =
                candidates.storeCandidate(workFolder, pageId, CandidateSource::Embedded,
                                          embeddedOutcome.text);
            if (!stored.success) {
                summary.embeddedMessage = stored.safeMessage;
            } else {
                summary.embeddedStored = true;
                summary.embeddedMessage = stored.safeMessage;
            }
        }
    }

    if (!options.extractOcr) {
        return summary;
    }

    summary.ocrAttempted = true;
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const auto tempRoot = std::filesystem::temp_directory_path()
        / ("pte-ocr-" + std::to_string(stamp) + "-" + pageId);

    std::error_code error;
    std::filesystem::create_directories(tempRoot, error);
    if (error) {
        summary.ocrMessage = "temporary render directory could not be created";
        return summary;
    }

    PopplerPageRenderer renderer;
    PopplerPageRenderer::Options renderOptions;
    renderOptions.pdftoppmExecutable = tools.pdftoppmExecutable;
    renderOptions.dpi = options.renderDpi;

    const std::string baseName = "page-" + pageId;
    const auto renderOutcome =
        renderer.renderPagePng(pdfPath, pdfPageOneBased, tempRoot, baseName, renderOptions);
    if (!renderOutcome.success) {
        std::filesystem::remove_all(tempRoot, error);
        summary.ocrMessage = renderOutcome.safeMessage;
        return summary;
    }

    TesseractOcrRunner ocrRunner;
    TesseractOcrRunner::Options ocrOptions;
    ocrOptions.tesseractExecutable = tools.tesseractExecutable;
    ocrOptions.language = options.ocrLanguage;

    const auto ocrOutcome = ocrRunner.runOnImageFile(renderOutcome.imagePath, ocrOptions);
    std::filesystem::remove_all(tempRoot, error);

    if (!ocrOutcome.success) {
        summary.ocrMessage = ocrOutcome.safeMessage;
        return summary;
    }

    const auto stored =
        candidates.storeCandidate(workFolder, pageId, CandidateSource::Ocr, ocrOutcome.text);
    if (!stored.success) {
        summary.ocrMessage = stored.safeMessage;
    } else {
        summary.ocrStored = true;
        summary.ocrMessage = stored.safeMessage;
    }

    return summary;
}

} // namespace pte::core
