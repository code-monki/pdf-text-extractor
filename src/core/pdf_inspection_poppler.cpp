// File: src/core/pdf_inspection_poppler.cpp
// Purpose: Implement argv-only pdfinfo inspection for PopplerPdfInspectionService.
// Architectural context: HLA-PDF inspection path, ADR-0005.
// Requirement references: FR-002, NFR-004.
// Constraint: Source PDF is never modified; no shell invocation.

#include "core/pdf_inspection_poppler.hpp"

#include "core/process_runner.hpp"

#include <cctype>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

namespace pte::core {

namespace {

std::string trimCopy(std::string_view text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return std::string(text.substr(begin, end - begin));
}

std::string asciiLowerCopy(std::string_view text) {
    std::string out(text.begin(), text.end());
    for (auto& ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

std::optional<int> parseFirstInteger(std::string_view text) {
    std::size_t index = 0;
    while (index < text.size() && !std::isdigit(static_cast<unsigned char>(text[index]))) {
        ++index;
    }
    if (index >= text.size()) {
        return std::nullopt;
    }
    int value = 0;
    while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index]))) {
        value = value * 10 + (text[index] - '0');
        ++index;
        if (value > 1000000) {
            return std::nullopt;
        }
    }
    return value;
}

} // namespace

PdfInspectionEvidence parsePdfinfoStdout(const std::string& stdoutText) {
    PdfInspectionEvidence evidence;
    std::istringstream stream(stdoutText);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        const auto key = asciiLowerCopy(trimCopy(line.substr(0, colon)));
        const auto value = trimCopy(line.substr(colon + 1));
        if (key == "pages") {
            const auto pages = parseFirstInteger(value);
            if (pages.has_value()) {
                evidence.pageCount = *pages;
            }
        } else if (key == "encrypted") {
            const auto lv = asciiLowerCopy(value);
            evidence.encrypted =
                lv.find("yes") != std::string::npos || lv.find("true") != std::string::npos;
        }
    }
    return evidence;
}

PopplerPdfInspectionService::InspectResult PopplerPdfInspectionService::inspect(
    const std::filesystem::path& pdfPath,
    const Options& options) const {
    InspectResult result;
#if defined(_WIN32)
    (void)options;
    result.safeMessage = "pdfinfo inspection is not implemented on Windows in this build slice";
    result.evidence.malformed = true;
    result.exitCode = -1;
    return result;
#else
    std::error_code error;
    if (pdfPath.empty() || !std::filesystem::is_regular_file(pdfPath, error) || error) {
        result.safeMessage = "pdfinfo target is not a readable file";
        result.evidence.malformed = true;
        result.exitCode = -1;
        return result;
    }

    const std::vector<std::string> argv = {"pdfinfo", pdfPath.string()};
    ProcessRunOptions runOptions;
    runOptions.mergeStderrIntoStdout = true;
    const auto optionalExe =
        options.pdfinfoExecutable.empty()
            ? std::optional<std::filesystem::path>{}
            : std::optional<std::filesystem::path>{options.pdfinfoExecutable};
    const auto run = runProcessArgv(optionalExe, argv, runOptions);

    if (run.spawnFailed) {
        result.safeMessage = "pdfinfo process could not be started";
        result.evidence.malformed = true;
        result.exitCode = -1;
        return result;
    }

    result.exitCode = run.exitCode;
    result.evidence = parsePdfinfoStdout(run.stdoutText);

    if (run.exitCode != 0) {
        result.evidence.malformed = true;
        result.evidence.readable = false;
        result.safeMessage = "pdfinfo exited with non-zero status";
        return result;
    }

    if (!result.evidence.pageCount.has_value()) {
        result.evidence.malformed = true;
        result.evidence.readable = false;
        result.safeMessage = "pdfinfo did not report a page count";
        return result;
    }

    if (*result.evidence.pageCount <= 0) {
        result.evidence.readable = false;
        result.evidence.malformed = true;
        result.safeMessage = "pdfinfo reported an invalid page count";
        return result;
    }

    result.evidence.readable = true;
    result.safeMessage = "pdfinfo inspection completed";
    return result;
#endif
}

} // namespace pte::core
