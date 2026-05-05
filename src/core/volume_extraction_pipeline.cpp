// File: src/core/volume_extraction_pipeline.cpp
// Purpose: Implement volume candidate extraction orchestration.

#include "core/volume_extraction_pipeline.hpp"

#include "core/candidate_text.hpp"
#include "core/page_id.hpp"
#include "core/page_quality.hpp"

#include <filesystem>
#include <utility>
#include <vector>

namespace pte::core {

VolumeCandidateExtractResult VolumeCandidateExtractService::extractAllPages(
    const VolumeCandidateExtractRequest& request) const {
    VolumeCandidateExtractResult result;
    std::error_code error;
    if (request.workFolder.empty() || !std::filesystem::is_directory(request.workFolder, error)
        || error) {
        result.safeMessage = "work folder is not available";
        return result;
    }
    if (request.volumeId.empty()) {
        result.safeMessage = "volumeId is required";
        return result;
    }
    if (request.pageCount <= 0) {
        result.safeMessage = "pageCount must be positive";
        return result;
    }
    if (request.sourcePdfPath.empty()
        || !std::filesystem::is_regular_file(request.sourcePdfPath, error) || error) {
        result.safeMessage = "source PDF path is not a readable file";
        return result;
    }

    CandidateTextService candidates;
    PopplerTesseractCandidateGenerator generator;
    std::vector<PageQualityEntry> entries;
    entries.reserve(static_cast<std::size_t>(request.pageCount));

    for (int pdfPage = 1; pdfPage <= request.pageCount; ++pdfPage) {
        const std::string pageId = pageIdForPdfPage(pdfPage);

        const auto summary = generator.generateForPage(request.sourcePdfPath, request.workFolder,
                                                       pdfPage, pageId, request.tools,
                                                       request.generation);

        PageQualityEntry entry;
        entry.pageId = pageId;
        entry.pdfPage = pdfPage;
        entry.selectedSource = "empty";

        const auto comparison = candidates.comparePage(request.workFolder, pageId);
        if (!comparison.success) {
            entry.errors.push_back(comparison.safeMessage);
        }

        entry.embedded.available = comparison.embedded.available;
        entry.embedded.charCount = comparison.embedded.charCount;
        entry.embedded.toolInfo =
            comparison.embedded.available ? std::string("pdftotext") : std::string();
        entry.embedded.modePolicy = request.generation.extractEmbedded ? "default" : "not-run";

        entry.ocr.available = comparison.ocr.available;
        entry.ocr.charCount = comparison.ocr.charCount;
        entry.ocr.toolInfo = comparison.ocr.available ? std::string("tesseract") : std::string();
        entry.ocr.modePolicy = request.generation.extractOcr ? "default" : "not-run";

        entry.flags = comparison.flags;
        if (summary.embeddedAttempted && !summary.embeddedStored) {
            entry.errors.push_back(summary.embeddedMessage);
        }
        if (summary.ocrAttempted && !summary.ocrStored) {
            entry.errors.push_back(summary.ocrMessage);
        }
        if (entry.flags.empty() && !entry.embedded.available && !entry.ocr.available
            && entry.errors.empty()) {
            entry.flags.push_back("empty");
        }

        entries.push_back(std::move(entry));
        ++result.pagesProcessed;
    }

    const PageQualityService quality;
    const auto written =
        quality.writePageQuality(request.workFolder, request.volumeId, entries);
    if (!written.success) {
        result.safeMessage = written.safeMessage;
        result.success = false;
        return result;
    }

    result.success = true;
    result.safeMessage = "volume candidate extraction completed";
    return result;
}

} // namespace pte::core
