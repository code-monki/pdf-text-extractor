// File: src/core/volume_metadata.hpp
// Purpose: Manage volume metadata, page-map labels, and cover metadata.
// Architectural context: HLA-META, HLA-WORK, HLA-SAFE.
// Requirement references: FR-005, FR-006, FR-007, FR-008, FR-009, FR-010,
// NFR-005, NFR-008, NFR-011, NFR-012.
// Test references: TC-FR-005, TC-FR-006, TC-FR-007, TC-FR-008, TC-FR-009,
// TC-FR-010, TC-NFR-005, TC-NFR-008, TC-NFR-011, TC-NFR-012.
// Constraint: Metadata edits must not modify source PDFs or reviewed page text.

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief One page-map entry in volume metadata.
 */
struct PageMapEntry {
    std::string pageId;
    int pdfPage = 0;
    std::string pageTextPath;
    std::string printedPageLabel;
    bool includeByDefault = true;
};

/**
 * @brief Optional cover metadata.
 */
struct CoverMetadata {
    std::optional<std::string> pageId;
    std::optional<int> pdfPage;
};

/**
 * @brief Editable volume metadata model.
 */
struct VolumeMetadata {
    std::string volumeId;
    std::string title;
    std::string subtitle;
    std::string sortTitle;
    std::string group;
    std::string sourcePath;
    std::string sourceFilename;
    int sourcePageCount = 0;
    std::map<std::string, std::string> bibliographic;
    bool includeByDefault = true;
    CoverMetadata cover;
    std::vector<PageMapEntry> pageMap;
    std::string notes;
};

/**
 * @brief Result for metadata write/edit operations.
 */
struct VolumeMetadataResult {
    bool success = false;
    VolumeMetadata metadata;
    std::string safeMessage;
};

/**
 * @brief Service for deterministic volume metadata serialization and edit helpers.
 */
class VolumeMetadataService {
public:
    /**
     * @brief Writes a complete `volume.json` artifact.
     *
     * @param workFolder Root work-folder path.
     * @param metadata Complete metadata model.
     * @return Success or safe validation failure.
     * @post Reviewed page text and source PDFs are not modified.
     * Side effects: Writes `volume.json` atomically below workFolder.
     * Determinism: Deterministic for stable metadata and filesystem state.
     */
    VolumeMetadataResult writeVolumeMetadata(const std::filesystem::path& workFolder,
                                             const VolumeMetadata& metadata) const;

    /**
     * @brief Reads `volume.json` into an in-memory model for editing (FR-006).
     *
     * @post Source PDFs and reviewed page text files are not modified.
     */
    VolumeMetadataResult loadVolumeMetadata(const std::filesystem::path& workFolder) const;

    /**
     * @brief Sets a printed page label on an in-memory metadata model.
     *
     * @param metadata Metadata model to update.
     * @param pageId Page ID to update.
     * @param label User-entered printed page label; empty labels are valid.
     * @return Updated metadata or safe failure.
     * Side effects: None.
     * Determinism: Deterministic for supplied metadata and page ID.
     */
    VolumeMetadataResult setPrintedPageLabel(const VolumeMetadata& metadata,
                                             const std::string& pageId,
                                             const std::string& label) const;

    /**
     * @brief Sets or clears cover page metadata on an in-memory metadata model.
     *
     * @param metadata Metadata model to update.
     * @param pageId Optional page ID. Empty optional clears cover metadata.
     * @return Updated metadata or safe failure.
     * Side effects: None.
     * Determinism: Deterministic for supplied metadata and page ID.
     */
    VolumeMetadataResult setCoverPage(const VolumeMetadata& metadata,
                                      const std::optional<std::string>& pageId) const;
};

} // namespace pte::core
