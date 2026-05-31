/**
 * @file link_map_editor_dialog.cpp
 * @brief Modeless TOC link editor — form UI; preview interaction is host-driven.
 */

#include "ui/link_map_editor_dialog.hpp"
#include "ui/review_session_facade.hpp"
#include "ui/shell_main_window.hpp"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace pte::ui {

namespace {

QString pageDisplayLabel(const ReviewSessionFacade* facade, int pageIndex) {
    if (facade == nullptr || pageIndex < 0) {
        return QObject::tr("—");
    }
    const QStringList ids = facade->pageIds();
    if (pageIndex >= 0 && pageIndex < ids.size()) {
        return QObject::tr("Page %1 (%2)").arg(pageIndex + 1).arg(ids.at(pageIndex));
    }
    return QObject::tr("Page %1").arg(pageIndex + 1);
}

} // namespace

LinkMapEditorDialog::LinkMapEditorDialog(::ShellMainWindow* shell, ReviewSessionFacade* facade)
    : QDialog(nullptr)
    , shell_(shell)
    , facade_(facade) {
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle(tr("TOC link editor"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(420, 480);

    auto* intro = new QLabel(
        tr("Draw or pick a rectangle on the PDF preview for the current source page, then navigate "
           "the main window and capture the destination page. The dialog stays open while you work."),
        this);
    intro->setWordWrap(true);

    sourcePageLabel_ = new QLabel(this);
    destinationLabel_ = new QLabel(this);
    statusLabel_ = new QLabel(this);
    statusLabel_->setWordWrap(true);

    linkList_ = new QListWidget(this);
    connect(linkList_, &QListWidget::currentRowChanged, this,
            &LinkMapEditorDialog::onLinkListSelectionChanged);

    drawRectButton_ = new QPushButton(tr("Draw rectangle on preview…"), this);
    pickExistingButton_ = new QPushButton(tr("Pick existing rectangle…"), this);
    captureDestinationButton_ = new QPushButton(tr("Capture destination from main window…"), this);
    applyButton_ = new QPushButton(tr("Apply link"), this);
    deleteButton_ = new QPushButton(tr("Delete selected"), this);
    saveButton_ = new QPushButton(tr("Save link-map.json"), this);
    auto* closeButton = new QPushButton(tr("Close"), this);

    connect(drawRectButton_, &QPushButton::clicked, this, &LinkMapEditorDialog::onDrawRectClicked);
    connect(pickExistingButton_, &QPushButton::clicked, this, &LinkMapEditorDialog::onPickExistingClicked);
    connect(captureDestinationButton_, &QPushButton::clicked, this,
            &LinkMapEditorDialog::onCaptureDestinationClicked);
    connect(applyButton_, &QPushButton::clicked, this, &LinkMapEditorDialog::onApplyClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &LinkMapEditorDialog::onDeleteClicked);
    connect(saveButton_, &QPushButton::clicked, this, &LinkMapEditorDialog::onSaveClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    auto* row1 = new QHBoxLayout();
    row1->addWidget(drawRectButton_);
    row1->addWidget(pickExistingButton_);

    auto* row2 = new QHBoxLayout();
    row2->addWidget(captureDestinationButton_);
    row2->addWidget(applyButton_);

    auto* row3 = new QHBoxLayout();
    row3->addWidget(deleteButton_);
    row3->addWidget(saveButton_);
    row3->addStretch();
    row3->addWidget(closeButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(intro);
    layout->addWidget(sourcePageLabel_);
    layout->addWidget(destinationLabel_);
    layout->addWidget(linkList_);
    layout->addLayout(row1);
    layout->addLayout(row2);
    layout->addLayout(row3);
    layout->addWidget(statusLabel_);

    if (facade_ != nullptr) {
        connect(facade_, &ReviewSessionFacade::currentPageChanged, this,
                &LinkMapEditorDialog::onFacadePageChanged);
        connect(facade_, &ReviewSessionFacade::sessionChanged, this,
                &LinkMapEditorDialog::reloadFromFacade);
    }

    reloadFromFacade();
}

void LinkMapEditorDialog::closeEvent(QCloseEvent* event) {
    emit interactionModeChanged(static_cast<int>(LinkMapInteractionMode::None));
    emit editorClosed();
    QDialog::closeEvent(event);
}

void LinkMapEditorDialog::reloadFromFacade() {
    if (facade_ != nullptr) {
        facade_->loadLinkMapForEditing();
    }
    hasPendingRect_ = false;
    captureDestinationActive_ = false;
    capturedDestinationPage_ = -1;
    rebuildLinkList();
    refreshFields();
}

void LinkMapEditorDialog::setPendingSourceRect(const pte::core::LinkMapEntry& entry) {
    hasPendingRect_ = true;
    pendingRect_ = entry;
    pendingRect_.manual = true;
    pendingRect_.targetType = "intra";
    capturedDestinationPage_ = -1;
    if (linkList_ != nullptr) {
        linkList_->clearSelection();
    }
    statusLabel_->setText(tr("Rectangle captured on %1. Set a destination page, then Apply.")
                              .arg(pageDisplayLabel(facade_, entry.pageIndex)));
    refreshFields();
}

void LinkMapEditorDialog::selectLinkByGlobalIndex(int globalIndex) {
    if (linkList_ == nullptr || globalIndex < 0) {
        return;
    }
    for (int row = 0; row < linkList_->count(); ++row) {
        if (linkList_->item(row)->data(Qt::UserRole).toInt() == globalIndex) {
            linkList_->setCurrentRow(row);
            return;
        }
    }
}

void LinkMapEditorDialog::setCapturedDestinationPage(int pageIndex) {
    capturedDestinationPage_ = pageIndex;
    captureDestinationActive_ = false;
    emit interactionModeChanged(static_cast<int>(LinkMapInteractionMode::None));
    statusLabel_->setText(tr("Destination set to %1.")
                              .arg(pageDisplayLabel(facade_, pageIndex)));
    refreshFields();
}

void LinkMapEditorDialog::onDrawRectClicked() {
    hasPendingRect_ = false;
    captureDestinationActive_ = false;
    if (linkList_ != nullptr) {
        linkList_->clearSelection();
    }
    statusLabel_->setText(tr("Drag a rectangle on the PDF preview for the current page."));
    emit interactionModeChanged(static_cast<int>(LinkMapInteractionMode::DrawSourceRect));
}

void LinkMapEditorDialog::onPickExistingClicked() {
    captureDestinationActive_ = false;
    statusLabel_->setText(tr("Click an existing overlay on the PDF preview."));
    emit interactionModeChanged(static_cast<int>(LinkMapInteractionMode::PickExistingSource));
}

void LinkMapEditorDialog::onCaptureDestinationClicked() {
    if (captureDestinationActive_) {
        if (facade_ == nullptr || facade_->currentPageIndex() < 0) {
            QMessageBox::warning(this, tr("TOC link editor"),
                                 tr("Select a destination page in the main window first."));
            return;
        }
        setCapturedDestinationPage(facade_->currentPageIndex());
        return;
    }
    captureDestinationActive_ = true;
    statusLabel_->setText(
        tr("Navigate using the page list or preview, then click Capture again."));
    emit interactionModeChanged(static_cast<int>(LinkMapInteractionMode::CaptureDestination));
    refreshFields();
}

void LinkMapEditorDialog::onApplyClicked() {
    if (facade_ == nullptr) {
        return;
    }

    pte::core::LinkMapEntry entry = buildEntryFromForm();
    if (entry.pageIndex < 0) {
        QMessageBox::warning(this, tr("TOC link editor"),
                             tr("Draw or pick a source rectangle on the preview first."));
        return;
    }
    if (entry.destinationPageIndex < 0) {
        QMessageBox::warning(this, tr("TOC link editor"),
                             tr("Capture a destination page from the main window first."));
        return;
    }

    const std::optional<std::size_t> selected = selectedGlobalIndex();
    if (!facade_->upsertLinkMapEntry(entry, selected)) {
        QMessageBox::warning(this, tr("TOC link editor"), facade_->linkMapEditStatus());
        return;
    }

    hasPendingRect_ = false;
    capturedDestinationPage_ = -1;
    rebuildLinkList();
    statusLabel_->setText(facade_->linkMapEditStatus());
    if (shell_ != nullptr) {
        shell_->refreshLinkMapEditorHighlights();
    }
}

void LinkMapEditorDialog::onDeleteClicked() {
    if (facade_ == nullptr) {
        return;
    }
    const std::optional<std::size_t> selected = selectedGlobalIndex();
    if (!selected.has_value()) {
        QMessageBox::information(this, tr("TOC link editor"), tr("Select a link to delete."));
        return;
    }
    if (!facade_->removeLinkMapEntry(*selected)) {
        QMessageBox::warning(this, tr("TOC link editor"), facade_->linkMapEditStatus());
        return;
    }
    hasPendingRect_ = false;
    rebuildLinkList();
    statusLabel_->setText(facade_->linkMapEditStatus());
    if (shell_ != nullptr) {
        shell_->refreshLinkMapEditorHighlights();
    }
}

void LinkMapEditorDialog::onSaveClicked() {
    if (facade_ == nullptr) {
        return;
    }
    if (!facade_->saveLinkMapDocument()) {
        QMessageBox::warning(this, tr("TOC link editor"), facade_->linkMapEditStatus());
        return;
    }
    statusLabel_->setText(facade_->linkMapEditStatus());
    if (shell_ != nullptr) {
        shell_->refreshLinkMapEditorHighlights();
    }
}

void LinkMapEditorDialog::onLinkListSelectionChanged() {
    if (facade_ == nullptr || linkList_ == nullptr) {
        return;
    }
    const std::optional<std::size_t> selected = selectedGlobalIndex();
    if (!selected.has_value()) {
        refreshFields();
        return;
    }
    const pte::core::LinkMapDocument& document = facade_->linkMapDocument();
    if (*selected >= document.links.size()) {
        return;
    }
    const pte::core::LinkMapEntry& entry = document.links[*selected];
    hasPendingRect_ = true;
    pendingRect_ = entry;
    capturedDestinationPage_ = entry.destinationPageIndex;
    refreshFields();
    if (shell_ != nullptr) {
        shell_->refreshLinkMapEditorHighlights();
    }
}

void LinkMapEditorDialog::onFacadePageChanged() {
    refreshFields();
    rebuildLinkList();
}

void LinkMapEditorDialog::rebuildLinkList() {
    if (linkList_ == nullptr || facade_ == nullptr) {
        return;
    }
    const int sourcePage = facade_->currentPageIndex();
    linkList_->clear();
    const pte::core::LinkMapDocument& document = facade_->linkMapDocument();
    for (std::size_t i = 0; i < document.links.size(); ++i) {
        const pte::core::LinkMapEntry& entry = document.links[i];
        if (entry.pageIndex != sourcePage) {
            continue;
        }
        QString dest = tr("no destination");
        if (entry.destinationPageIndex >= 0) {
            dest = pageDisplayLabel(facade_, entry.destinationPageIndex);
        }
        auto* item = new QListWidgetItem(
            tr("Link %1 → %2").arg(static_cast<int>(i) + 1).arg(dest));
        item->setData(Qt::UserRole, static_cast<int>(i));
        linkList_->addItem(item);
    }
}

void LinkMapEditorDialog::refreshFields() {
    if (facade_ == nullptr) {
        return;
    }
    sourcePageLabel_->setText(tr("Source page (main window): %1")
                                  .arg(pageDisplayLabel(facade_, facade_->currentPageIndex())));
    if (capturedDestinationPage_ >= 0) {
        destinationLabel_->setText(tr("Destination: %1")
                                       .arg(pageDisplayLabel(facade_, capturedDestinationPage_)));
    } else if (hasPendingRect_ && pendingRect_.destinationPageIndex >= 0) {
        destinationLabel_->setText(tr("Destination: %1")
                                       .arg(pageDisplayLabel(facade_, pendingRect_.destinationPageIndex)));
    } else {
        destinationLabel_->setText(tr("Destination: not set"));
    }

    const bool hasSession = facade_->currentPageIndex() >= 0;
    drawRectButton_->setEnabled(hasSession);
    pickExistingButton_->setEnabled(hasSession);
    captureDestinationButton_->setEnabled(hasSession);
    applyButton_->setEnabled(hasSession);
    deleteButton_->setEnabled(selectedGlobalIndex().has_value());
    saveButton_->setEnabled(!facade_->workFolderPath().isEmpty());

    if (captureDestinationActive_) {
        captureDestinationButton_->setText(tr("Capture current main-window page"));
    } else {
        captureDestinationButton_->setText(tr("Capture destination from main window…"));
    }
}

std::optional<std::size_t> LinkMapEditorDialog::selectedGlobalIndex() const {
    if (linkList_ == nullptr || linkList_->currentItem() == nullptr) {
        return std::nullopt;
    }
    const int global = linkList_->currentItem()->data(Qt::UserRole).toInt();
    if (global < 0) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(global);
}

pte::core::LinkMapEntry LinkMapEditorDialog::buildEntryFromForm() const {
    pte::core::LinkMapEntry entry;
    if (hasPendingRect_) {
        entry = pendingRect_;
    } else if (facade_ != nullptr) {
        entry.pageIndex = facade_->currentPageIndex();
    }
    if (capturedDestinationPage_ >= 0) {
        entry.destinationPageIndex = capturedDestinationPage_;
    }
    entry.manual = true;
    entry.targetType = "intra";
    entry.destinationId.clear();
    entry.url.clear();
    return entry;
}

} // namespace pte::ui
