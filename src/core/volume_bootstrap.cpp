// File: src/core/volume_bootstrap.cpp
// Purpose: Implement inventory → init → extraction sequencing.

#include "core/volume_bootstrap.hpp"

#include "core/local_path_intent.hpp"
#include "core/volume_extraction_pipeline.hpp"
#include "core/volume_metadata.hpp"
#include "core/work_folder_initializer.hpp"

namespace pte::core {

namespace {

bool metadataMatchesResume(const VolumeMetadata& meta,
                           const VolumeBootstrapRequest& request,
                           int inventoryPageCount) {
    if (meta.volumeId != request.volumeId) {
        return false;
    }
    if (meta.sourcePageCount != inventoryPageCount) {
        return false;
    }
    return meta.sourceFilename == request.sourcePdfPath.filename().string();
}

} // namespace

VolumeBootstrapResult VolumeBootstrapService::run(const VolumeBootstrapRequest& request) const {
    VolumeBootstrapResult result;

    if (request.workFolder.empty()) {
        result.safeMessage = "work folder path is required";
        return result;
    }
    if (request.volumeId.empty()) {
        result.safeMessage = "volumeId is required";
        return result;
    }
    if (request.sourcePdfPath.empty()) {
        result.safeMessage = "source PDF path is required";
        return result;
    }

    if (!request.corpusRoot.empty()
        && pathTextLooksLikeHostedUriScheme(request.corpusRoot.generic_string())) {
        result.safeMessage = "corpus root must be a local filesystem path";
        return result;
    }
    if (pathTextLooksLikeHostedUriScheme(request.sourcePdfPath.generic_string())) {
        result.safeMessage = "source PDF must be a local filesystem path";
        return result;
    }
    if (pathTextLooksLikeHostedUriScheme(request.workFolder.generic_string())) {
        result.safeMessage = "work folder must be a local filesystem path";
        return result;
    }

    SourceInventoryService inventoryService;
    const auto inventoryRecord = inventoryService.inventoryWithPopplerInspection(
        request.sourcePdfPath, request.corpusRoot, request.pdfInspection);

    if (!inventoryRecord.success) {
        result.safeMessage = inventoryRecord.safeMessage;
        return result;
    }
    result.inventoryRecorded = true;

    if (!inventoryRecord.pageCount.has_value() || *inventoryRecord.pageCount <= 0) {
        result.safeMessage = "PDF inspection did not yield a positive page count";
        result.pageCount = inventoryRecord.pageCount;
        return result;
    }
    result.pageCount = *inventoryRecord.pageCount;

    if (request.resumeWhenWorkFolderMatches) {
        const VolumeMetadataResult existing = VolumeMetadataService().loadVolumeMetadata(request.workFolder);
        if (existing.success && metadataMatchesResume(existing.metadata, request, *inventoryRecord.pageCount)) {
            result.workFolderInitialized = true;
            result.success = true;
            result.inventoryRecorded = true;

            const bool wantExtract =
                request.runCandidateExtraction && request.reextractWhenResuming;
            if (!wantExtract) {
                result.safeMessage = "resumed existing work folder (skipped re-init and extraction)";
                return result;
            }

            VolumeCandidateExtractRequest extractRequest;
            extractRequest.workFolder = request.workFolder;
            extractRequest.sourcePdfPath = request.sourcePdfPath;
            extractRequest.volumeId = request.volumeId;
            extractRequest.pageCount = *inventoryRecord.pageCount;
            extractRequest.generation = request.generation;
            extractRequest.tools = request.extractionTools;

            const VolumeCandidateExtractService extractor;
            const auto extractOutcome = extractor.extractAllPages(extractRequest);
            if (!extractOutcome.success) {
                result.success = false;
                result.safeMessage = extractOutcome.safeMessage;
                return result;
            }

            result.extractionCompleted = true;
            result.safeMessage = "re-ran candidate extraction for existing work folder";
            return result;
        }
    }

    WorkFolderInitRequest initRequest;
    initRequest.workFolder = request.workFolder;
    initRequest.volumeId = request.volumeId;
    initRequest.title = request.title;
    initRequest.subtitle = request.subtitle;
    initRequest.sortTitle = request.sortTitle;
    initRequest.group = request.group;
    initRequest.sourcePdf.path = request.sourcePdfPath;
    initRequest.sourcePdf.filename = request.sourcePdfPath.filename().string();
    initRequest.sourcePdf.pageCount = *inventoryRecord.pageCount;

    const WorkFolderInitializer initializer;
    const auto initOutcome = initializer.initialize(initRequest);
    if (!initOutcome.success) {
        result.safeMessage = initOutcome.safeMessage;
        return result;
    }
    result.workFolderInitialized = true;

    if (!request.runCandidateExtraction) {
        result.success = true;
        result.safeMessage = "volume work folder initialized without candidate extraction";
        return result;
    }

    VolumeCandidateExtractRequest extractRequest;
    extractRequest.workFolder = request.workFolder;
    extractRequest.sourcePdfPath = request.sourcePdfPath;
    extractRequest.volumeId = request.volumeId;
    extractRequest.pageCount = *inventoryRecord.pageCount;
    extractRequest.generation = request.generation;
    extractRequest.tools = request.extractionTools;

    const VolumeCandidateExtractService extractor;
    const auto extractOutcome = extractor.extractAllPages(extractRequest);
    if (!extractOutcome.success) {
        result.safeMessage = extractOutcome.safeMessage;
        return result;
    }

    result.extractionCompleted = true;
    result.success = true;
    result.safeMessage = "volume bootstrapped with inventory, initialization, and extraction";
    return result;
}

} // namespace pte::core
