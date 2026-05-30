#pragma once

/**
 * @file volume_metadata_dialog.hpp
 * @brief Declares the modal `volume.json` editor used from ShellMainWindow.
 */

#include <QDialog>

#include <filesystem>

namespace pte::ui {

/**
 * @brief Modal form for editing work-folder `volume.json` (FR-006, NFR-009).
 *
 * @details Manages title, notes, bibliographic fields, page label map, and cover page id. Does not
 *          modify the source PDF or pages/NNNN.txt reviewed text files; saving rewrites volume.json atomically
 *          through core helpers. Caller should invoke ReviewSessionFacade::refreshAfterVolumeMetadataSave
 *          after accept.
 */
class VolumeMetadataDialog final : public QDialog {
    Q_OBJECT

public:
    explicit VolumeMetadataDialog(QWidget* parent, const std::filesystem::path& workFolder);

private:
    std::filesystem::path workFolder_;
};

} // namespace pte::ui
