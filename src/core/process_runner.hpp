// File: src/core/process_runner.hpp
// Purpose: Argv-only subprocess execution with stdout/stderr capture (no shell).
// Architectural context: HLA-PDF, HLA-EXT, HLA-OCR, HLA-DEP.
// Requirement references: FR-012, FR-013, FR-014, FR-031, NFR-004, NFR-010.
// Constraint: External tools are invoked only through this runner or equivalent argv exec.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Optional controls for child standard streams.
 */
struct ProcessRunOptions {
    /** When true, child STDERR is merged into the stdout capture (like 2>&1). */
    bool mergeStderrIntoStdout = false;
};

/**
 * @brief Output from a finished child process.
 */
struct ProcessRunOutcome {
    std::string stdoutText;
    std::string stderrText;
    int exitCode = -1;
    bool spawnFailed = false;
};

/**
 * @brief Runs an executable with argv list (argv[0] is the program name for PATH lookup).
 *
 * @param executableOverride When set, exec this path; otherwise resolve argv[0] via PATH.
 * @param argv Argument vector including argv[0]; must be non-empty.
 * @param options Stream redirection options.
 */
ProcessRunOutcome runProcessArgv(const std::optional<std::filesystem::path>& executableOverride,
                                 const std::vector<std::string>& argv,
                                 const ProcessRunOptions& options = {});

} // namespace pte::core
