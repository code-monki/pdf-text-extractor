/**
 * @file shell_main_window.cpp
 * @brief Implements @ref ShellMainWindow — menus, icon toolbar, and facade-driven refresh logic.
 *
 * Toolbar pixmap choices and operator help text are summarized in docs/shell-user-guide.md.
 */

#include "ui/app_theme.hpp"
#include "ui/shell_main_window.hpp"
#include "ui/volume_metadata_dialog.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QAction>
#include <QActionGroup>
#include <QDialog>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSize>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
#include <QSizePolicy>

#include <pdf_document_view/pdf_document_view_widget.hpp>

#include "core/readiness_summary.hpp"

#include <QtGlobal>

#include <filesystem>

namespace {

/**
 * @brief Returns a style-standard icon for toolbar and menu actions.
 *
 * Uses the active QApplication style so icons follow the platform/Fusion theme.
 */
QIcon shellStandardIcon(QStyle::StandardPixmap which) {
    return QApplication::style()->standardIcon(which);
}

/** @brief Resolves operator guide path for Help → Documentation (repo dev tree or app bundle). */
QString operatorGuidePath() {
#ifdef PTE_REPO_ROOT
    const QString repoGuide =
        QStringLiteral(PTE_REPO_ROOT) + QStringLiteral("/docs/shell-user-guide.md");
    if (QFileInfo::exists(repoGuide)) {
        return repoGuide;
    }
#endif
    const QString bundleGuide =
        QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources/docs/shell-user-guide.md");
    if (QFileInfo::exists(bundleGuide)) {
        return bundleGuide;
    }
    return {};
}

} // namespace

ShellMainWindow::ShellMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , facade_(std::make_unique<pte::ui::ReviewSessionFacade>(this)) {
    setWindowTitle(tr("PDF Text Extractor"));
    buildUi();
    wireFacade();
}

ShellMainWindow::~ShellMainWindow() = default;

