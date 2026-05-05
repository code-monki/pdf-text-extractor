// File: src/core/poppler_embedded_text.cpp
// Purpose: Implement pdftotext single-page extraction.

#include "core/poppler_embedded_text.hpp"

#include "core/process_runner.hpp"

#include <filesystem>
#include <vector>

namespace pte::core {

EmbeddedTextExtractOutcome PopplerEmbeddedTextExtractor::extractPage(
    const std::filesystem::path& pdfPath,
    int pdfPageOneBased,
    const Options& options) const {
    EmbeddedTextExtractOutcome outcome;
    std::error_code error;
    if (pdfPath.empty() || !std::filesystem::is_regular_file(pdfPath, error) || error) {
        outcome.safeMessage = "pdftotext source is not a readable file";
        return outcome;
    }
    if (pdfPageOneBased < 1) {
        outcome.safeMessage = "PDF page number must be positive";
        return outcome;
    }

    const std::string pageText = std::to_string(pdfPageOneBased);
    std::vector<std::string> argv = {
        "pdftotext",
        "-f",
        pageText,
        "-l",
        pageText,
        pdfPath.string(),
        "-",
    };

    const auto run = runProcessArgv(options.pdftotextExecutable, argv, {});
    outcome.exitCode = run.exitCode;

    if (run.spawnFailed) {
        outcome.safeMessage = "pdftotext could not be started";
        return outcome;
    }
    if (run.exitCode != 0) {
        outcome.safeMessage = "pdftotext exited with non-zero status";
        return outcome;
    }

    outcome.text = run.stdoutText;
    outcome.charCount = outcome.text.size();
    outcome.success = true;
    outcome.safeMessage = "embedded text extracted";
    return outcome;
}

} // namespace pte::core
