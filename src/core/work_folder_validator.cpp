// File: src/core/work_folder_validator.cpp
// Purpose: Implement safe structural validation for local extractor work folders.
// Architectural context: HLA-VALID with HLA-WORK, HLA-META, HLA-REVIEW, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-011, FR-017, FR-018, FR-025, NFR-011, NFR-012.
// Test references: TC-FR-011, TC-FR-017, TC-FR-018, TC-FR-025, TC-NFR-011, TC-NFR-012.
// Assumption: Validation operates on local filesystem paths and does not inspect substantive
// page text.
// Constraint: Findings include artifact names, page IDs, status values, and structural codes only.

#include "core/work_folder_validator.hpp"

#include "core/json.hpp"
#include "core/page_id.hpp"
#include "core/review_state.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace pte::core {

namespace {

/**
 * @brief Appends one safe structural finding to a validation report.
 *
 * @param report Report receiving the finding.
 * @param severity Blocking or non-blocking severity.
 * @param code Stable machine-readable finding code.
 * @param artifact Artifact name or safe path associated with the finding.
 * @param pageId Optional page ID associated with the finding.
 * @param safeMessage Human-readable message that must omit source/page text.
 * @param repairable Whether later repair tooling may be able to correct the finding.
 */
void addFinding(ValidationReport& report,
                Severity severity,
                std::string code,
                std::string artifact,
                std::string pageId,
                std::string safeMessage,
                bool repairable = false) {
    report.findings.push_back(ValidationFinding{
        severity,
        std::move(code),
        std::move(artifact),
        std::move(pageId),
        std::move(safeMessage),
        repairable
    });
}

/**
 * @brief Reads a local artifact file as binary text.
 *
 * @param path Local filesystem path to read.
 * @return File contents, or std::nullopt when the file cannot be opened.
 * Side effects: Reads local filesystem content only.
 */
std::optional<std::string> readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

/**
 * @brief Reads and parses a required JSON artifact.
 *
 * @param report Report receiving missing or invalid JSON findings.
 * @param path Artifact path.
 * @param artifactName Stable artifact name for safe diagnostics.
 * @return Parsed JSON object, or std::nullopt when missing or structurally invalid.
 * @post Findings contain artifact names and byte offsets only, not JSON excerpts.
 */
std::optional<JsonValue> parseArtifact(ValidationReport& report,
                                       const std::filesystem::path& path,
                                       const std::string& artifactName) {
    const auto content = readFile(path);
    if (!content) {
        addFinding(report, Severity::Blocker, "missing-artifact", artifactName, "",
                   artifactName + " is missing", true);
        return std::nullopt;
    }

    const auto parsed = parseJson(*content);
    if (!parsed.value) {
        addFinding(report, Severity::Blocker, "invalid-json", artifactName, "",
                   artifactName + " is not valid JSON at byte offset "
                       + std::to_string(parsed.offset),
                   false);
        return std::nullopt;
    }
    if (!parsed.value->isObject()) {
        addFinding(report, Severity::Blocker, "invalid-json-root", artifactName, "",
                   artifactName + " root must be an object", false);
        return std::nullopt;
    }
    return parsed.value;
}

/**
 * @brief Extracts an integer-valued numeric field from an object.
 *
 * @param object JSON object value to inspect.
 * @param key Field key.
 * @return Integer value, or std::nullopt when absent, non-numeric, or fractional.
 */
std::optional<int> integerField(const JsonValue& object, std::string_view key) {
    const auto* value = object.find(key);
    if (!value) {
        return std::nullopt;
    }
    const auto* number = value->asNumber();
    if (!number) {
        return std::nullopt;
    }
    const int integer = static_cast<int>(*number);
    if (*number != static_cast<double>(integer)) {
        return std::nullopt;
    }
    return integer;
}

/**
 * @brief Requires an object-valued child field and records safe findings when invalid.
 *
 * @param report Report receiving missing/invalid object findings.
 * @param parent Parent JSON value.
 * @param key Required child key.
 * @param artifactName Artifact name used in safe diagnostics.
 * @return Child object pointer, or nullptr when absent or wrong type.
 */
const JsonValue::Object* requiredObject(ValidationReport& report,
                                        const JsonValue& parent,
                                        std::string_view key,
                                        const std::string& artifactName) {
    const auto* value = parent.find(key);
    if (!value) {
        addFinding(report, Severity::Blocker, "missing-object", artifactName, "",
                   artifactName + " missing required object '" + std::string(key) + "'", false);
        return nullptr;
    }
    const auto* object = value->asObject();
    if (!object) {
        addFinding(report, Severity::Blocker, "invalid-object", artifactName, "",
                   artifactName + " field '" + std::string(key) + "' must be an object", false);
        return nullptr;
    }
    return object;
}

/**
 * @brief Extracts a string field from an object-like JSON value.
 *
 * @param object JSON value expected to contain object members.
 * @param key Field key.
 * @return String value, or an empty string when absent or wrong type.
 */
std::string stringField(const JsonValue& object, std::string_view key) {
    const auto* value = object.find(key);
    if (!value) {
        return {};
    }
    const auto* text = value->asString();
    if (!text) {
        return {};
    }
    return *text;
}

/**
 * @brief Validates an artifact's schemaVersion field against the current implementation slice.
 *
 * @param report Report receiving schema findings.
 * @param artifact Parsed artifact root object.
 * @param artifactName Artifact name used in safe diagnostics.
 */
void validateSchemaVersion(ValidationReport& report,
                           const JsonValue& artifact,
                           const std::string& artifactName) {
    const auto version = integerField(artifact, "schemaVersion");
    if (!version) {
        addFinding(report, Severity::Blocker, "missing-schema-version", artifactName, "",
                   artifactName + " missing numeric schemaVersion", false);
        return;
    }
    if (*version != 1) {
        addFinding(report, Severity::Blocker, "unsupported-schema-version", artifactName, "",
                   artifactName + " schemaVersion is unsupported", false);
    }
}

} // namespace

