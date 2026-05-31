// File: src/core/link_map_store.hpp
// Purpose: Load, edit, and save enrichment link-map.json sidecars (FR-035/036).
// Constraint: Does not read PDF page text; geometry and targets only.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pte::core {

/** @brief One editable link-map entry (PDF user-space rect + target). */
struct LinkMapEntry {
    int pageIndex = 0;
    /** PDF user space: llx, lly, urx, ury (bottom-left origin). */
    double rect[4]{};
    bool manual = true;
    std::string targetType = "intra";
    /** Intra TOC destination PDF page index (zero-based). */
    int destinationPageIndex = -1;
    double destinationYPt = 720.0;
    std::string destinationId;
    std::string url;
};

struct LinkMapDocument {
    std::vector<LinkMapEntry> links;
};

struct LinkMapStoreResult {
    bool success = false;
    std::string safeMessage;
    LinkMapDocument document;
};

LinkMapStoreResult loadLinkMapDocument(const std::filesystem::path& path);
LinkMapStoreResult saveLinkMapDocument(const std::filesystem::path& path,
                                       const LinkMapDocument& document);

/** @brief Resolves `{work}/link-map.json` or `{work}/enrichment/link-map.json`. */
std::optional<std::filesystem::path> resolveLinkMapPath(const std::filesystem::path& workFolder);

} // namespace pte::core