void ShellMainWindow::buildUi() {
    // Main toolbar: ToolButtonIconOnly; QAction text remains on menu items. Icon roles are
    // documented in docs/shell-user-guide.md (Help → Documentation target).
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* openAct = new QAction(tr("&Open PDF…"), this);
    openAct->setIcon(shellStandardIcon(QStyle::SP_DialogOpenButton));
    openAct->setShortcut(QKeySequence::Open);
    openAct->setToolTip(tr("Open PDF…\n%1")
                            .arg(openAct->shortcut().toString(QKeySequence::NativeText)));
    openAct->setStatusTip(tr("Open a PDF and initialize or resume its work folder"));
    connect(openAct, &QAction::triggered, this, &ShellMainWindow::onOpenPdf);
    fileMenu->addAction(openAct);

    savePageAction_ = new QAction(tr("&Save Page Text"), this);
    savePageAction_->setIcon(shellStandardIcon(QStyle::SP_DialogSaveButton));
    savePageAction_->setEnabled(false);
    savePageAction_->setToolTip(tr("Save page text to pages/NNNN.txt"));
    savePageAction_->setStatusTip(savePageAction_->toolTip());
    connect(savePageAction_, &QAction::triggered, this, &ShellMainWindow::onSavePageText);
    fileMenu->addAction(savePageAction_);

    editVolumeMetadataAction_ = new QAction(tr("Volume &metadata…"), this);
    editVolumeMetadataAction_->setIcon(shellStandardIcon(QStyle::SP_FileDialogInfoView));
    editVolumeMetadataAction_->setEnabled(false);
    editVolumeMetadataAction_->setShortcut(QKeySequence(tr("Ctrl+M")));
    editVolumeMetadataAction_->setShortcutContext(Qt::ApplicationShortcut);
    editVolumeMetadataAction_->setToolTip(
        tr("Edit volume title, notes, bibliographic fields, page labels, and cover (volume.json)\n%1")
            .arg(editVolumeMetadataAction_->shortcut().toString(QKeySequence::NativeText)));
    editVolumeMetadataAction_->setStatusTip(
        tr("Edit volume title, notes, and bibliographic fields (volume.json)"));
    connect(editVolumeMetadataAction_, &QAction::triggered, this, &ShellMainWindow::onEditVolumeMetadata);
    fileMenu->addAction(editVolumeMetadataAction_);

    reextractEmbeddedAction_ = new QAction(tr("Re-extract embedded &candidates…"), this);
    reextractEmbeddedAction_->setIcon(shellStandardIcon(QStyle::SP_BrowserReload));
    reextractEmbeddedAction_->setEnabled(false);
    reextractEmbeddedAction_->setToolTip(
        tr("Re-run Poppler pdftotext for all pages into raw/embedded (does not erase pages/*.txt)"));
    reextractEmbeddedAction_->setStatusTip(reextractEmbeddedAction_->toolTip());
    connect(reextractEmbeddedAction_, &QAction::triggered, this,
            &ShellMainWindow::onReextractEmbeddedCandidates);
    fileMenu->addAction(reextractEmbeddedAction_);

    readinessSummaryAction_ = new QAction(tr("Readiness su&mmary…"), this);
    readinessSummaryAction_->setIcon(shellStandardIcon(QStyle::SP_MessageBoxInformation));
    readinessSummaryAction_->setEnabled(false);
    readinessSummaryAction_->setToolTip(tr("Show release/readiness counts from local metadata"));
    readinessSummaryAction_->setStatusTip(readinessSummaryAction_->toolTip());
    connect(readinessSummaryAction_, &QAction::triggered, this, &ShellMainWindow::onReadinessSummary);
    fileMenu->addAction(readinessSummaryAction_);

    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), QKeySequence::Quit, this, &QWidget::close);

    findInPreviewAction_ = new QAction(tr("Find in &preview…"), this);
    findInPreviewAction_->setShortcut(QKeySequence::Find);
    findInPreviewAction_->setShortcutContext(Qt::ApplicationShortcut);
    findInPreviewAction_->setEnabled(false);
    findInPreviewAction_->setStatusTip(tr("Focus the preview search field (searches PDF text)"));
    connect(findInPreviewAction_, &QAction::triggered, this, &ShellMainWindow::onFocusPreviewFind);

    findNextInPreviewAction_ = new QAction(tr("Find &next in preview"), this);
    findNextInPreviewAction_->setShortcut(QKeySequence::FindNext);
    findNextInPreviewAction_->setShortcutContext(Qt::ApplicationShortcut);
    findNextInPreviewAction_->setEnabled(false);
    connect(findNextInPreviewAction_, &QAction::triggered, this, &ShellMainWindow::onPreviewFindNext);

    findPrevInPreviewAction_ = new QAction(tr("Find &previous in preview"), this);
    findPrevInPreviewAction_->setShortcut(QKeySequence::FindPrevious);
    findPrevInPreviewAction_->setShortcutContext(Qt::ApplicationShortcut);
    findPrevInPreviewAction_->setEnabled(false);
    connect(findPrevInPreviewAction_, &QAction::triggered, this, &ShellMainWindow::onPreviewFindPrevious);

    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(editVolumeMetadataAction_);
    editMenu->addAction(reextractEmbeddedAction_);
    editMenu->addAction(readinessSummaryAction_);
    editMenu->addSeparator();
    editMenu->addAction(findInPreviewAction_);
    editMenu->addAction(findNextInPreviewAction_);
    editMenu->addAction(findPrevInPreviewAction_);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* themeMenu = viewMenu->addMenu(tr("&Theme"));
    themeActionGroup_ = new QActionGroup(this);
    themeActionGroup_->setExclusive(true);
    for (const pte::ui::ThemeDescriptor& td : pte::ui::listBuiltInThemes()) {
        auto* act = new QAction(td.displayName, this);
        act->setCheckable(true);
        act->setData(td.themeId);
        themeActionGroup_->addAction(act);
        themeMenu->addAction(act);
    }
    connect(themeActionGroup_, &QActionGroup::triggered, this, &ShellMainWindow::onThemeTriggered);
    const QString currentTheme = pte::ui::loadSavedThemeId();
    for (QAction* act : themeActionGroup_->actions()) {
        if (act->data().toString() == currentTheme) {
            act->setChecked(true);
            break;
        }
    }

    auto* previewFitWidthAction = viewMenu->addAction(tr("Preview &fit width"));
    previewFitWidthAction->setStatusTip(tr("Zoom the PDF preview to fit the column width"));
    connect(previewFitWidthAction, &QAction::triggered, this, &ShellMainWindow::onPreviewFitWidth);

    auto* previewResetZoomAction = viewMenu->addAction(tr("Preview &reset zoom"));
    previewResetZoomAction->setStatusTip(tr("Restore 100% zoom in the PDF preview"));
    connect(previewResetZoomAction, &QAction::triggered, this, &ShellMainWindow::onPreviewResetZoom);

    viewMenu->addSeparator();
    viewMenu->addAction(findInPreviewAction_);
    viewMenu->addAction(findNextInPreviewAction_);
    viewMenu->addAction(findPrevInPreviewAction_);

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* checkToolsAction = helpMenu->addAction(tr("Check extraction &tools…"));
    checkToolsAction->setStatusTip(tr("Report Poppler and Tesseract availability (FR-031)"));
    connect(checkToolsAction, &QAction::triggered, this, &ShellMainWindow::onCheckExtractionTools);
    helpMenu->addSeparator();
    auto* documentationAction = helpMenu->addAction(tr("&Documentation…"));
    documentationAction->setStatusTip(tr("Open the shell user guide"));
    connect(documentationAction, &QAction::triggered, this, &ShellMainWindow::onOpenDocumentation);
    helpMenu->addAction(tr("&About…"), this, &ShellMainWindow::onAbout);

    auto* toolBar = addToolBar(tr("Main"));
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(22, 22));
    toolBar->addAction(openAct);

    toolBar->addAction(savePageAction_);
    toolBar->addAction(editVolumeMetadataAction_);
    toolBar->addAction(reextractEmbeddedAction_);
    toolBar->addAction(readinessSummaryAction_);

    toolBar->addSeparator();

    firstPageAction_ = new QAction(shellStandardIcon(QStyle::SP_MediaSkipBackward), QString(), this);
    firstPageAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Home));
    firstPageAction_->setShortcutContext(Qt::ApplicationShortcut);
    firstPageAction_->setToolTip(tr("First page\n%1")
                                     .arg(firstPageAction_->shortcut().toString(QKeySequence::NativeText)));
    connect(firstPageAction_, &QAction::triggered, this, &ShellMainWindow::onFirstPage);
    toolBar->addAction(firstPageAction_);

    prevPageAction_ = new QAction(shellStandardIcon(QStyle::SP_ArrowLeft), QString(), this);
    prevPageAction_->setToolTip(tr("Previous page"));
    connect(prevPageAction_, &QAction::triggered, this, &ShellMainWindow::onPrevPage);
    toolBar->addAction(prevPageAction_);

    pageLabel_ = new QLabel(tr("Page — / —"), this);
    toolBar->addWidget(pageLabel_);

    nextPageAction_ = new QAction(shellStandardIcon(QStyle::SP_ArrowRight), QString(), this);
    nextPageAction_->setToolTip(tr("Next page"));
    connect(nextPageAction_, &QAction::triggered, this, &ShellMainWindow::onNextPage);
    toolBar->addAction(nextPageAction_);

    lastPageAction_ = new QAction(shellStandardIcon(QStyle::SP_MediaSkipForward), QString(), this);
    lastPageAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_End));
    lastPageAction_->setShortcutContext(Qt::ApplicationShortcut);
    lastPageAction_->setToolTip(tr("Last page\n%1")
                                    .arg(lastPageAction_->shortcut().toString(QKeySequence::NativeText)));
    connect(lastPageAction_, &QAction::triggered, this, &ShellMainWindow::onLastPage);
    toolBar->addAction(lastPageAction_);

    firstPageAction_->setEnabled(false);
    prevPageAction_->setEnabled(false);
    nextPageAction_->setEnabled(false);
    lastPageAction_->setEnabled(false);

    reviewSyncLabel_ = new QLabel(tr("—"), this);
    reviewSyncLabel_->setMinimumWidth(280);
    reviewSyncLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(reviewSyncLabel_);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* split = new QSplitter(Qt::Horizontal, central);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(split);

    pageList_ = new QListWidget(split);
    pageList_->setMinimumWidth(220);
    connect(pageList_, &QListWidget::currentRowChanged, this, &ShellMainWindow::onPageListRowChanged);

    previewColumn_ = new QWidget(split);
    auto* previewLayout = new QVBoxLayout(previewColumn_);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(4);

    auto* findRow = new QWidget(previewColumn_);
    auto* findLayout = new QHBoxLayout(findRow);
    findLayout->setContentsMargins(0, 0, 0, 0);
    findLayout->setSpacing(4);

    auto* findLabel = new QLabel(tr("Find:"), findRow);
    previewFindField_ = new QLineEdit(findRow);
    previewFindField_->setPlaceholderText(tr("Search in PDF…"));
    previewFindField_->setClearButtonEnabled(true);
    previewFindField_->setEnabled(false);
    previewFindField_->setToolTip(tr("Search PDF text; Enter runs search, F3 next match"));
    connect(previewFindField_, &QLineEdit::returnPressed, this, &ShellMainWindow::onPreviewFindTriggered);

    auto* findPrevButton = new QToolButton(findRow);
    findPrevButton->setIcon(shellStandardIcon(QStyle::SP_ArrowUp));
    findPrevButton->setToolTip(tr("Previous match in PDF"));
    connect(findPrevButton, &QToolButton::clicked, this, &ShellMainWindow::onPreviewFindPrevious);

    auto* findNextButton = new QToolButton(findRow);
    findNextButton->setIcon(shellStandardIcon(QStyle::SP_ArrowDown));
    findNextButton->setToolTip(tr("Next match in PDF"));
    connect(findNextButton, &QToolButton::clicked, this, &ShellMainWindow::onPreviewFindNext);

    previewFindStatus_ = new QLabel(tr("—"), findRow);
    previewFindStatus_->setMinimumWidth(72);

    findLayout->addWidget(findLabel);
    findLayout->addWidget(previewFindField_, /*stretch*/ 1);
    findLayout->addWidget(findPrevButton);
    findLayout->addWidget(findNextButton);
    findLayout->addWidget(previewFindStatus_);

    pdfPreview_ = new pdf_document_view::PdfDocumentViewWidget(previewColumn_);
    pdfPreview_->setMinimumWidth(280);
    connect(pdfPreview_, &pdf_document_view::PdfDocumentViewWidget::documentOpened, this,
            [this](bool ok) {
                if (pdfPreview_ == nullptr) {
                    return;
                }
                if (previewFindField_ != nullptr) {
                    previewFindField_->setEnabled(ok);
                }
                if (findInPreviewAction_ != nullptr) {
                    findInPreviewAction_->setEnabled(ok);
                }
                if (ok) {
                    pdfPreview_->fitWidth();
                } else {
                    if (previewFindField_ != nullptr) {
                        previewFindField_->clear();
                    }
                    pdfPreview_->clearFind();
                    refreshPreviewFindUi();
                    statusBar()->showMessage(
                        tr("PDF preview: %1").arg(pdfPreview_->documentError()), 5000);
                }
            });
    connect(pdfPreview_, &pdf_document_view::PdfDocumentViewWidget::viewStateChanged, this,
            &ShellMainWindow::onPreviewViewStateChanged);
    connect(pdfPreview_, &pdf_document_view::PdfDocumentViewWidget::findResultsChanged, this,
            [this](int, int) { refreshPreviewFindUi(); });

    previewLayout->addWidget(findRow);
    previewLayout->addWidget(pdfPreview_, /*stretch*/ 1);

    pageText_ = new QTextEdit(split);
    pageText_->setMinimumWidth(320);
    pageText_->setPlaceholderText(tr("Page text (pages/NNNN.txt)"));

    split->addWidget(pageList_);
    split->addWidget(previewColumn_);
    split->addWidget(pageText_);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);
    split->setStretchFactor(2, 1);

    statusBar()->showMessage(tr("Open a PDF to initialize a review session."));
}

