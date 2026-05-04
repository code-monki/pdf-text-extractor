#include "ui/shell_main_window.hpp"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QEventLoop>
#include <QFileDialog>
#include <QFrame>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

#include <QtGlobal>

ShellMainWindow::ShellMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , facade_(std::make_unique<pte::ui::ReviewSessionFacade>(this)) {
    setWindowTitle(tr("PDF Text Extractor"));
    buildUi();
    wireFacade();
}

ShellMainWindow::~ShellMainWindow() = default;

void ShellMainWindow::buildUi() {
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* openAct = new QAction(tr("&Open PDF…"), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &ShellMainWindow::onOpenPdf);
    fileMenu->addAction(openAct);
    savePageAction_ = new QAction(tr("&Save Page Text"), this);
    savePageAction_->setEnabled(false);
    connect(savePageAction_, &QAction::triggered, this, &ShellMainWindow::onSavePageText);
    fileMenu->addAction(savePageAction_);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), QKeySequence::Quit, this, &QWidget::close);

    auto* toolBar = addToolBar(tr("Main"));
    toolBar->setMovable(false);
    toolBar->addAction(openAct);

    toolBar->addAction(savePageAction_);

    auto* prevAct = new QAction(tr("‹ Prev"), this);
    connect(prevAct, &QAction::triggered, this, &ShellMainWindow::onPrevPage);
    toolBar->addAction(prevAct);

    pageLabel_ = new QLabel(tr("Page — / —"), this);
    toolBar->addWidget(pageLabel_);

    auto* nextAct = new QAction(tr("Next ›"), this);
    connect(nextAct, &QAction::triggered, this, &ShellMainWindow::onNextPage);
    toolBar->addAction(nextAct);

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
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::sessionChanged, this, &ShellMainWindow::refreshSessionUi);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::currentPageChanged, this, &ShellMainWindow::refreshPageUi);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::currentPageTextChanged, this, &ShellMainWindow::refreshPageText);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::statusMessageChanged, this, &ShellMainWindow::refreshStatus);
    connect(facade_.get(), &pte::ui::ReviewSessionFacade::previewPixmapChanged, this,
            &ShellMainWindow::refreshPreviewColumn);
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

void ShellMainWindow::onSavePageText() {
    if (facade_->currentPageIndex() < 0) {
        return;
    }
    facade_->saveCurrentPageText(pageText_->toPlainText());
}

void ShellMainWindow::onPageListRowChanged(int row) {
    if (row < 0 || row == facade_->currentPageIndex()) {
        return;
    }
    facade_->selectPage(row);
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
    if (pageList_ == nullptr || pageLabel_ == nullptr || preview_ == nullptr) {
        return;
    }
    if (savePageAction_ != nullptr) {
        savePageAction_->setEnabled(facade_->currentPageIndex() >= 0);
    }
    const int n = facade_->pageCount();
    const int idx = facade_->currentPageIndex();
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
