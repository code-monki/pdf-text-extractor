// File: src/core/dependency_capability.hpp
// Purpose: Report local tool capability metadata for dependency-gated operations.
// Architectural context: HLA-DEP with HLA-DIAG, HLA-OCR, HLA-PKG.
// Requirement references: FR-031, NFR-004, NFR-010, NFR-014.
// Test references: TC-FR-031, TC-NFR-004, TC-NFR-010, TC-NFR-014.
// Constraint: This core slice detects local executable availability without running tools.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Tool detection request for one dependency.
 *
 * The request supports an explicit configured path first, then PATH-like search using
 * executableName. Version is optional caller-supplied metadata until process-adapter probing lands.
 */
struct ToolCheckRequest {
    std::string toolId;
    std::string executableName;
    std::filesystem::path configuredPath;
    bool required = false;
    std::string version;
    std::vector<std::string> supportedFeatures;
};

/**
 * @brief Safe capability result for one local tool.
 */
struct ToolCapability {
    std::string toolId;
    bool detected = false;
    bool required = false;
    std::string version;
    std::filesystem::path path;
    std::vector<std::string> supportedFeatures;
    std::string safeMessage;
};

/**
 * @brief Capability report for dependency-gated operations.
 */
struct CapabilityReport {
    std::vector<ToolCapability> tools;

    /**
     * @brief Reports whether all required tools were detected.
     *
     * @return true when missingRequiredCount() is zero.
     * Side effects: None.
     * Determinism: Deterministic for the stored tool capabilities.
     */
    bool ok() const;

    /**
     * @brief Counts required tools that were not detected.
     *
     * @return Number of required missing capabilities.
     * Side effects: None.
     * Determinism: Deterministic for the stored tool capabilities.
     */
    int missingRequiredCount() const;
};

/**
 * @brief Detects local dependency capability without invoking external tools.
 *
 * This service checks filesystem availability and executable permission where supported. It does
 * not run OCR/PDF tools, preventing extraction side effects and keeping tests synthetic.
 */
class DependencyCapabilityService {
public:
    /**
     * @brief Checks one tool request.
     *
     * @param request Tool request to check.
     * @param searchPath PATH-like directory list used when configuredPath is empty.
     * @return Safe capability result with detected path, version metadata, and message.
     * @post Missing required tools return detected=false and an actionable safeMessage.
     * Side effects: Reads local filesystem metadata only.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    ToolCapability checkTool(const ToolCheckRequest& request,
                             const std::string& searchPath) const;

    /**
     * @brief Checks multiple tool requests into one report.
     *
     * @param requests Tool requests to check.
     * @param searchPath PATH-like directory list used when configuredPath is empty.
     * @return Aggregate capability report.
     * Side effects: Reads local filesystem metadata only.
     * Determinism: Deterministic for stable request values and filesystem state.
     */
    CapabilityReport detectCapabilities(const std::vector<ToolCheckRequest>& requests,
                                         const std::string& searchPath) const;
};

} // namespace pte::core
