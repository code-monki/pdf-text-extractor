/**
 * @file review_session_facade.cpp
 * @brief Implementation of @ref pte::ui::ReviewSessionFacade — session lifecycle and I/O.
 */

#include "ui/review_session_facade.hpp"

#include "core/local_path_intent.hpp"
#include "core/page_id.hpp"
#include "core/candidate_text.hpp"
#include "core/page_review_sync.hpp"
#include "core/reviewed_page_text.hpp"
#include "core/review_state_update.hpp"
#include "core/volume_bootstrap.hpp"
#include "core/volume_extraction_pipeline.hpp"
#include "core/volume_metadata.hpp"

#include <QStringList>
#include <QUrl>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string_view>
#include <optional>
#include <sstream>
#include <vector>

namespace pte::ui {

namespace {

namespace core = pte::core;

std::optional<std::filesystem::path> findExecutableOnPath(const std::string& name) {
    const char* rawPath = std::getenv("PATH");
    if (rawPath == nullptr || *rawPath == '\0') {
        return std::nullopt;
    }

    std::stringstream stream(rawPath);
    std::string entry;
    while (std::getline(stream, entry, ':')) {
        if (entry.empty()) {
            continue;
        }
        const auto candidate = std::filesystem::path(entry) / name;
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> defaultPdfinfoPath() {
    if (const auto fromPath = findExecutableOnPath("pdfinfo"); fromPath.has_value()) {
        return fromPath;
    }

    static const std::vector<std::filesystem::path> kCommon = {
        "/opt/homebrew/bin/pdfinfo",
        "/usr/local/bin/pdfinfo",
        "/opt/local/bin/pdfinfo",
        "/usr/bin/pdfinfo",
    };
    for (const auto& candidate : kCommon) {
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> defaultPdftoppmPath() {
    if (const auto fromPath = findExecutableOnPath("pdftoppm"); fromPath.has_value()) {
        return fromPath;
    }

    static const std::vector<std::filesystem::path> kCommon = {
        "/opt/homebrew/bin/pdftoppm",
        "/usr/local/bin/pdftoppm",
        "/opt/local/bin/pdftoppm",
        "/usr/bin/pdftoppm",
    };
    for (const auto& candidate : kCommon) {
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> defaultPdftotextPath() {
    if (const auto fromPath = findExecutableOnPath("pdftotext"); fromPath.has_value()) {
        return fromPath;
    }

    static const std::vector<std::filesystem::path> kCommon = {
        "/opt/homebrew/bin/pdftotext",
        "/usr/local/bin/pdftotext",
        "/opt/local/bin/pdftotext",
        "/usr/bin/pdftotext",
    };
    for (const auto& candidate : kCommon) {
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
    }
    return std::nullopt;
}

bool isBlankReviewedText(std::string_view text) {
    return std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isspace(c) != 0; });
}

} // namespace

ReviewSessionFacade::ReviewSessionFacade(QObject* parent) : QObject(parent) {}

QString ReviewSessionFacade::statusMessage() const {
    return statusMessage_;
}

QString ReviewSessionFacade::sourcePdfPath() const {
    return sourcePdfPath_;
}

QString ReviewSessionFacade::workFolderPath() const {
    return workFolderPath_;
}

int ReviewSessionFacade::pageCount() const {
    return pageIds_.size();
}

QStringList ReviewSessionFacade::pageIds() const {
    return pageIds_;
}

int ReviewSessionFacade::currentPageIndex() const {
    return currentPageIndex_;
}

QString ReviewSessionFacade::currentPageId() const {
    if (currentPageIndex_ < 0 || currentPageIndex_ >= pageIds_.size()) {
        return {};
    }
    return pageIds_.at(currentPageIndex_);
}

QString ReviewSessionFacade::currentPageText() const {
    return currentPageText_;
}

QString ReviewSessionFacade::reviewStatus() const {
    return reviewStatus_;
}

QString ReviewSessionFacade::selectedSource() const {
    return selectedSource_;
}

QString ReviewSessionFacade::printedPageLabel() const {
    return printedPageLabel_;
}

QString ReviewSessionFacade::volumeTitle() const {
    return volumeTitle_;
}

QString ReviewSessionFacade::reviewSyncSummary() const {
    if (workFolderPath_.isEmpty() || currentPageIndex_ < 0) {
        return tr("—");
    }
    QStringList parts;
    if (!volumeTitle_.isEmpty()) {
        parts << tr("Vol: %1").arg(volumeTitle_);
    }
    if (!reviewStatus_.isEmpty()) {
        parts << tr("Review: %1").arg(reviewStatus_);
    }
    if (!selectedSource_.isEmpty()) {
        parts << tr("Src: %1").arg(selectedSource_);
    }
    if (!printedPageLabel_.isEmpty()) {
        parts << tr("Printed: %1").arg(printedPageLabel_);
    }
    if (parts.isEmpty()) {
        return tr("(no review metadata)");
    }
    return parts.join(QStringLiteral(" \u00b7 "));
}

bool ReviewSessionFacade::openPdf(const QString& pdfPath, const QString& requestedWorkFolder) {
    QString resolvedSource = pdfPath;
    if (pdfPath.startsWith(QStringLiteral("file:"))) {
        const QUrl asUrl(pdfPath);
        if (!asUrl.isValid() || !asUrl.isLocalFile()) {
            statusMessage_ = QStringLiteral("file URL is not a valid local path");
            emit statusMessageChanged();
            return false;
        }
        resolvedSource = asUrl.toLocalFile();
        if (resolvedSource.isEmpty()) {
            statusMessage_ = QStringLiteral("file URL did not resolve to a local path");
            emit statusMessageChanged();
            return false;
        }
    }
    const std::filesystem::path source = resolvedSource.toStdString();
    if (source.empty()) {
        statusMessage_ = QStringLiteral("source PDF path is required");
        emit statusMessageChanged();
        return false;
    }
    if (pte::core::pathTextLooksLikeHostedUriScheme(resolvedSource.toStdString())) {
        statusMessage_ = QStringLiteral("source PDF must be a local filesystem path");
        emit statusMessageChanged();
        return false;
    }
    {
        std::error_code error;
        if (!std::filesystem::is_regular_file(source, error) || error) {
            statusMessage_ = QStringLiteral("selected path is not a readable PDF file");
            emit statusMessageChanged();
            return false;
        }
    }

    const std::filesystem::path workFolder =
        requestedWorkFolder.isEmpty()
            ? defaultWorkFolderForPdf(source)
            : std::filesystem::path(requestedWorkFolder.toStdString());

    core::VolumeBootstrapRequest request;
    request.corpusRoot = source.parent_path();
    request.sourcePdfPath = source;
    request.workFolder = workFolder;
    request.volumeId = makeVolumeId(source.stem().string());
    request.title = source.stem().string();
    request.generation.extractEmbedded = true;
    request.generation.extractOcr = false;
    if (const auto pdfinfo = defaultPdfinfoPath(); pdfinfo.has_value()) {
        request.pdfInspection.pdfinfoExecutable = *pdfinfo;
    }
    if (const auto pdftotext = defaultPdftotextPath(); pdftotext.has_value()) {
        request.extractionTools.pdftotextExecutable = *pdftotext;
        request.runCandidateExtraction = true;
    } else {
        request.runCandidateExtraction = false;
    }
    if (const auto pdftoppm = defaultPdftoppmPath(); pdftoppm.has_value()) {
        request.extractionTools.pdftoppmExecutable = *pdftoppm;
    }
    request.resumeWhenWorkFolderMatches = true;
    request.reextractWhenResuming = false;

    const core::VolumeBootstrapResult result = core::VolumeBootstrapService().run(request);
    statusMessage_ = QString::fromStdString(result.safeMessage);
    if (!result.success && request.pdfInspection.pdfinfoExecutable.empty()) {
        statusMessage_ += QStringLiteral(" (pdfinfo not found; install Poppler and ensure pdfinfo is on PATH)");
    } else if (result.success && !request.runCandidateExtraction) {
        statusMessage_ += QStringLiteral(
            " (pdftotext not found; page text stays empty until Poppler pdftotext is on PATH)");
    }
    emit statusMessageChanged();
    if (!result.success || !result.pageCount.has_value() || *result.pageCount <= 0) {
        return false;
    }

    sourcePdfPath_ = QString::fromStdString(source.string());
    workFolderPath_ = QString::fromStdString(workFolder.string());

    pageIds_.clear();
    for (int page = 1; page <= *result.pageCount; ++page) {
        pageIds_.push_back(QString::fromStdString(core::pageIdForPdfPage(page)));
    }
    currentPageIndex_ = 0;
    reloadReviewSync();
    emit sessionChanged();
    emit currentPageChanged();
    return reloadCurrentPageText();
}

void ReviewSessionFacade::refreshAfterVolumeMetadataSave() {
    reloadReviewSync();
}

bool ReviewSessionFacade::reextractEmbeddedCandidates() {
    if (workFolderPath_.isEmpty() || sourcePdfPath_.isEmpty() || pageIds_.isEmpty()) {
        statusMessage_ = QStringLiteral("open a PDF with a work folder first");
        emit statusMessageChanged();
        return false;
    }
    const std::filesystem::path workFolder(workFolderPath_.toStdString());
    const auto loaded = pte::core::VolumeMetadataService().loadVolumeMetadata(workFolder);
    if (!loaded.success) {
        statusMessage_ = QString::fromStdString(loaded.safeMessage);
        emit statusMessageChanged();
        return false;
    }

    pte::core::VolumeCandidateExtractRequest extractRequest;
    extractRequest.workFolder = workFolder;
    extractRequest.sourcePdfPath = sourcePdfPath_.toStdString();
    extractRequest.volumeId = loaded.metadata.volumeId;
    extractRequest.pageCount = loaded.metadata.sourcePageCount;
    extractRequest.generation.extractEmbedded = true;
    extractRequest.generation.extractOcr = false;
    if (const auto pdftotext = defaultPdftotextPath(); pdftotext.has_value()) {
        extractRequest.tools.pdftotextExecutable = *pdftotext;
    }
    if (const auto pdftoppm = defaultPdftoppmPath(); pdftoppm.has_value()) {
        extractRequest.tools.pdftoppmExecutable = *pdftoppm;
    }

    const auto outcome = pte::core::VolumeCandidateExtractService().extractAllPages(extractRequest);
    statusMessage_ = QString::fromStdString(outcome.safeMessage);
    emit statusMessageChanged();
    if (!outcome.success) {
        return false;
    }
    reloadCurrentPageText();
    return true;
}

bool ReviewSessionFacade::selectPage(int index) {
    if (index < 0 || index >= pageIds_.size()) {
        return false;
    }
    if (index == currentPageIndex_) {
        return true;
    }
    currentPageIndex_ = index;
    reloadReviewSync();
    emit currentPageChanged();
    return reloadCurrentPageText();
}

bool ReviewSessionFacade::saveCurrentPageText(const QString& text) {
    const std::filesystem::path path = currentPagePath();
    if (path.empty()) {
        statusMessage_ = QStringLiteral("no selected page to save");
        emit statusMessageChanged();
        return false;
    }

    core::ReviewedPageTextService service;
    const auto loaded = service.load(path);
    if (!loaded.success) {
        statusMessage_ = QString::fromStdString(loaded.safeMessage);
        emit statusMessageChanged();
        return false;
    }

    core::ReviewedPageSavePolicy policy;
    policy.expectedSnapshot = loaded.snapshot;
    const auto saved = service.save(path, text.toStdString(), policy);
    statusMessage_ = QString::fromStdString(saved.safeMessage);
    emit statusMessageChanged();
    if (!saved.success) {
        return false;
    }

    const std::string pageId = pageIds_.at(currentPageIndex_).toStdString();
    const std::filesystem::path wf(workFolderPath_.toStdString());
    const std::string body = text.toStdString();
    core::ReviewStatePageSavePatch patch;
    patch.selectedSource = isBlankReviewedText(body) ? "empty"
        : (lastFillWasEmbeddedCandidate_ ? "embedded" : "manual");
    const auto stateUpdate = core::applyReviewStateAfterPageTextSave(wf, pageId, patch);
    if (!stateUpdate.success) {
        statusMessage_ += QStringLiteral(" (") + QString::fromStdString(stateUpdate.safeMessage) + QStringLiteral(")");
        emit statusMessageChanged();
    } else {
        reloadReviewSync();
    }
    lastFillWasEmbeddedCandidate_ = false;

    currentPageText_ = text;
    emit currentPageTextChanged();
    return true;
}

std::filesystem::path ReviewSessionFacade::currentPagePath() const {
    if (workFolderPath_.isEmpty() || currentPageIndex_ < 0 || currentPageIndex_ >= pageIds_.size()) {
        return {};
    }
    return std::filesystem::path(workFolderPath_.toStdString()) / "pages" /
           (pageIds_.at(currentPageIndex_).toStdString() + ".txt");
}

bool ReviewSessionFacade::reloadCurrentPageText() {
    const std::filesystem::path path = currentPagePath();
    if (path.empty()) {
        currentPageText_.clear();
        emit currentPageTextChanged();
        return false;
    }

    const auto loaded = core::ReviewedPageTextService().load(path);
    if (!loaded.success) {
        statusMessage_ = QString::fromStdString(loaded.safeMessage);
        emit statusMessageChanged();
        return false;
    }

    std::string displayText = loaded.text;
    QString extraStatus;
    lastFillWasEmbeddedCandidate_ = false;
    if (isBlankReviewedText(displayText)) {
        const std::filesystem::path wf(workFolderPath_.toStdString());
        const std::string pageId = pageIds_.at(currentPageIndex_).toStdString();
        auto embedded =
            core::CandidateTextService().loadCandidate(wf, pageId, core::CandidateSource::Embedded);
        if (embedded.success && embedded.charCount > 0) {
            displayText = std::move(embedded.text);
            lastFillWasEmbeddedCandidate_ = true;
            extraStatus = QStringLiteral(
                " - showing embedded extraction; Save page text to store as reviewed");
        }
    }

    statusMessage_ = QString::fromStdString(loaded.safeMessage) + extraStatus;
    emit statusMessageChanged();

    currentPageText_ = QString::fromStdString(displayText);
    emit currentPageTextChanged();
    return true;
}

void ReviewSessionFacade::reloadReviewSync() {
    reviewStatus_.clear();
    selectedSource_.clear();
    printedPageLabel_.clear();
    volumeTitle_.clear();
    if (workFolderPath_.isEmpty() || currentPageIndex_ < 0 || currentPageIndex_ >= pageIds_.size()) {
        emit reviewSyncChanged();
        return;
    }

    const pte::core::PageReviewSyncSnapshot snapshot = pte::core::loadPageReviewSyncSnapshot(
        std::filesystem::path(workFolderPath_.toStdString()),
        pageIds_.at(currentPageIndex_).toStdString());
    if (snapshot.loaded) {
        reviewStatus_ = QString::fromStdString(snapshot.reviewStatus);
        selectedSource_ = QString::fromStdString(snapshot.selectedSource);
        printedPageLabel_ = QString::fromStdString(snapshot.printedPageLabel);
        volumeTitle_ = QString::fromStdString(snapshot.volumeTitle);
    }
    emit reviewSyncChanged();
}

std::string ReviewSessionFacade::makeVolumeId(const std::string& filenameStem) {
    std::string out;
    out.reserve(filenameStem.size());
    for (const unsigned char c : filenameStem) {
        if (std::isalnum(c) != 0U) {
            out.push_back(static_cast<char>(std::tolower(c)));
            continue;
        }
        if (c == '-' || c == '_' || c == '.') {
            out.push_back('-');
        }
    }
    if (out.empty()) {
        out = "volume";
    }
    return out;
}

std::filesystem::path ReviewSessionFacade::defaultWorkFolderForPdf(
    const std::filesystem::path& pdfPath) {
    return pdfPath.parent_path() / (pdfPath.stem().string() + "-work");
}

} // namespace pte::ui
