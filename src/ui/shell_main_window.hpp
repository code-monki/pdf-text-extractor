#pragma once

/**
 * @file shell_main_window.hpp
 * @brief Main Qt Widgets window for the `pte_shell` reviewer application.
 *
 * @see pte::ui::ReviewSessionFacade for session and extraction orchestration.
 * @see docs/shell-user-guide.md for operator-facing help (Help → Documentation).
 */

#include <QMainWindow>
#include <QPointer>
#include <optional>

#include "ui/link_map_editor_dialog.hpp"
#include "ui/review_session_facade.hpp"

class QAction;
class QActionGroup;
class QLabel;
class QLineEdit;
class QListWidget;
class QRubberBand;
class QTextEdit;
class QWidget;

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

    /** @brief Refreshes preview overlays after in-memory link-map edits. */
    void refreshLinkMapEditorHighlights();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onOpenPdf();
    void onEditVolumeMetadata();
    void onSavePageText();
    void onReextractEmbeddedCandidates();
    void onReadinessSummary();
    void onThemeTriggered(QAction* action);
    void onOpenDocumentation();
    void onAbout();
    void onCheckExtractionTools();
    void onPreviewFitWidth();
    void onPreviewResetZoom();
    void onPreviewFindTriggered();
    void onPreviewFindNext();
    void onPreviewFindPrevious();
    void onFocusPreviewFind();
    void onPreviewViewStateChanged();
    void refreshPreviewFindUi();
    void syncLinkMapHighlights();
    void onLinkMapOverlaysToggled(bool enabled);
    void onEditTocLinks();
    void onLinkMapEditorClosed();
    void onLinkMapInteractionModeChanged(int mode);
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
    void setLinkMapInteractionMode(pte::ui::LinkMapInteractionMode mode);
    void finishLinkMapRectDrag(const QPoint& viewportPos);
    bool mapPreviewViewportToPagePoint(const QPoint& viewportPos, double& topDownX, double& topDownY) const;
    void topDownRectToPdfUser(const QRectF& topDownRect,
                              double pageHeightPt,
                              double outRect[4]) const;

    std::unique_ptr<pte::ui::ReviewSessionFacade> facade_;
    QListWidget* pageList_{};
    QWidget* previewColumn_{};
    QLineEdit* previewFindField_{};
    QLabel* previewFindStatus_{};
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
    QAction* findInPreviewAction_{};
    QAction* findNextInPreviewAction_{};
    QAction* findPrevInPreviewAction_{};
    QAction* showLinkMapOverlaysAction_{};
    QAction* editTocLinksAction_{};
    pte::ui::LinkMapInteractionMode linkMapInteractionMode_{pte::ui::LinkMapInteractionMode::None};
    QPointer<pte::ui::LinkMapEditorDialog> linkMapEditor_;
    QRubberBand* linkMapRubberBand_{};
    QPoint linkMapDragOrigin_;
    bool linkMapDragging_{false};
    std::optional<std::size_t> linkMapSelectedGlobalIndex_;
    QActionGroup* themeActionGroup_{};
};
