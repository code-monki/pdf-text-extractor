#pragma once

/**
 * @file link_map_editor_dialog.hpp
 * @brief Modeless TOC link-map editor (FR-035/036 shell editing slice).
 */

#include "core/link_map_store.hpp"

#include <QDialog>
#include <QPointer>

class ShellMainWindow;
class QLabel;
class QListWidget;
class QPushButton;

namespace pte::ui {

class ReviewSessionFacade;

/** @brief Preview interaction modes coordinated with @c ShellMainWindow. */
enum class LinkMapInteractionMode {
    None = 0,
    DrawSourceRect = 1,
    PickExistingSource = 2,
    CaptureDestination = 3,
};

/**
 * @brief Non-modal editor for intra-document TOC links on the active work folder.
 *
 * @details Stays open while the operator navigates the main window to pick destination pages.
 *          Rectangle drawing and hit-testing are handled by @c ShellMainWindow on the PDF preview.
 */
class LinkMapEditorDialog final : public QDialog {
    Q_OBJECT

public:
    explicit LinkMapEditorDialog(::ShellMainWindow* shell, ReviewSessionFacade* facade);

    void reloadFromFacade();
    void setPendingSourceRect(const pte::core::LinkMapEntry& entry);
    void selectLinkByGlobalIndex(int globalIndex);
    void setCapturedDestinationPage(int pageIndex);

signals:
    void interactionModeChanged(int mode);
    void editorClosed();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onDrawRectClicked();
    void onPickExistingClicked();
    void onCaptureDestinationClicked();
    void onApplyClicked();
    void onDeleteClicked();
    void onSaveClicked();
    void onLinkListSelectionChanged();
    void onFacadePageChanged();

private:
    void rebuildLinkList();
    void refreshFields();
    std::optional<std::size_t> selectedGlobalIndex() const;
    pte::core::LinkMapEntry buildEntryFromForm() const;

    class ::ShellMainWindow* shell_;
    ReviewSessionFacade* facade_;
    QListWidget* linkList_{};
    QLabel* sourcePageLabel_{};
    QLabel* destinationLabel_{};
    QLabel* statusLabel_{};
    QPushButton* drawRectButton_{};
    QPushButton* pickExistingButton_{};
    QPushButton* captureDestinationButton_{};
    QPushButton* applyButton_{};
    QPushButton* deleteButton_{};
    QPushButton* saveButton_{};
    bool hasPendingRect_{false};
    pte::core::LinkMapEntry pendingRect_{};
    int capturedDestinationPage_{-1};
    bool captureDestinationActive_{false};
};

} // namespace pte::ui