void ShellMainWindow::wireFacade() {
    // Facade is the only path to domain services (see DD §4.1 HLA-UI).
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::sessionChanged, this, &ShellMainWindow::refreshSessionUi);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::currentPageChanged, this, &ShellMainWindow::refreshPageUi);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::currentPageTextChanged, this, &ShellMainWindow::refreshPageText);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::statusMessageChanged, this, &ShellMainWindow::refreshStatus);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::sessionChanged, this, &ShellMainWindow::syncPdfPreviewWidget);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::currentPageChanged, this, &ShellMainWindow::syncPdfPreviewWidget);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::reviewSyncChanged, this,
            &ShellMainWindow::refreshReviewSync);
    refreshReviewSync();
}

void ShellMainWindow::onOpenDocumentation() {
    const QString guidePath = operatorGuidePath();
    if (guidePath.isEmpty()) {
        QMessageBox::warning(this, tr("Documentation"),
                             tr("User guide not found. See docs/shell-user-guide.md in the repository."));
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(guidePath))) {
        QMessageBox::warning(this, tr("Documentation"),
                             tr("Could not open %1").arg(guidePath));
    }
}

void ShellMainWindow::onAbout() {
    const QString version = QApplication::applicationVersion();
    QMessageBox::about(
        this,
        tr("About PDF Text Extractor"),
        tr("PDF Text Extractor\nVersion %1\n\nLocal PDF text extraction and review shell.\n"
           "PDF preview powered by PDFDocumentView (PDFium).\n\n"
           "See Help → Documentation for operator guidance.")
            .arg(version.isEmpty() ? QStringLiteral("unknown") : version));
}

