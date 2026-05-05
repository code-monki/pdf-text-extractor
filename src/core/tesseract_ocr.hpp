// File: src/core/tesseract_ocr.hpp
// Purpose: OCR candidate generation via Tesseract CLI (HLA-OCR).
// Architectural context: HLA-OCR, HLA-DEP.
// Requirement references: FR-013, FR-014, FR-015, FR-024, FR-031, NFR-004, NFR-010, NFR-012.
// Constraint: Raw OCR output only; review-gated per OCR-BND-001.

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace pte::core {

/**
 * @brief OCR text outcome from tesseract writing to stdout.
 */
struct TesseractOcrOutcome {
    bool success = false;
    std::string text;
    std::size_t charCount = 0;
    std::string engineId = "tesseract";
    std::string safeMessage;
    int exitCode = -1;
};

/**
 * @brief Runs Tesseract on a raster image file.
 */
class TesseractOcrRunner {
public:
    struct Options {
        std::optional<std::filesystem::path> tesseractExecutable;
        std::string language = "eng";
        /** Page segmentation mode (e.g. 3 = fully automatic). */
        std::string pageSegmentationMode = "3";
    };

    /**
     * @brief Invokes `tesseract <image> stdout -l <lang> --psm <mode>`.
     */
    TesseractOcrOutcome runOnImageFile(const std::filesystem::path& imagePath,
                                       const Options& options) const;
};

} // namespace pte::core
