#pragma once

/**
 * @file shell_main_window.hpp
 * @brief Main Qt Widgets window for the `pte_shell` reviewer application.
 *
 * @see pte::ui::ReviewSessionFacade for session and extraction orchestration.
 * @see docs/shell-user-guide.md for operator-facing help (Help → Documentation).
 */

#include <QMainWindow>
#include <memory>

#include "ui/review_session_facade.hpp"

class QAction;
class QActionGroup;
class QLabel;
class QListWidget;
class QTextEdit;

namespace pdf_document_view {
class PdfDocumentViewWidget;
}

/**
 * @brief Primary UI shell: menus, toolbar, page list, PDF preview, and page text editor.
 *
 * @details Implements FR-028 / NFR-009 (native Widgets presentation). Business logic and file I/O
 *          are delegated to ReviewSessionFacade; PDF preview uses embedded PDFDocumentView (PDFium).
 *          The main toolbar uses @c Qt::ToolButtonIconOnly with QStyle standard icons.
 */
class ShellMainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit ShellMainWindow(QWidget* parent = nullptr);
    ~ShellMainWindow() override;

private slots:
    void onOpenPdf();
    void onEditVolumeMetadata();
    void onSavePageText();
    void onReextractEmbeddedCandidates();
    void onReadinessSummary();
    void onThemeTriggered(QAction* action);
    void onOpenDocumentation();
    void onAbout();
    void onPreviewFitWidth();
    void onPreviewResetZoom();
    void onPageListRowChanged(int row);
    void onFirstPage();
    void onPrevPage();
    void onNextPage();
    void onLastPage();
    void refreshSessionUi();
    void refreshPageUi();
    void refreshPageText();
    void refreshStatus();
    void syncPdfPreviewWidget();
    void refreshReviewSync();

private:
    void buildUi();
    void wireFacade();

    std::unique_ptr<pte::ui::ReviewSessionFacade> facade_;
    QListWidget* pageList_{};
    pdf_document_view::PdfDocumentViewWidget* pdfPreview_{};
    QTextEdit* pageText_{};
    QLabel* pageLabel_{};
    QLabel* reviewSyncLabel_{};
    QAction* savePageAction_{};
    QAction* reextractEmbeddedAction_{};
    QAction* readinessSummaryAction_{};
    QAction* editVolumeMetadataAction_{};
    QAction* firstPageAction_{};
    QAction* prevPageAction_{};
    QAction* nextPageAction_{};
    QAction* lastPageAction_{};
    QActionGroup* themeActionGroup_{};
};
