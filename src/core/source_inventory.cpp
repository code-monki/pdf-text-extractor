// File: src/core/source_inventory.cpp
// Purpose: Implement safe local source PDF identity and duplicate detection.
// Architectural context: HLA-INV, HLA-SAFE, HLA-DIAG.
// Requirement references: FR-001, FR-002, FR-003, NFR-001, NFR-003, NFR-004.
// Test references: TC-FR-001, TC-FR-002, TC-FR-003, TC-NFR-001, TC-NFR-003,
// TC-NFR-004.
// Constraint: Inventory never writes source PDFs or reports PDF contents.

#include "core/source_inventory.hpp"

#include "core/file_digest.hpp"
#include "core/pdf_inspection_poppler.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>

namespace pte::core {

namespace {

/**
 * @brief Converts filesystem write time to a stable integer token.
 */
std::int64_t modifiedToken(const std::filesystem::path& path) {
    std::error_code error;
    const auto time = std::filesystem::last_write_time(path, error);
    if (error) {
        return 0;
    }
    return time.time_since_epoch().count();
}

/**
 * @brief Builds a corpus-relative path string when the source is inside the corpus root.
 */
std::string relativePathText(const std::filesystem::path& source,
                             const std::filesystem::path& corpusRoot) {
    if (corpusRoot.empty()) {
        return source.filename().string();
    }

    std::error_code error;
    const auto relative = std::filesystem::relative(source, corpusRoot, error);
    if (error || relative.empty() || relative.native().find("..") == 0) {
        return source.filename().string();
    }
    return relative.generic_string();
}

} // namespace

/**
 * @brief Reports whether duplicate checksum matches were found.
 */
bool DuplicateSourceReport::hasDuplicates() const {
    return !matches.empty();
}

/**
 * @brief Records safe source PDF identity and adapter-supplied PDF evidence.
 */
SourceInventoryRecord SourceInventoryService::inventory(
    const SourceInventoryRequest& request) const {
    SourceInventoryRecord record;
    record.sourcePath = request.sourcePath;
    record.filename = request.sourcePath.filename().string();

    std::error_code error;
    if (request.sourcePath.empty()) {
        record.safeMessage = "source PDF path is required";
        return record;
    }
    if (!std::filesystem::is_regular_file(request.sourcePath, error) || error) {
        record.safeMessage = "source PDF is unavailable";
        return record;
    }

    record.fileSize = std::filesystem::file_size(request.sourcePath, error);
    if (error) {
        record.fileSize = 0;
    }
    record.modifiedToken = modifiedToken(request.sourcePath);
    const auto digest = sha256HexFile(request.sourcePath);
    if (!digest) {
        record.safeMessage = "source PDF could not be read";
        return record;
    }

    record.sha256 = *digest;
    record.corpusRelativePath = relativePathText(request.sourcePath, request.corpusRoot);
    record.pageCount = request.inspection.pageCount;
    record.readable = request.inspection.readable;
    record.encrypted = request.inspection.encrypted;
    record.malformed = request.inspection.malformed;
    record.damaged = request.inspection.damaged;
    record.success = true;
    record.safeMessage = "source PDF inventory recorded";
    return record;
}

/**
 * @brief Records inventory after Poppler pdfinfo inspection.
 */
SourceInventoryRecord SourceInventoryService::inventoryWithPopplerInspection(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& corpusRoot,
    const PopplerPdfInspectionOptions& pdfOptions) const {
    PopplerPdfInspectionService inspector;
    PopplerPdfInspectionService::Options inspectOpts;
    inspectOpts.pdfinfoExecutable = pdfOptions.pdfinfoExecutable;
    const auto inspected = inspector.inspect(sourcePath, inspectOpts);
    SourceInventoryRequest request;
    request.sourcePath = sourcePath;
    request.corpusRoot = corpusRoot;
    request.inspection = inspected.evidence;
    return inventory(request);
}

/**
 * @brief Finds known records with the same checksum without deleting anything.
 */
DuplicateSourceReport SourceInventoryService::findDuplicates(
    const SourceInventoryRecord& selected,
    const std::vector<SourceInventoryRecord>& knownRecords) const {
    DuplicateSourceReport report;
    if (!selected.success || selected.sha256.empty()) {
        return report;
    }

    for (const auto& known : knownRecords) {
        if (!known.success || known.sha256 != selected.sha256) {
            continue;
        }
        if (known.sourcePath == selected.sourcePath) {
            continue;
        }
        report.matches.push_back(DuplicateSourceMatch{known.sourcePath, known.sha256});
    }
    return report;
}

} // namespace pte::core
