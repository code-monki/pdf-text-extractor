// File: src/core/work_folder_backup.cpp
// Purpose: Implement local protected work-folder backup and restore operations.
// Architectural context: HLA-WORK, HLA-APP, HLA-SAFE.
// Requirement references: FR-023, FR-026, NFR-007, NFR-012.
// Test references: TC-FR-023, TC-FR-026, TC-NFR-007, TC-NFR-012.
// Constraint: Backup/restore reports artifact names only and omit page text contents.

#include "core/work_folder_backup.hpp"

#include <filesystem>

namespace pte::core {

namespace {

/**
 * @brief Protected artifact roots covered by backup/restore in this implementation slice.
 */
std::vector<std::filesystem::path> protectedArtifacts() {
    return {
        "volume.json",
        "review-state.json",
        "page-quality.json",
        "pages"
    };
}

/**
 * @brief Copies a file or directory tree recursively.
 */
bool copyArtifact(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::error_code error;
    if (!std::filesystem::exists(from, error) || error) {
        return true;
    }
    if (std::filesystem::is_directory(from, error) && !error) {
        std::filesystem::create_directories(to, error);
        if (error) {
            return false;
        }
        for (const auto& entry : std::filesystem::recursive_directory_iterator(from, error)) {
            if (error) {
                return false;
            }
            const auto relative = std::filesystem::relative(entry.path(), from, error);
            if (error) {
                return false;
            }
            const auto destination = to / relative;
            if (entry.is_directory(error)) {
                std::filesystem::create_directories(destination, error);
            } else if (entry.is_regular_file(error)) {
                std::filesystem::create_directories(destination.parent_path(), error);
                std::filesystem::copy_file(entry.path(),
                                           destination,
                                           std::filesystem::copy_options::overwrite_existing,
                                           error);
            }
            if (error) {
                return false;
            }
        }
        return true;
    }

    std::filesystem::create_directories(to.parent_path(), error);
    if (error) {
        return false;
    }
    std::filesystem::copy_file(from,
                               to,
                               std::filesystem::copy_options::overwrite_existing,
                               error);
    return !error;
}

/**
 * @brief Removes a prior destination before restoring a directory or file.
 */
bool replaceArtifact(const std::filesystem::path& from, const std::filesystem::path& to) {
    std::error_code error;
    std::filesystem::remove_all(to, error);
    if (error) {
        return false;
    }
    return copyArtifact(from, to);
}

} // namespace

/**
 * @brief Creates or plans protected artifact backup.
 */
WorkFolderBackupReport WorkFolderBackupService::createBackup(
    const WorkFolderBackupRequest& request) const {
    WorkFolderBackupReport report;
    report.dryRun = request.dryRun;
    if (request.workFolder.empty()) {
        report.safeMessage = "work folder path is required";
        return report;
    }
    if (request.backupId.empty() || request.backupId.find("..") != std::string::npos) {
        report.safeMessage = "backup id is invalid";
        return report;
    }

    report.backupFolder = request.workFolder / "backups" / request.backupId;
    for (const auto& artifact : protectedArtifacts()) {
        report.plannedArtifacts.push_back(artifact.generic_string());
    }

    if (request.dryRun) {
        report.success = true;
        report.safeMessage = "backup planned";
        return report;
    }

    for (const auto& artifact : protectedArtifacts()) {
        if (!copyArtifact(request.workFolder / artifact, report.backupFolder / artifact)) {
            report.safeMessage = "backup failed while copying protected artifact";
            return report;
        }
    }

    report.success = true;
    report.safeMessage = "backup created";
    return report;
}

/**
 * @brief Restores or plans restore of protected artifacts from backup.
 */
WorkFolderBackupReport WorkFolderBackupService::restoreBackup(
    const WorkFolderRestoreRequest& request) const {
    WorkFolderBackupReport report;
    report.dryRun = request.dryRun;
    report.backupFolder = request.backupFolder;
    if (!request.dryRun && !request.confirmed) {
        report.safeMessage = "restore requires explicit confirmation";
        return report;
    }
    if (request.workFolder.empty() || request.backupFolder.empty()) {
        report.safeMessage = "work folder and backup folder are required";
        return report;
    }

    for (const auto& artifact : protectedArtifacts()) {
        report.plannedArtifacts.push_back(artifact.generic_string());
    }

    if (request.dryRun) {
        report.success = true;
        report.safeMessage = "restore planned";
        return report;
    }

    for (const auto& artifact : protectedArtifacts()) {
        if (!replaceArtifact(request.backupFolder / artifact, request.workFolder / artifact)) {
            report.safeMessage = "restore failed while replacing protected artifact";
            return report;
        }
    }

    report.success = true;
    report.safeMessage = "backup restored";
    return report;
}

} // namespace pte::core
