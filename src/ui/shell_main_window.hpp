#pragma once

/**
 * @file shell_main_window.hpp
 * @brief Main Qt Widgets window for the `pte_shell` reviewer application.
 *
 * @see pte::ui::ReviewSessionFacade for session and extraction orchestration.
 * @see docs/shell-user-guide.md for operator-facing help (intended target for Help → Documentation).
 */

#include <QMainWindow>
#include <QPixmap>
#include <memory>

#include "ui/review_session_facade.hpp"

class QAction;
class QActionGroup;
class QLabel;
class QListWidget;
class QResizeEvent;
class QTextEdit;

/**
 * @brief Primary UI shell: menus, toolbar, page list, PDF preview, and page text editor.
 *
 * @details Implements FR-028 / NFR-009 (native Widgets presentation). Business logic and file I/O
 *          are delegated to ReviewSessionFacade; this class only wires signals, enables actions,
 *          and keeps the preview scaled. The main toolbar uses @c Qt::ToolButtonIconOnly with
 *          QStyle standard icons; human-readable strings live in @c QAction tooltips and menu text.
 */
class ShellMainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit ShellMainWindow(QWidget* parent = nullptr);
    ~ShellMainWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /** @brief Opens QFileDialog and passes the chosen PDF to the facade. */
    void onOpenPdf();
    /** @brief Modal VolumeMetadataDialog over the current work folder; refreshes facade on accept. */
    void onEditVolumeMetadata();
    /** @brief Persists editor plain text for the current page via the facade. */
    void onSavePageText();
    /** @brief Runs full-volume embedded candidate re-extraction (Poppler path). */
    void onReextractEmbeddedCandidates();
    /** @brief Shows ReadinessSummaryService output in a message box (safe text only). */
    void onReadinessSummary();
    /** @brief Persists theme id and applies palette through app_theme helpers. */
    void onThemeTriggered(QAction* action);
    /** @brief Syncs facade page selection when the list selection changes. */
    void onPageListRowChanged(int row);
    /** @brief Jumps to the first page (index 0). */
    void onFirstPage();
    /** @brief Moves to the previous page if not already at the start. */
    void onPrevPage();
    /** @brief Moves to the next page if not already at the end. */
    void onNextPage();
    /** @brief Jumps to the last page. */
    void onLastPage();
    /** @brief Repopulates the page list and refreshes page-dependent widgets after sessionChanged. */
    void refreshSessionUi();
    /** @brief Updates actions, page label, list highlight, preview, and review line after page change. */
    void refreshPageUi();
    /** @brief Reloads the editor from facade currentPageText (signal-driven). */
    void refreshPageText();
    /** @brief Mirrors facade statusMessage into the status bar. */
    void refreshStatus();
    /** @brief Shows pixmap preview or an unavailable hint when Poppler preview is missing. */
    void refreshPreviewColumn();
    /** @brief Updates the toolbar review-sync summary label (NFR-006). */
    void refreshReviewSync();

private:
    /** @brief Creates menus, icon toolbar, central splitter, and initial widget state. */
    void buildUi();
    /** @brief Connects ReviewSessionFacade signals to refresh slots. */
    void wireFacade();
    /** @brief Scales the cached preview pixmap to the preview label’s contents rect. */
    void applyPreviewScale();

    std::unique_ptr<pte::ui::ReviewSessionFacade> facade_; ///< Session and I/O boundary (HLA-UI).
    QListWidget* pageList_{};                             ///< Page IDs; drives selectPage on change.
    QLabel* preview_{};                                   ///< Raster preview or placeholder text.
    QTextEdit* pageText_{};                               ///< Reviewed / candidate page text editor.
    QLabel* pageLabel_{};                                 ///< “Page i / n” in the toolbar.
    QLabel* reviewSyncLabel_{};                           ///< Facade reviewSyncSummary() line.
    QAction* savePageAction_{};                          ///< Shared File menu + toolbar.
    QAction* reextractEmbeddedAction_{};                 ///< Shared File + Edit + toolbar.
    QAction* readinessSummaryAction_{};                   ///< Shared File + Edit + toolbar.
    QAction* editVolumeMetadataAction_{};                ///< Shared File + Edit + toolbar; Ctrl+M.
    QAction* firstPageAction_{};                         ///< Toolbar only; Ctrl+Home.
    QAction* prevPageAction_{};                           ///< Toolbar only.
    QAction* nextPageAction_{};                           ///< Toolbar only.
    QAction* lastPageAction_{};                           ///< Toolbar only; Ctrl+End.
    QActionGroup* themeActionGroup_{};                    ///< View → Theme exclusive choices.
    QPixmap previewSource_;                               ///< Unscaled pixmap before label scaling.
};