void ShellMainWindow::onCheckExtractionTools() {
    if (facade_ == nullptr) {
        return;
    }
    QMessageBox::information(this, tr("Extraction tools"), facade_->extractionToolsReport());
}

void ShellMainWindow::onPreviewFitWidth() {
    if (pdfPreview_ != nullptr) {
        pdfPreview_->fitWidth();
    }
}

void ShellMainWindow::onPreviewResetZoom() {
    if (pdfPreview_ != nullptr) {
        pdfPreview_->resetZoom();
    }
}

void ShellMainWindow::onFocusPreviewFind() {
    if (previewFindField_ == nullptr || !previewFindField_->isEnabled()) {
        statusBar()->showMessage(tr("Open a PDF to search the preview."), 2500);
        return;
    }
    previewFindField_->setFocus(Qt::ShortcutFocusReason);
    previewFindField_->selectAll();
}

void ShellMainWindow::onPreviewFindTriggered() {
    if (pdfPreview_ == nullptr || previewFindField_ == nullptr) {
        return;
    }
    const QString needle = previewFindField_->text();
    if (needle.trimmed().isEmpty()) {
        pdfPreview_->clearFind();
        refreshPreviewFindUi();
        return;
    }
    pdfPreview_->findText(needle);
    refreshPreviewFindUi();
    const int count = pdfPreview_->findMatchCount();
    if (count <= 0) {
        statusBar()->showMessage(tr("No matches in PDF."), 2500);
    }
}

