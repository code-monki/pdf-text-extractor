// File: src/cli/bootstrap_main.cpp
// Purpose: Headless CLI for VolumeBootstrapService (local volume creation from a source PDF).
// Architectural context: HLA-APP, HLA-PKG orchestration entry.
// Requirement references: FR-004, FR-024, FR-031, NFR-014.
// Constraint: Prints safe messages only; never emits extracted page text.
// Operator documentation: docs/cli-reference.md (keep switches in sync with printUsage() below).

#include "core/volume_bootstrap.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

void printUsage(std::ostream& out) {
    out << "pte_bootstrap — create a work folder from a source PDF (inventory, init, optional extraction)\n\n"
           "Required:\n"
           "  --corpus DIR       Corpus root for relative path display in inventory\n"
           "  --source PATH      Source PDF file (not modified)\n"
           "  --work DIR         Work folder to create or populate\n"
           "  --volume-id ID     Stable volume identifier\n"
           "  --title TEXT       Volume title\n\n"
           "Optional:\n"
           "  --subtitle TEXT\n"
           "  --sort-title TEXT\n"
           "  --group TEXT\n"
           "  --init-only        Initialize work folder only (skip candidate extraction)\n"
           "  --no-ocr           Run embedded extraction only (no Tesseract), when not --init-only\n"
           "  --pdfinfo PATH     Override pdfinfo executable\n"
           "  --pdftotext PATH   Override pdftotext executable\n"
           "  --pdftoppm PATH    Override pdftoppm executable\n"
           "  --tesseract PATH   Override tesseract executable\n"
           "  -h, --help         Show this help\n";
}

bool needsValue(int argc, int i) {
    return i + 1 < argc;
}

} // namespace

int main(int argc, char** argv) {
    pte::core::VolumeBootstrapRequest request;
    bool initOnly = false;
    bool noOcr = false;
    std::optional<std::filesystem::path> corpus;
    std::optional<std::filesystem::path> source;
    std::optional<std::filesystem::path> work;
    std::optional<std::string> volumeId;
    std::optional<std::string> title;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(std::cout);
            return 0;
        }
        if (arg == "--init-only") {
            initOnly = true;
            continue;
        }
        if (arg == "--no-ocr") {
            noOcr = true;
            continue;
        }

        if (!needsValue(argc, i)) {
            std::cerr << "missing value after " << arg << '\n';
            printUsage(std::cerr);
            return 2;
        }

        const std::string value = argv[++i];
        if (arg == "--corpus") {
            corpus = value;
        } else if (arg == "--source") {
            source = value;
        } else if (arg == "--work") {
            work = value;
        } else if (arg == "--volume-id") {
            volumeId = value;
        } else if (arg == "--title") {
            title = value;
        } else if (arg == "--subtitle") {
            request.subtitle = value;
        } else if (arg == "--sort-title") {
            request.sortTitle = value;
        } else if (arg == "--group") {
            request.group = value;
        } else if (arg == "--pdfinfo") {
            request.pdfInspection.pdfinfoExecutable = value;
        } else if (arg == "--pdftotext") {
            request.extractionTools.pdftotextExecutable = value;
        } else if (arg == "--pdftoppm") {
            request.extractionTools.pdftoppmExecutable = value;
        } else if (arg == "--tesseract") {
            request.extractionTools.tesseractExecutable = value;
        } else {
            std::cerr << "unknown option " << arg << '\n';
            printUsage(std::cerr);
            return 2;
        }
    }

    if (!corpus || !source || !work || !volumeId || !title) {
        std::cerr << "missing required arguments\n";
        printUsage(std::cerr);
        return 2;
    }

    request.corpusRoot = *corpus;
    request.sourcePdfPath = *source;
    request.workFolder = *work;
    request.volumeId = *volumeId;
    request.title = *title;
    request.runCandidateExtraction = !initOnly;
    request.generation.extractEmbedded = true;
    request.generation.extractOcr = !initOnly && !noOcr;

    const pte::core::VolumeBootstrapResult outcome =
        pte::core::VolumeBootstrapService().run(request);

    if (!outcome.success) {
        std::cerr << outcome.safeMessage << '\n';
        return 1;
    }

    std::cout << outcome.safeMessage << '\n';
    if (outcome.pageCount.has_value()) {
        std::cout << "page_count: " << *outcome.pageCount << '\n';
    }
    std::cout << "inventory: " << (outcome.inventoryRecorded ? "yes" : "no") << '\n';
    std::cout << "initialized: " << (outcome.workFolderInitialized ? "yes" : "no") << '\n';
    std::cout << "extraction: " << (outcome.extractionCompleted ? "yes" : "no") << '\n';
    return 0;
}
