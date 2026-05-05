// File: src/core/dependency_capability.cpp
// Purpose: Implement safe local dependency capability detection.
// Architectural context: HLA-DEP with HLA-DIAG, HLA-OCR, HLA-PKG.
// Requirement references: FR-031, NFR-004, NFR-010, NFR-014.
// Test references: TC-FR-031, TC-NFR-004, TC-NFR-010, TC-NFR-014.
// Assumption: Version probing will be added later through a controlled process adapter.
// Constraint: This implementation reads filesystem metadata only and does not execute tools.

#include "core/dependency_capability.hpp"

#include <cstdlib>
#include <sstream>
#include <utility>

namespace pte::core {

namespace {

/**
 * @brief Returns the platform search-path separator.
 */
char pathSeparator() {
#ifdef _WIN32
    return ';';
#else
    return ':';
#endif
}

/**
 * @brief Checks whether a path points to a usable executable candidate.
 *
 * @param path Candidate executable path.
 * @return true when the path is a regular file with executable permission where applicable.
 * Side effects: Reads filesystem metadata only.
 */
bool isExecutableFile(const std::filesystem::path& path) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error) || error) {
        return false;
    }
#ifdef _WIN32
    return true;
#else
    const auto permissions = std::filesystem::status(path, error).permissions();
    if (error) {
        return false;
    }
    using Permissions = std::filesystem::perms;
    return (permissions & Permissions::owner_exec) != Permissions::none
        || (permissions & Permissions::group_exec) != Permissions::none
        || (permissions & Permissions::others_exec) != Permissions::none;
#endif
}

/**
 * @brief Splits a PATH-like string into directory entries.
 *
 * @param searchPath PATH-like directory list.
 * @return Directory entries in order.
 */
std::vector<std::filesystem::path> splitSearchPath(const std::string& searchPath) {
    std::vector<std::filesystem::path> paths;
    std::stringstream stream(searchPath);
    std::string item;
    while (std::getline(stream, item, pathSeparator())) {
        if (!item.empty()) {
            paths.emplace_back(item);
        }
    }
    return paths;
}

/**
 * @brief Finds an executable by name in a supplied PATH-like search path.
 *
 * @param executableName Executable filename to locate.
 * @param searchPath PATH-like directory list.
 * @return First matching executable path, or empty path when not found.
 */
std::filesystem::path findOnSearchPath(const std::string& executableName,
                                       const std::string& searchPath) {
    if (executableName.empty()) {
        return {};
    }
    for (const auto& directory : splitSearchPath(searchPath)) {
        const auto candidate = directory / executableName;
        if (isExecutableFile(candidate)) {
            return candidate;
        }
#ifdef _WIN32
        const auto exeCandidate = directory / (executableName + ".exe");
        if (isExecutableFile(exeCandidate)) {
            return exeCandidate;
        }
#endif
    }
    return {};
}

/**
 * @brief Builds a safe missing-tool message without source-content data.
 *
 * @param request Tool request that failed detection.
 * @return Safe actionable message.
 */
std::string missingMessage(const ToolCheckRequest& request) {
    if (request.required) {
        return "required tool '" + request.toolId + "' is not available";
    }
    return "optional tool '" + request.toolId + "' is not available";
}

} // namespace

/**
 * @brief Reports whether all required tools were detected.
 */
bool CapabilityReport::ok() const {
    return missingRequiredCount() == 0;
}

/**
 * @brief Counts missing required tool capabilities.
 */
int CapabilityReport::missingRequiredCount() const {
    int count = 0;
    for (const auto& tool : tools) {
        if (tool.required && !tool.detected) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Checks one configured or PATH-discovered tool without executing it.
 */
ToolCapability DependencyCapabilityService::checkTool(
    const ToolCheckRequest& request,
    const std::string& searchPath) const {
    ToolCapability capability;
    capability.toolId = request.toolId;
    capability.required = request.required;
    capability.version = request.version;
    capability.supportedFeatures = request.supportedFeatures;

    if (!request.configuredPath.empty()) {
        capability.path = request.configuredPath;
        capability.detected = isExecutableFile(request.configuredPath);
    } else {
        capability.path = findOnSearchPath(request.executableName, searchPath);
        capability.detected = !capability.path.empty();
    }

    if (capability.detected) {
        capability.safeMessage = "tool '" + capability.toolId + "' is available";
    } else {
        capability.safeMessage = missingMessage(request);
    }

    return capability;
}

/**
 * @brief Checks multiple local tool dependencies.
 */
CapabilityReport DependencyCapabilityService::detectCapabilities(
    const std::vector<ToolCheckRequest>& requests,
    const std::string& searchPath) const {
    CapabilityReport report;
    for (const auto& request : requests) {
        report.tools.push_back(checkTool(request, searchPath));
    }
    return report;
}

} // namespace pte::core
