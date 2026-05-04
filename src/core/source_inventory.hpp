// File: src/core/source_inventory.hpp
// Purpose: Record non-mutating local source PDF identity and duplicate evidence.
// Architectural context: HLA-INV, HLA-SAFE, HLA-DIAG.
// Requirement references: FR-001, FR-002, FR-003, NFR-001, NFR-003, NFR-004. (See also `file_digest`.)
// Test references: TC-FR-001, TC-FR-002, TC-FR-003, TC-NFR-001, TC-NFR-003,
// TC-NFR-004.
// Constraint: Inventory reads local file metadata/bytes only and must not modify source PDFs.

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Options for Poppler pdfinfo subprocess inspection (mirrors PopplerPdfInspectionService).
 */
struct PopplerPdfInspectionOptions {
    /** When empty, resolves `pdfinfo` from PATH. */
    std::filesystem::path pdfinfoExecutable;
};

/**
 * @brief Caller-supplied PDF structural evidence from a future PDF adapter.
 *
 * The inventory service owns file identity. Page count and readability flags are normally
 * supplied by Poppler pdfinfo via SourceInventoryService::inventoryWithPopplerInspection or by
 * tests/synthetic callers.
 */
struct PdfInspectionEvidence {
    std::optional<int> pageCount;
    bool readable = false;
    bool encrypted = false;
    bool malformed = false;
    bool damaged = false;
};

/**
 * @brief Request for source PDF inventory.
 */
struct SourceInventoryRequest {
    std::filesystem::path sourcePath;
    std::filesystem::path corpusRoot;
    PdfInspectionEvidence inspection;
};

/**
 * @brief Safe local source PDF inventory record.
 */
struct SourceInventoryRecord {
    bool success = false;
    std::filesystem::path sourcePath;
    std::string filename;
    std::string corpusRelativePath;
    std::uintmax_t fileSize = 0;
    std::int64_t modifiedToken = 0;
    std::string sha256;
    std::optional<int> pageCount;
    bool readable = false;
    bool encrypted = false;
    bool malformed = false;
    bool damaged = false;
    std::string safeMessage;
};

/**
 * @brief Evidence that two known local source PDFs have the same checksum.
 */
struct DuplicateSourceMatch {
    std::filesystem::path matchingPath;
    std::string sha256;
};

/**
 * @brief Duplicate detection result for one selected source PDF.
 */
struct DuplicateSourceReport {
    std::vector<DuplicateSourceMatch> matches;

    /**
     * @brief Reports whether any duplicate checksum matches were found.
     *
     * @return true when matches is non-empty.
     * Side effects: None.
     * Determinism: Deterministic for this result value.
     */
    bool hasDuplicates() const;
};

/**
 * @brief Builds safe source PDF inventory records and duplicate evidence.
 */
class SourceInventoryService {
public:
    /**
     * @brief Records local source identity from filesystem metadata and adapter evidence.
     *
     * @param request Source path, optional corpus root, and PDF inspection evidence.
     * @return Inventory record containing safe identity metadata and SHA-256.
     * @pre request.sourcePath points to a local file selected by the operator.
     * @post Source file bytes are unchanged.
     * Side effects: Reads file metadata and bytes to compute SHA-256.
     * Determinism: Deterministic for stable file content, path, and metadata.
     */
    SourceInventoryRecord inventory(const SourceInventoryRequest& request) const;

    /**
     * @brief Runs pdfinfo inspection then records inventory using returned structural evidence.
     *
     * @post Source PDF bytes are unchanged.
     */
    SourceInventoryRecord inventoryWithPopplerInspection(
        const std::filesystem::path& sourcePath,
        const std::filesystem::path& corpusRoot,
        const PopplerPdfInspectionOptions& pdfOptions) const;

    /**
     * @brief Finds known source records with the same SHA-256 as the selected record.
     *
     * @param selected Selected source record.
     * @param knownRecords Existing local inventory records from the current context.
     * @return Duplicate matches; no files are deleted or modified.
     * Side effects: None.
     * Determinism: Deterministic for the supplied records.
     */
    DuplicateSourceReport findDuplicates(
        const SourceInventoryRecord& selected,
        const std::vector<SourceInventoryRecord>& knownRecords) const;
};

} // namespace pte::core