void ShellMainWindow::onPreviewFindNext() {
    if (pdfPreview_ == nullptr || pdfPreview_->findMatchCount() <= 0) {
        return;
    }
    pdfPreview_->findNext();
    refreshPreviewFindUi();
}

void ShellMainWindow::onPreviewFindPrevious() {
    if (pdfPreview_ == nullptr || pdfPreview_->findMatchCount() <= 0) {
        return;
    }
    pdfPreview_->findPrevious();
    refreshPreviewFindUi();
}

void ShellMainWindow::refreshPreviewFindUi() {
    const bool hasDoc = pdfPreview_ != nullptr && pdfPreview_->isDocumentOpen();
    const int count = hasDoc ? pdfPreview_->findMatchCount() : 0;
    const int cur = hasDoc ? pdfPreview_->findCurrentIndex() : -1;

    if (findNextInPreviewAction_ != nullptr) {
        findNextInPreviewAction_->setEnabled(count > 0);
    }
    if (findPrevInPreviewAction_ != nullptr) {
        findPrevInPreviewAction_->setEnabled(count > 0);
    }
    if (previewFindStatus_ != nullptr) {
        if (count <= 0 || cur < 0) {
            previewFindStatus_->setText(tr("—"));
        } else {
            previewFindStatus_->setText(tr("%1 / %2").arg(cur + 1).arg(count));
        }
    }
}

