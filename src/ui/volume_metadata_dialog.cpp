/**
 * @file volume_metadata_dialog.cpp
 * @brief Implements @ref pte::ui::VolumeMetadataDialog tabs and load/save of `volume.json`.
 */

#include "ui/volume_metadata_dialog.hpp"

#include "core/volume_metadata.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QString>
#include <QVBoxLayout>

namespace pte::ui {

namespace {

QString biblioValue(const pte::core::VolumeMetadata& metadata, const char* key) {
    const auto iter = metadata.bibliographic.find(key);
    if (iter == metadata.bibliographic.end()) {
        return {};
    }
    return QString::fromStdString(iter->second);
}

void setBiblio(pte::core::VolumeMetadata& metadata, const char* key, const QString& text) {
    const auto trimmed = text.trimmed().toStdString();
    if (trimmed.empty()) {
        metadata.bibliographic.erase(key);
        return;
    }
    metadata.bibliographic[key] = trimmed;
}

} // namespace

VolumeMetadataDialog::VolumeMetadataDialog(QWidget* parent,
                                           const std::filesystem::path& workFolder)
    : QDialog(parent)
    , workFolder_(workFolder) {
    setWindowTitle(tr("Volume metadata"));
    resize(640, 520);

    pte::core::VolumeMetadataService service;
    const pte::core::VolumeMetadataResult loaded = service.loadVolumeMetadata(workFolder_);
    if (!loaded.success) {
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel(tr("Could not load volume.json."), this));
        layout->addWidget(
            new QLabel(QString::fromStdString(loaded.safeMessage), this));
        auto* closeBtn = new QPushButton(tr("Close"), this);
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
        layout->addWidget(closeBtn);
        return;
    }

    pte::core::VolumeMetadata metadata = loaded.metadata;

    auto* titleEdit = new QLineEdit(QString::fromStdString(metadata.title));
    auto* subtitleEdit = new QLineEdit(QString::fromStdString(metadata.subtitle));
    auto* sortTitleEdit = new QLineEdit(QString::fromStdString(metadata.sortTitle));
    auto* groupEdit = new QLineEdit(QString::fromStdString(metadata.group));
    auto* notesEdit = new QPlainTextEdit();
    notesEdit->setPlainText(QString::fromStdString(metadata.notes));

    auto* languageEdit = new QLineEdit(biblioValue(metadata, "language"));
    auto* productCodeEdit = new QLineEdit(biblioValue(metadata, "productCode"));
    auto* publisherEdit = new QLineEdit(biblioValue(metadata, "publisher"));

    auto* volumeTab = new QWidget(this);
    auto* form = new QFormLayout(volumeTab);
    form->addRow(tr("Volume ID"), new QLabel(QString::fromStdString(metadata.volumeId)));
    form->addRow(tr("Source PDF"), new QLabel(QString::fromStdString(metadata.sourceFilename)));
    form->addRow(tr("PDF pages"), new QLabel(QString::number(metadata.sourcePageCount)));
    form->addRow(tr("Title"), titleEdit);
    form->addRow(tr("Subtitle"), subtitleEdit);
    form->addRow(tr("Sort title"), sortTitleEdit);
    form->addRow(tr("Group"), groupEdit);
    form->addRow(tr("Notes"), notesEdit);
    form->addRow(tr("Language"), languageEdit);
    form->addRow(tr("Product code"), productCodeEdit);
    form->addRow(tr("Publisher"), publisherEdit);

