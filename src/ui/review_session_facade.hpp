#pragma once

/**
 * @file review_session_facade.hpp
 * @brief Facade between Qt Widgets UI and core extraction/review services (HLA-UI).
 */

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <filesystem>
#include <memory>

namespace pte::ui {

/**
 * @brief Exposes one PDF review session to QML-free Widgets UI code.
 *
 * @details Owns session state (paths, page list, current index, preview pixmap cache). Emits Qt
 *          signals when data changes so ShellMainWindow can refresh without polling. Does not
 *          render widgets.
 *
 * @note Full behavioral contracts are in detailed-design HLA-UI and core services; see also
 *       docs/shell-user-guide.md for operator-visible workflows.
 */
class ReviewSessionFacade : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString sourcePdfPath READ sourcePdfPath NOTIFY sessionChanged)
    Q_PROPERTY(QString workFolderPath READ workFolderPath NOTIFY sessionChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY sessionChanged)
    Q_PROPERTY(QStringList pageIds READ pageIds NOTIFY sessionChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageId READ currentPageId NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentPageText READ currentPageText NOTIFY currentPageTextChanged)
    Q_PROPERTY(QString reviewStatus READ reviewStatus NOTIFY reviewSyncChanged)
    Q_PROPERTY(QString selectedSource READ selectedSource NOTIFY reviewSyncChanged)
    Q_PROPERTY(QString printedPageLabel READ printedPageLabel NOTIFY reviewSyncChanged)
    Q_PROPERTY(QString volumeTitle READ volumeTitle NOTIFY reviewSyncChanged)

public:
    explicit ReviewSessionFacade(QObject* parent = nullptr);

    /** @brief Last user-facing status line for the status bar. */
    QString statusMessage() const;
    /** @brief Absolute path of the source PDF for the active session (empty if none). */
    QString sourcePdfPath() const;
    /** @brief Root work folder for the active volume (empty if none). */
    QString workFolderPath() const;
    /** @brief Number of pages in the current PDF session. */
    int pageCount() const;
    /** @brief Stable page identifiers for list display (e.g. zero-padded indices). */
    QStringList pageIds() const;
    /** @brief Zero-based index of the selected page, or -1. */
    int currentPageIndex() const;
    /** @brief Current page id string, suitable for diagnostics and labels. */
    QString currentPageId() const;
    /** @brief Text shown in the page editor (reviewed file or embedded candidate fill). */
    QString currentPageText() const;
    /** @brief Raster preview for the current page when Poppler rendering succeeded. */
    QPixmap currentPreviewPixmap() const;
    /** @brief Raw review status token for the current page. */
    QString reviewStatus() const;
    /** @brief Selected extraction source label for the current page. */
    QString selectedSource() const;
    /** @brief Printed page label from volume metadata when defined. */
    QString printedPageLabel() const;
    /** @brief Volume title from metadata for lightweight chrome. */
    QString volumeTitle() const;
    /** @brief One-line summary for the shell toolbar (NFR-006). */
    QString reviewSyncSummary() const;

    /**
     * @brief Opens or resumes a session for `pdfPath`, optionally forcing a work-folder path.
     * @return Whether the facade entered a consistent session state (caller may check statusMessage).
     */
    Q_INVOKABLE bool openPdf(const QString& pdfPath, const QString& requestedWorkFolder);
    /**
     * @brief Selects page by index and reloads text/preview as needed.
     * @return False if index is out of range or no session.
     */
    Q_INVOKABLE bool selectPage(int index);
    /**
     * @brief Writes reviewed page text through core services (conflict and normalization rules apply).
     */
    Q_INVOKABLE bool saveCurrentPageText(const QString& text);
    /** @brief Reloads metadata-dependent fields after VolumeMetadataDialog saves `volume.json` (FR-006). */
    Q_INVOKABLE void refreshAfterVolumeMetadataSave();
    /**
     * @brief Re-runs Poppler embedded candidate extraction for all pages (does not clear reviewed
     *        `pages/*.txt` files). No-op if no work folder or volume metadata.
     */
    Q_INVOKABLE bool reextractEmbeddedCandidates();

signals:
    void statusMessageChanged();
    void sessionChanged();
    void currentPageChanged();
    void currentPageTextChanged();
    void previewPixmapChanged();
    void reviewSyncChanged();

private:
    /** @brief Resolved filesystem path for the current page’s reviewed text file. */
    std::filesystem::path currentPagePath() const;
    /** @brief Reloads currentPageText_ from disk or embedded candidate policy. */
    bool reloadCurrentPageText();
    /** @brief Regenerates previewPixmap_ via Poppler raster helper when dependencies exist. */
    void refreshPreviewRaster();
    /** @brief Reloads review-status-derived fields for reviewSyncSummary(). */
    void reloadReviewSync();
    /** @brief Derives a stable volume directory name from the PDF filename stem. */
    static std::string makeVolumeId(const std::string& filenameStem);
    /** @brief Default sibling work-folder path for a given PDF (implementation-defined layout). */
    static std::filesystem::path defaultWorkFolderForPdf(const std::filesystem::path& pdfPath);

    QString statusMessage_;
    QString sourcePdfPath_;
    QString workFolderPath_;
    QStringList pageIds_;
    int currentPageIndex_ = -1;
    QString currentPageText_;
    QPixmap previewPixmap_;
    std::unique_ptr<QTemporaryDir> previewTempDir_;
    QString reviewStatus_;
    QString selectedSource_;
    QString printedPageLabel_;
    QString volumeTitle_;
    /** True when the editor text came from embedded candidates because pages/N.txt was blank. */
    bool lastFillWasEmbeddedCandidate_{false};
};

} // namespace pte::ui
