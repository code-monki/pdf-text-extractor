// File: src/core/tesseract_ocr.cpp
// Purpose: Tesseract subprocess OCR.

#include "core/tesseract_ocr.hpp"

#include "core/process_runner.hpp"

#include <filesystem>
#include <vector>

namespace pte::core {

TesseractOcrOutcome TesseractOcrRunner::runOnImageFile(const std::filesystem::path& imagePath,
                                                       const Options& options) const {
    TesseractOcrOutcome outcome;
    std::error_code error;
    if (imagePath.empty() || !std::filesystem::is_regular_file(imagePath, error) || error) {
        outcome.safeMessage = "OCR image path is not a readable file";
        return outcome;
    }

    std::vector<std::string> argv = {
        "tesseract",
        imagePath.string(),
        "stdout",
        "-l",
        options.language,
        "--psm",
        options.pageSegmentationMode,
    };

    const auto run = runProcessArgv(options.tesseractExecutable, argv, {});
    outcome.exitCode = run.exitCode;

    if (run.spawnFailed) {
        outcome.safeMessage = "tesseract could not be started";
        return outcome;
    }
    if (run.exitCode != 0) {
        outcome.safeMessage = "tesseract exited with non-zero status";
        return outcome;
    }

    outcome.text = run.stdoutText;
    outcome.charCount = outcome.text.size();
    outcome.success = true;
    outcome.safeMessage = "OCR text captured";
    return outcome;
}

} // namespace pte::core