    auto* pagesTab = new QWidget(this);
    auto* pagesLayout = new QVBoxLayout(pagesTab);
    auto* table = new QTableWidget(static_cast<int>(metadata.pageMap.size()), 3);
    table->setHorizontalHeaderLabels({tr("Page ID"), tr("PDF page"), tr("Printed label")});
    table->horizontalHeader()->setStretchLastSection(true);
    for (size_t i = 0; i < metadata.pageMap.size(); ++i) {
        const auto& page = metadata.pageMap[i];
        const int row = static_cast<int>(i);
        auto* idItem = new QTableWidgetItem(QString::fromStdString(page.pageId));
        idItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        table->setItem(row, 0, idItem);
        auto* pdfItem = new QTableWidgetItem(QString::number(page.pdfPage));
        pdfItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        table->setItem(row, 1, pdfItem);
        auto* labelEdit = new QLineEdit(QString::fromStdString(page.printedPageLabel));
        table->setCellWidget(row, 2, labelEdit);
    }
    pagesLayout->addWidget(table);

    auto* coverEdit = new QLineEdit();
    if (metadata.cover.pageId.has_value()) {
        coverEdit->setText(QString::fromStdString(*metadata.cover.pageId));
    }
    auto* coverForm = new QFormLayout();
    coverForm->addRow(tr("Cover page ID"), coverEdit);
    pagesLayout->addLayout(coverForm);
    pagesLayout->addWidget(new QLabel(
        tr("Leave cover page ID blank to clear cover metadata. Printed labels are indexing hints "
           "only (FR-008, FR-010)."),
        pagesTab));
    pagesLayout->addStretch(1);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(volumeTab, tr("Volume"));
    tabs->addTab(pagesTab, tr("Pages and cover"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->addWidget(tabs);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    rootLayout->addWidget(box);

    connect(box, &QDialogButtonBox::accepted, this, [this,
                                                       metadata,
                                                       titleEdit,
                                                       subtitleEdit,
                                                       sortTitleEdit,
                                                       groupEdit,
                                                       notesEdit,
                                                       languageEdit,
                                                       productCodeEdit,
                                                       publisherEdit,
                                                       table,
                                                       coverEdit]() mutable {
        metadata.title = titleEdit->text().toStdString();
        metadata.subtitle = subtitleEdit->text().toStdString();
        metadata.sortTitle = sortTitleEdit->text().toStdString();
        metadata.group = groupEdit->text().toStdString();
        metadata.notes = notesEdit->toPlainText().toStdString();
        setBiblio(metadata, "language", languageEdit->text());
        setBiblio(metadata, "productCode", productCodeEdit->text());
        setBiblio(metadata, "publisher", publisherEdit->text());

        pte::core::VolumeMetadataService editor;
        const int rows = table->rowCount();
        for (int row = 0; row < rows; ++row) {
            if (row < 0 || static_cast<size_t>(row) >= metadata.pageMap.size()) {
                continue;
            }
            const std::string pageId = metadata.pageMap[static_cast<size_t>(row)].pageId;
            auto* widget = table->cellWidget(row, 2);
            auto* line = qobject_cast<QLineEdit*>(widget);
            const std::string label = line != nullptr ? line->text().toStdString() : "";
            const auto labeled = editor.setPrintedPageLabel(metadata, pageId, label);
            if (!labeled.success) {
                QMessageBox::warning(this, tr("Volume metadata"),
                                     QString::fromStdString(labeled.safeMessage));
                return;
            }
            metadata = labeled.metadata;
        }

        const QString trimmedCover = coverEdit->text().trimmed();
        const auto covered =
            trimmedCover.isEmpty()
                ? editor.setCoverPage(metadata, std::nullopt)
                : editor.setCoverPage(metadata, trimmedCover.toStdString());
        if (!covered.success) {
            QMessageBox::warning(this, tr("Volume metadata"),
                                 QString::fromStdString(covered.safeMessage));
            return;
        }
        metadata = covered.metadata;

        pte::core::VolumeMetadataService writer;
        const pte::core::VolumeMetadataResult saved =
            writer.writeVolumeMetadata(workFolder_, metadata);
        if (!saved.success) {
            QMessageBox::warning(this, tr("Volume metadata"),
                                 QString::fromStdString(saved.safeMessage));
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

} // namespace pte::ui