/**
 * @brief Reports whether the validation run has no Error or Blocker findings.
 */
bool ValidationReport::ok() const {
    return errorCount() == 0;
}

/**
 * @brief Counts Error and Blocker findings that prevent a clean validation result.
 */
int ValidationReport::errorCount() const {
    int count = 0;
    for (const auto& finding : findings) {
        if (finding.severity == Severity::Error || finding.severity == Severity::Blocker) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Converts a severity enum to its stable lowercase diagnostic label.
 */
std::string severityName(Severity severity) {
    switch (severity) {
    case Severity::Info: return "info";
    case Severity::Warning: return "warning";
    case Severity::Error: return "error";
    case Severity::Blocker: return "blocker";
    }
    return "unknown";
}

/**
 * @brief Validates required work-folder artifacts and page coverage.
 *
 * The implementation validates JSON shape, schema version, volume identity consistency, page map
 * coverage, review entry validity, and reviewed page-file presence. It avoids opening page text
 * files so findings cannot reproduce restricted PDF-derived content.
 */
ValidationReport WorkFolderValidator::validate(const std::filesystem::path& workFolder) const {
    ValidationReport report;

    const auto volume = parseArtifact(report, workFolder / "volume.json", "volume.json");
    const auto quality = parseArtifact(report,
                                       workFolder / "page-quality.json",
                                       "page-quality.json");
    const auto review = parseArtifact(report,
                                      workFolder / "review-state.json",
                                      "review-state.json");

    if (!volume || !quality || !review) {
        return report;
    }

    validateSchemaVersion(report, *volume, "volume.json");
    validateSchemaVersion(report, *quality, "page-quality.json");
    validateSchemaVersion(report, *review, "review-state.json");

    const auto volumeId = stringField(*volume, "volumeId");
    const auto qualityVolumeId = stringField(*quality, "volumeId");
    const auto reviewVolumeId = stringField(*review, "volumeId");
    if (volumeId.empty()) {
        addFinding(report, Severity::Blocker, "missing-volume-id", "volume.json", "",
                   "volume.json missing volumeId", false);
    }
    if (!volumeId.empty() && !qualityVolumeId.empty() && qualityVolumeId != volumeId) {
        addFinding(report, Severity::Blocker, "volume-id-mismatch", "page-quality.json", "",
                   "page-quality.json volumeId does not match volume.json", false);
    }
    if (!volumeId.empty() && !reviewVolumeId.empty() && reviewVolumeId != volumeId) {
        addFinding(report, Severity::Blocker, "volume-id-mismatch", "review-state.json", "",
                   "review-state.json volumeId does not match volume.json", false);
    }

    const auto* sourcePdfValue = volume->find("sourcePdf");
    const auto* sourcePdf = sourcePdfValue ? sourcePdfValue->asObject() : nullptr;
    if (!sourcePdf) {
        addFinding(report, Severity::Blocker, "missing-source-pdf", "volume.json", "",
                   "volume.json missing sourcePdf object", false);
        return report;
    }

    const auto pageCountValue = sourcePdfValue
        ? integerField(*sourcePdfValue, "pageCount")
        : std::nullopt;
    if (!pageCountValue || *pageCountValue < 0) {
        addFinding(report, Severity::Blocker, "invalid-page-count", "volume.json", "",
                   "sourcePdf.pageCount must be a non-negative integer", false);
        return report;
    }
    const int pageCount = *pageCountValue;

    const auto* pageMap = requiredObject(report, *volume, "pageMap", "volume.json");
    const auto* qualityPages = requiredObject(report, *quality, "pages", "page-quality.json");
    const auto* reviewPages = requiredObject(report, *review, "pages", "review-state.json");
    if (!pageMap || !qualityPages || !reviewPages) {
        return report;
    }

    if (!std::filesystem::is_directory(workFolder / "pages")) {
        addFinding(report, Severity::Blocker, "missing-pages-directory", "pages", "",
                   "pages directory is missing", true);
    }

    for (int pdfPage = 1; pdfPage <= pageCount; ++pdfPage) {
        const std::string pageId = pageIdForPdfPage(pdfPage);

        if (pageMap->find(pageId) == pageMap->end()) {
            addFinding(report, Severity::Error, "missing-page-map-entry", "volume.json", pageId,
                       "pageMap entry is missing", true);
        }
        if (qualityPages->find(pageId) == qualityPages->end()) {
            addFinding(report, Severity::Error, "missing-quality-entry", "page-quality.json",
                       pageId,
                       "quality entry is missing", true);
        }

        const auto reviewEntry = reviewPages->find(pageId);
        if (reviewEntry == reviewPages->end()) {
            addFinding(report, Severity::Error, "missing-review-entry", "review-state.json", pageId,
                       "review entry is missing", true);
        } else if (const auto* reviewObject = reviewEntry->second.asObject()) {
            const auto status = stringField(reviewEntry->second, "status");
            if (!isAllowedReviewStatus(status)) {
                addFinding(report, Severity::Error, "invalid-review-status", "review-state.json",
                           pageId,
                           "review status is not allowed", false);
            }
            (void)reviewObject;
        } else {
            addFinding(report, Severity::Error, "invalid-review-entry", "review-state.json", pageId,
                       "review entry must be an object", false);
        }

        const auto pageFile = workFolder / "pages" / (pageId + ".txt");
        if (!std::filesystem::exists(pageFile)) {
            addFinding(report, Severity::Error, "missing-page-file",
                       pageFile.lexically_normal().string(), pageId,
                       "reviewed page text file is missing", true);
        }
    }

    for (const auto& [pageId, value] : *pageMap) {
        (void)value;
        if (!isValidPageId(pageId)) {
            addFinding(report, Severity::Error, "invalid-page-id", "volume.json", pageId,
                       "pageMap contains invalid page ID", false);
        }
    }

    return report;
}

} // namespace pte::core