void ShellMainWindow::onPreviewViewStateChanged() {
    if (pdfPreview_ == nullptr || facade_ == nullptr) {
        return;
    }
    const int previewIdx = pdfPreview_->currentPageIndex();
    const int facadeIdx = facade_->currentPageIndex();
    if (previewIdx >= 0 && previewIdx != facadeIdx) {
        facade_->selectPage(previewIdx);
    }
}

void ShellMainWindow::onOpenPdf() {
    if (windowHandle() != nullptr) {
        windowHandle()->requestActivate();
    }
    raise();
    activateWindow();
    QApplication::processEvents(QEventLoop::AllEvents);

    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open source PDF"));
    dialog.setModal(true);
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(tr("PDF files (*.pdf);;All files (*)"));
#if defined(Q_OS_MACOS)
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
#endif

    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(tr("Open canceled."));
        return;
    }
    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty()) {
        statusBar()->showMessage(tr("No file selected."));
        return;
    }
    facade_->openPdf(files.constFirst(), QString());
}

void ShellMainWindow::onEditVolumeMetadata() {
    const QString folder = facade_->workFolderPath();
    if (folder.isEmpty()) {
        return;
    }
    const std::filesystem::path workFolder(folder.toStdString());
    pte::ui::VolumeMetadataDialog dialog(this, workFolder);
    if (dialog.exec() == QDialog::Accepted) {
        facade_->refreshAfterVolumeMetadataSave();
        statusBar()->showMessage(tr("Volume metadata saved."));
    }
}

void ShellMainWindow::onSavePageText() {
    if (facade_->currentPageIndex() < 0) {
        return;
    }
    facade_->saveCurrentPageText(pageText_->toPlainText());
}

void ShellMainWindow::onReextractEmbeddedCandidates() {
    facade_->reextractEmbeddedCandidates();
}

void ShellMainWindow::onThemeTriggered(QAction* action) {
    if (action == nullptr) {
        return;
    }
    const QString id = action->data().toString();
    pte::ui::saveThemeId(id);
    pte::ui::applyTheme(qApp, id);
    statusBar()->showMessage(
        tr("Theme preference saved for this user on this computer."), 2500);
}

void ShellMainWindow::onReadinessSummary() {
    const QString folder = facade_->workFolderPath();
    if (folder.isEmpty()) {
        return;
    }
    const std::filesystem::path workFolder(folder.toStdString());
    const pte::core::ReadinessSummary summary =
        pte::core::ReadinessSummaryService().summarizeWorkFolder(workFolder);
    QString message = QString::fromStdString(summary.safeMessage);
    if (summary.success) {
        message += QLatin1Char('\n');
        for (const auto& entry : summary.statusCounts) {
            message += QStringLiteral("status %1: %2\n")
                           .arg(QString::fromStdString(entry.first))
                           .arg(entry.second);
        }
        for (const auto& entry : summary.selectedSourceCounts) {
            message += QStringLiteral("source %1: %2\n")
                           .arg(QString::fromStdString(entry.first))
                           .arg(entry.second);
        }
        message += QStringLiteral("ocrUsed: %1\n").arg(summary.ocrUsedCount);
        message += QStringLiteral("validationErrors: %1\n").arg(summary.validationErrorCount);
    }
    QMessageBox::information(this, tr("Readiness summary"), message);
}

void ShellMainWindow::onPageListRowChanged(int row) {
    if (row < 0 || row == facade_->currentPageIndex()) {
        return;
    }
    facade_->selectPage(row);
}

void ShellMainWindow::onFirstPage() {
    if (facade_->pageCount() > 0) {
        facade_->selectPage(0);
    }
}

void ShellMainWindow::onPrevPage() {
    const int i = facade_->currentPageIndex();
    if (i > 0) {
        facade_->selectPage(i - 1);
    }
}

