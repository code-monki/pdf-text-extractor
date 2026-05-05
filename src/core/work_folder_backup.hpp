// File: src/core/work_folder_backup.hpp
// Purpose: Create and restore local work-folder backups before protected bulk changes.
// Architectural context: HLA-WORK, HLA-APP, HLA-SAFE.
// Requirement references: FR-023, FR-026, NFR-007, NFR-012.
// Test references: TC-FR-023, TC-FR-026, TC-NFR-007, TC-NFR-012.
// Constraint: Backups copy local work-folder artifacts only and must not copy source PDFs.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Request for local work-folder backup creation.
 */
struct WorkFolderBackupRequest {
    std::filesystem::path workFolder;
    std::string backupId;
    bool dryRun = false;
};

/**
 * @brief Request for restoring reviewed text and metadata from a backup.
 */
struct WorkFolderRestoreRequest {
    std::filesystem::path workFolder;
    std::filesystem::path backupFolder;
    bool confirmed = false;
    bool dryRun = false;
};

/**
 * @brief Safe backup or restore report.
 */
struct WorkFolderBackupReport {
    bool success = false;
    bool dryRun = false;
    std::filesystem::path backupFolder;
    std::vector<std::string> plannedArtifacts;
    std::string safeMessage;
};

/**
 * @brief Service for protected local work-folder backup and restore operations.
 */
class WorkFolderBackupService {
public:
    /**
     * @brief Creates or plans a backup of protected reviewed text and metadata artifacts.
     *
     * @param request Work-folder root, backup id, and dry-run flag.
     * @return Backup report with planned/copied artifact names.
     * @post Source PDFs and raw candidates are not copied by this protection backup slice.
     * Side effects: In non-dry-run mode, writes files below `backups/<backupId>/`.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    WorkFolderBackupReport createBackup(const WorkFolderBackupRequest& request) const;

    /**
     * @brief Restores protected reviewed text and metadata from a prior backup.
     *
     * @param request Work-folder root, backup folder, confirmation, and dry-run flag.
     * @return Restore report with planned/restored artifact names.
     * @pre request.confirmed must be true unless request.dryRun is true.
     * @post Source PDFs are not modified.
     * Side effects: In non-dry-run mode, replaces protected work-folder artifacts.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    WorkFolderBackupReport restoreBackup(const WorkFolderRestoreRequest& request) const;
};

} // namespace pte::core
