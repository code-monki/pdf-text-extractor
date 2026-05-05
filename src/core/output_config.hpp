// File: src/core/output_config.hpp
// Purpose: Validate local output/source path configuration and artifact retention policy.
// Architectural context: HLA-CONFIG, HLA-WORK, HLA-SAFE.
// Requirement references: FR-029, FR-030, NFR-001, NFR-002, NFR-008, NFR-011.
// Test references: TC-FR-029, TC-FR-030, TC-NFR-001, TC-NFR-002, TC-NFR-008,
// TC-NFR-011.
// Constraint: Configuration uses OS filesystem paths and does not require network services.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Local artifact retention policy for generated diagnostic artifacts.
 */
enum class ArtifactRetentionPolicy {
    RetainRawCandidates,
    AllowUserCleanup
};

/**
 * @brief Local application configuration relevant to extraction output placement.
 */
struct AppConfiguration {
    std::filesystem::path sourceRoot;
    std::filesystem::path outputRoot;
    ArtifactRetentionPolicy retentionPolicy = ArtifactRetentionPolicy::RetainRawCandidates;
};

/**
 * @brief Safe path validation finding for configuration changes.
 */
struct OutputPathFinding {
    std::string code;
    std::string safeMessage;
};

/**
 * @brief Result of applying an output-root configuration change.
 */
struct OutputConfigResult {
    bool success = false;
    AppConfiguration configuration;
    std::vector<OutputPathFinding> findings;
    std::string safeMessage;
};

/**
 * @brief Validates and applies local path configuration for extraction outputs.
 */
class OutputConfigService {
public:
    /**
     * @brief Validates that a configured output root is usable as a local filesystem path.
     *
     * @param outputRoot Candidate output root.
     * @return Safe validation findings; empty means valid for this implementation slice.
     * @post Findings do not expose source PDF text or reviewed page text.
     * Side effects: Reads filesystem metadata only.
     * Determinism: Deterministic for stable path and filesystem state.
     */
    std::vector<OutputPathFinding> validateOutputRoot(
        const std::filesystem::path& outputRoot) const;

    /**
     * @brief Applies a valid output root to an existing configuration.
     *
     * @param current Current local app configuration.
     * @param outputRoot Candidate output root.
     * @return Updated configuration or safe validation findings.
     * @post Does not create work-folder artifacts or hard-code corpus paths.
     * Side effects: Reads filesystem metadata only.
     * Determinism: Deterministic for stable inputs and filesystem state.
     */
    OutputConfigResult setOutputRoot(const AppConfiguration& current,
                                     const std::filesystem::path& outputRoot) const;
};

/**
 * @brief Converts artifact retention policy to its stable configuration name.
 *
 * @param policy Retention policy enum.
 * @return Stable lowercase policy name.
 */
std::string artifactRetentionPolicyName(ArtifactRetentionPolicy policy);

} // namespace pte::core
