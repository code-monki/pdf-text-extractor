/**
 * @file shell_main_window.cpp
 * @brief Implements @ref ShellMainWindow — menus, icon toolbar, and facade-driven refresh logic.
 *
 * Toolbar pixmap choices and operator help text are summarized in docs/shell-user-guide.md.
 */

#include "ui/app_theme.hpp"
#include "ui/shell_main_window.hpp"
#include "ui/volume_metadata_dialog.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QEventLoop>
#include <QFileDialog>
#include <QFrame>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSize>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QToolBar>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

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

    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(editVolumeMetadataAction_);
    editMenu->addAction(reextractEmbeddedAction_);
    editMenu->addAction(readinessSummaryAction_);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
    reviewSyncLabel_->setElideMode(Qt::ElideMiddle);
#endif
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

    preview_ = new QLabel(tr("PDF page preview"), split);
    preview_->setMinimumWidth(280);
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setFrameShape(QFrame::StyledPanel);

    pageText_ = new QTextEdit(split);
    pageText_->setMinimumWidth(320);
    pageText_->setPlaceholderText(tr("Page text (pages/NNNN.txt)"));

    split->addWidget(pageList_);
    split->addWidget(preview_);
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
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::previewPixmapChanged, this,
            &ShellMainWindow::refreshPreviewColumn);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::reviewSyncChanged, this,
            &ShellMainWindow::refreshReviewSync);
    refreshReviewSync();
}

void ShellMainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    applyPreviewScale();
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
    // Navigation actions: disabled when no session page; first/prev off at idx 0; next/last off at last page.
    if (pageList_ == nullptr || pageLabel_ == nullptr || preview_ == nullptr) {
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
    refreshPreviewColumn();
    refreshReviewSync();
}

void ShellMainWindow::refreshReviewSync() {
    if (reviewSyncLabel_ != nullptr && facade_ != nullptr) {
        reviewSyncLabel_->setText(facade_->reviewSyncSummary());
    }
}

void ShellMainWindow::refreshPreviewColumn() {
    if (preview_ == nullptr || facade_ == nullptr) {
        return;
    }
    const QPixmap px = facade_->currentPreviewPixmap();
    if (!px.isNull()) {
        preview_->setText(QString());
        previewSource_ = px;
        applyPreviewScale();
        return;
    }
    preview_->setPixmap(QPixmap());
    previewSource_ = QPixmap();
    const int n = facade_->pageCount();
    const int idx = facade_->currentPageIndex();
    if (n > 0 && idx >= 0) {
        preview_->setText(
            tr("Preview unavailable\n(page %1)\nInstall Poppler pdftoppm on PATH.")
                .arg(facade_->currentPageId()));
    } else {
        preview_->setText(tr("PDF page preview"));
    }
}

void ShellMainWindow::applyPreviewScale() {
    if (preview_ == nullptr || previewSource_.isNull()) {
        return;
    }
    const QSize size = preview_->contentsRect().size();
    if (size.width() <= 0 || size.height() <= 0) {
        return;
    }
    preview_->setPixmap(
        previewSource_.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    preview_->setAlignment(Qt::AlignCenter);
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
