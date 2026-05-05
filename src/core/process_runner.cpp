// File: src/core/process_runner.cpp
// Purpose: POSIX argv subprocess runner with captured pipes.
// Architectural context: shared adapter infrastructure.

#include "core/process_runner.hpp"

#include <array>
#include <cstring>

#if defined(_WIN32)

namespace pte::core {

ProcessRunOutcome runProcessArgv(const std::optional<std::filesystem::path>& executableOverride,
                                 const std::vector<std::string>& argv,
                                 const ProcessRunOptions& options) {
    (void)executableOverride;
    (void)argv;
    (void)options;
    ProcessRunOutcome outcome;
    outcome.spawnFailed = true;
    outcome.stderrText = "process_runner is not implemented on Windows in this build slice";
    return outcome;
}

} // namespace pte::core

#else

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace pte::core {

namespace {

void drainPipe(int fd, std::string& sink) {
    std::array<char, 8192> buffer{};
    while (true) {
        const ssize_t n = read(fd, buffer.data(), buffer.size());
        if (n > 0) {
            sink.append(buffer.data(), static_cast<std::size_t>(n));
            continue;
        }
        break;
    }
}

} // namespace

ProcessRunOutcome runProcessArgv(const std::optional<std::filesystem::path>& executableOverride,
                                 const std::vector<std::string>& argv,
                                 const ProcessRunOptions& options) {
    ProcessRunOutcome outcome;
    if (argv.empty()) {
        outcome.spawnFailed = true;
        outcome.stderrText = "argv must be non-empty";
        return outcome;
    }

    std::array<int, 2> outPipe{};
    std::array<int, 2> errPipe{};
    if (pipe(outPipe.data()) < 0) {
        outcome.spawnFailed = true;
        return outcome;
    }
    if (!options.mergeStderrIntoStdout && pipe(errPipe.data()) < 0) {
        close(outPipe[0]);
        close(outPipe[1]);
        outcome.spawnFailed = true;
        return outcome;
    }

    std::vector<std::string> argvStorage = argv;
    std::vector<char*> argvPtr;
    argvPtr.reserve(argvStorage.size() + 1);
    for (auto& piece : argvStorage) {
        argvPtr.push_back(piece.data());
    }
    argvPtr.push_back(nullptr);

    const pid_t pid = fork();
    if (pid < 0) {
        close(outPipe[0]);
        close(outPipe[1]);
        if (!options.mergeStderrIntoStdout) {
            close(errPipe[0]);
            close(errPipe[1]);
        }
        outcome.spawnFailed = true;
        return outcome;
    }

    if (pid == 0) {
        close(outPipe[0]);
        if (dup2(outPipe[1], STDOUT_FILENO) < 0) {
            _exit(126);
        }
        if (options.mergeStderrIntoStdout) {
            if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
                _exit(126);
            }
        } else {
            close(errPipe[0]);
            if (dup2(errPipe[1], STDERR_FILENO) < 0) {
                _exit(126);
            }
            close(errPipe[1]);
        }
        close(outPipe[1]);

        if (executableOverride.has_value()) {
            execv(executableOverride->c_str(), argvPtr.data());
        } else {
            execvp(argvPtr[0], argvPtr.data());
        }
        _exit(127);
    }

    close(outPipe[1]);
    if (!options.mergeStderrIntoStdout) {
        close(errPipe[1]);
    }

    drainPipe(outPipe[0], outcome.stdoutText);
    close(outPipe[0]);
    if (!options.mergeStderrIntoStdout) {
        drainPipe(errPipe[0], outcome.stderrText);
        close(errPipe[0]);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        outcome.exitCode = -1;
        return outcome;
    }
    if (WIFEXITED(status)) {
        outcome.exitCode = WEXITSTATUS(status);
    } else {
        outcome.exitCode = -1;
    }
    return outcome;
}

} // namespace pte::core

#endif