void ShellMainWindow::onNextPage() {
    const int i = facade_->currentPageIndex();
    if (i >= 0 && i + 1 < facade_->pageCount()) {
        facade_->selectPage(i + 1);
    }
}

void ShellMainWindow::onLastPage() {
    const int n = facade_->pageCount();
    if (n > 0) {
        facade_->selectPage(n - 1);
    }
}

void ShellMainWindow::refreshSessionUi() {
    if (pageList_ == nullptr) {
        return;
    }
    const QSignalBlocker blocker(pageList_);
    pageList_->clear();
    for (const QString& id : facade_->pageIds()) {
        pageList_->addItem(id);
    }
    if (facade_->pageCount() > 0 && facade_->currentPageIndex() >= 0) {
        pageList_->setCurrentRow(facade_->currentPageIndex());
    }
    refreshPageUi();
    refreshPageText();
}

void ShellMainWindow::refreshPageUi() {
    if (pageList_ == nullptr || pageLabel_ == nullptr) {
        return;
    }
    if (savePageAction_ != nullptr) {
        savePageAction_->setEnabled(facade_->currentPageIndex() >= 0);
    }
    if (editVolumeMetadataAction_ != nullptr) {
        editVolumeMetadataAction_->setEnabled(!facade_->workFolderPath().isEmpty());
    }
    if (reextractEmbeddedAction_ != nullptr) {
        reextractEmbeddedAction_->setEnabled(!facade_->workFolderPath().isEmpty());
    }
    if (readinessSummaryAction_ != nullptr) {
        readinessSummaryAction_->setEnabled(!facade_->workFolderPath().isEmpty());
    }
    const int n = facade_->pageCount();
    const int idx = facade_->currentPageIndex();
    if (firstPageAction_ != nullptr && prevPageAction_ != nullptr && nextPageAction_ != nullptr
        && lastPageAction_ != nullptr) {
        const bool hasPages = n > 0 && idx >= 0;
        firstPageAction_->setEnabled(hasPages && idx > 0);
        prevPageAction_->setEnabled(hasPages && idx > 0);
        nextPageAction_->setEnabled(hasPages && idx < n - 1);
        lastPageAction_->setEnabled(hasPages && idx < n - 1);
    }
    {
        const QSignalBlocker blocker(pageList_);
        if (idx >= 0 && idx < pageList_->count()) {
            pageList_->setCurrentRow(idx);
        }
    }
    if (n > 0 && idx >= 0) {
        pageLabel_->setText(tr("Page %1 / %2").arg(idx + 1).arg(n));
    } else {
        pageLabel_->setText(tr("Page — / —"));
    }
    refreshReviewSync();
    syncPdfPreviewWidget();
}

void ShellMainWindow::syncPdfPreviewWidget() {
    if (pdfPreview_ == nullptr || facade_ == nullptr) {
        return;
    }
    const QString pdfPath = facade_->sourcePdfPath();
    if (pdfPath.isEmpty()) {
        return;
    }
    if (pdfPreview_->documentPath() != pdfPath || !pdfPreview_->isDocumentOpen()) {
        if (previewFindField_ != nullptr) {
            previewFindField_->clear();
        }
        pdfPreview_->clearFind();
        refreshPreviewFindUi();
        pdfPreview_->setDocumentPath(pdfPath);
    }
    const int idx = facade_->currentPageIndex();
    if (idx >= 0 && idx < pdfPreview_->pageCount() && pdfPreview_->currentPageIndex() != idx) {
        pdfPreview_->setCurrentPageIndex(idx);
    }
}

void ShellMainWindow::refreshReviewSync() {
    if (reviewSyncLabel_ != nullptr && facade_ != nullptr) {
        reviewSyncLabel_->setText(facade_->reviewSyncSummary());
    }
}

void ShellMainWindow::refreshPageText() {
    if (pageText_ == nullptr) {
        return;
    }
    const QSignalBlocker blocker(pageText_);
    pageText_->setPlainText(facade_->currentPageText());
}

void ShellMainWindow::refreshStatus() {
    statusBar()->showMessage(facade_->statusMessage());
}
