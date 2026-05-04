// File: src/core/output_config.cpp
// Purpose: Implement local output path configuration validation.
// Architectural context: HLA-CONFIG, HLA-WORK, HLA-SAFE.
// Requirement references: FR-029, FR-030, NFR-001, NFR-002, NFR-008, NFR-011.
// Test references: TC-FR-029, TC-FR-030, TC-NFR-001, TC-NFR-002, TC-NFR-008,
// TC-NFR-011.
// Constraint: Validation must not require remote APIs, databases, or corpus-specific paths.

#include "core/output_config.hpp"

#include "core/local_path_intent.hpp"

#include <filesystem>

namespace pte::core {

namespace {

/**
 * @brief Adds one safe configuration finding.
 */
void addFinding(std::vector<OutputPathFinding>& findings,
                std::string code,
                std::string safeMessage) {
    findings.push_back(OutputPathFinding{std::move(code), std::move(safeMessage)});
}

} // namespace

/**
 * @brief Converts retention policy enum to a stable lowercase name.
 */
std::string artifactRetentionPolicyName(ArtifactRetentionPolicy policy) {
    switch (policy) {
    case ArtifactRetentionPolicy::RetainRawCandidates: return "retain-raw-candidates";
    case ArtifactRetentionPolicy::AllowUserCleanup: return "allow-user-cleanup";
    }
    return "unknown";
}

/**
 * @brief Validates output roots as normal local filesystem paths.
 */
std::vector<OutputPathFinding> OutputConfigService::validateOutputRoot(
    const std::filesystem::path& outputRoot) const {
    std::vector<OutputPathFinding> findings;

    if (outputRoot.empty()) {
        addFinding(findings, "missing-output-root", "output root path is required");
        return findings;
    }
    if (pathTextLooksLikeHostedUriScheme(outputRoot.generic_string())) {
        addFinding(findings,
                   "non-filesystem-output-root",
                   "output root must be an operating-system filesystem path");
        return findings;
    }

    std::error_code error;
    if (std::filesystem::exists(outputRoot, error)) {
        if (error || !std::filesystem::is_directory(outputRoot, error) || error) {
            addFinding(findings, "invalid-output-root", "output root is not a directory");
        }
        return findings;
    }

    const auto parent = outputRoot.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent, error)) {
        addFinding(findings, "missing-output-parent", "output root parent is unavailable");
    }
    return findings;
}

/**
 * @brief Applies a valid output root without creating extraction artifacts.
 */
OutputConfigResult OutputConfigService::setOutputRoot(
    const AppConfiguration& current,
    const std::filesystem::path& outputRoot) const {
    OutputConfigResult result;
    result.configuration = current;
    result.findings = validateOutputRoot(outputRoot);
    if (!result.findings.empty()) {
        result.safeMessage = "output root rejected";
        return result;
    }

    result.configuration.outputRoot = outputRoot;
    result.success = true;
    result.safeMessage = "output root configured";
    return result;
}

} // namespace pte::core
