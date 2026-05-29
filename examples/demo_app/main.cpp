// File: examples/demo_app/main.cpp
// Purpose: Minimal integration demo for the pdf_text_extractor_core library surface.
// Architectural context: HLA-APP consumer; HLA-INV, HLA-WORK, HLA-VALID, HLA-DIAG read-only views.
// Requirement references (illustrative): FR-001, FR-002, FR-004, FR-012, FR-024,
// FR-025, FR-027, FR-029, FR-031, NFR-001, NFR-003, NFR-012.
// Constraints: Local-only; never modifies the source PDF; prints safe summaries only
// (no page text, no source bytes). Work-folder path is user-configurable.
//
// This file is documentation-grade reference code consumed by docs/integration-tutorial.md.
// It links the public static library `pdf_text_extractor_core` and calls documented services.

#include "core/readiness_summary.hpp"
#include "core/volume_bootstrap.hpp"
#include "core/work_folder_validator.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

namespace fs = std::filesystem;
namespace core = pte::core;

struct DemoOptions {
    fs::path sourcePath;
    fs::path workFolder;
    fs::path corpusRoot;
    std::string volumeId{"demo-volume"};
    std::string title{"Demo Volume"};
    bool initOnly = false;
    bool noOcr = false;
    bool useDefaultFixture = false;
};

void printUsage(std::ostream& out) {
    out << "pte_demo_app — minimal integration demo for pdf_text_extractor_core\n\n"
           "Demonstrates the public library surface a consumer integrates against:\n"
           "  1) inventory + work-folder bootstrap   (VolumeBootstrapService)\n"
           "  2) safe readiness summary              (ReadinessSummaryService)\n"
           "  3) structural validation               (WorkFolderValidator)\n\n"
           "Usage:\n"
           "  pte_demo_app --source PATH --work DIR [options]\n"
           "  pte_demo_app --default-fixture --work DIR [options]\n"
           "  pte_demo_app {-h|--help}\n\n"
           "Required:\n"
           "  --source PATH         Source PDF to bootstrap (never modified).\n"
           "                        Mutually exclusive with --default-fixture.\n"
           "  --work DIR            Work-folder root to create or reuse (user-configurable).\n\n"
           "Optional:\n"
           "  --default-fixture     Use the built-in synthetic fixture (tests/fixtures/hello.pdf).\n"
           "                        Only available when the demo was built from the source tree.\n"
           "  --corpus DIR          Corpus root used to derive a relative path in inventory.\n"
           "                        Defaults to the parent directory of --source.\n"
           "  --volume-id ID        Stable volume identifier (default: demo-volume).\n"
           "  --title TEXT          Volume title (default: \"Demo Volume\").\n"
           "  --init-only           Stop after work-folder init; skip Poppler/Tesseract extraction.\n"
           "  --no-ocr              When extracting, skip OCR (run embedded extraction only).\n"
           "  -h, --help            Show this help and exit 0.\n";
}

bool needsValue(int argc, int i) {
    return i + 1 < argc;
}

[[nodiscard]] std::optional<DemoOptions> parseArgs(int argc, char** argv, std::ostream& err) {
    DemoOptions opts;
    std::optional<fs::path> source;
    std::optional<fs::path> work;
    std::optional<fs::path> corpus;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(std::cout);
            std::exit(0);
        }
        if (arg == "--init-only") {
            opts.initOnly = true;
            continue;
        }
        if (arg == "--no-ocr") {
            opts.noOcr = true;
            continue;
        }
        if (arg == "--default-fixture") {
            opts.useDefaultFixture = true;
            continue;
        }
        if (!needsValue(argc, i)) {
            err << "missing value after " << arg << '\n';
            return std::nullopt;
        }
        const std::string value = argv[++i];
        if (arg == "--source") {
            source = value;
        } else if (arg == "--work") {
            work = value;
        } else if (arg == "--corpus") {
            corpus = value;
        } else if (arg == "--volume-id") {
            opts.volumeId = value;
        } else if (arg == "--title") {
            opts.title = value;
        } else {
            err << "unknown option " << arg << '\n';
            return std::nullopt;
        }
    }

    if (opts.useDefaultFixture && source.has_value()) {
        err << "--default-fixture is mutually exclusive with --source\n";
        return std::nullopt;
    }
    if (opts.useDefaultFixture) {
#if defined(PTE_DEMO_DEFAULT_FIXTURE)
        opts.sourcePath = fs::path(PTE_DEMO_DEFAULT_FIXTURE);
#else
        err << "--default-fixture not available: this build did not embed a fixture path.\n";
        return std::nullopt;
#endif
    } else if (source.has_value()) {
        opts.sourcePath = *source;
    } else {
        err << "missing required --source PATH (or pass --default-fixture)\n";
        return std::nullopt;
    }

    if (!work.has_value()) {
        err << "missing required --work DIR\n";
        return std::nullopt;
    }
    opts.workFolder = *work;

    opts.corpusRoot = corpus.value_or(opts.sourcePath.parent_path());
    return opts;
}

[[nodiscard]] bool validateInputs(const DemoOptions& opts, std::ostream& err) {
    std::error_code ec;
    if (!fs::exists(opts.sourcePath, ec) || ec) {
        err << "source PDF not found: " << opts.sourcePath << '\n';
        return false;
    }
    if (!fs::is_regular_file(opts.sourcePath, ec) || ec) {
        err << "source PDF is not a regular file: " << opts.sourcePath << '\n';
        return false;
    }
    if (opts.workFolder.empty()) {
        err << "work folder path is empty\n";
        return false;
    }
    if (opts.volumeId.empty() || opts.title.empty()) {
        err << "--volume-id and --title must be non-empty\n";
        return false;
    }
    return true;
}

void printBootstrapResult(const core::VolumeBootstrapResult& outcome) {
    std::cout << "[bootstrap] " << outcome.safeMessage << '\n';
    if (outcome.pageCount.has_value()) {
        std::cout << "[bootstrap] page_count: " << *outcome.pageCount << '\n';
    }
    std::cout << "[bootstrap] inventory:   " << (outcome.inventoryRecorded ? "yes" : "no") << '\n';
    std::cout << "[bootstrap] initialized: " << (outcome.workFolderInitialized ? "yes" : "no")
              << '\n';
    std::cout << "[bootstrap] extraction:  " << (outcome.extractionCompleted ? "yes" : "no")
              << '\n';
}

void printReadiness(const core::ReadinessSummary& summary) {
    std::cout << "[readiness] " << summary.safeMessage << '\n';
    if (!summary.success) {
        return;
    }
    std::cout << "[readiness] ocr_used_pages:     " << summary.ocrUsedCount << '\n';
    std::cout << "[readiness] validation_errors:  " << summary.validationErrorCount << '\n';
    std::cout << "[readiness] status_counts:";
    for (const auto& [status, count] : summary.statusCounts) {
        std::cout << ' ' << status << '=' << count;
    }
    std::cout << '\n';
    std::cout << "[readiness] selected_source:";
    for (const auto& [source, count] : summary.selectedSourceCounts) {
        std::cout << ' ' << source << '=' << count;
    }
    std::cout << '\n';
    if (!summary.flagCounts.empty()) {
        std::cout << "[readiness] flag_counts:";
        for (const auto& [flag, count] : summary.flagCounts) {
            std::cout << ' ' << flag << '=' << count;
        }
        std::cout << '\n';
    }
}

void printValidation(const core::ValidationReport& report) {
    std::cout << "[validate] ok: " << (report.ok() ? "yes" : "no")
              << "; error_count: " << report.errorCount()
              << "; total_findings: " << report.findings.size() << '\n';
    for (const auto& finding : report.findings) {
        std::cout << "[validate] " << core::severityName(finding.severity) << ' '
                  << finding.code << " artifact=" << (finding.artifact.empty() ? "-" : finding.artifact)
                  << " page=" << (finding.pageId.empty() ? "-" : finding.pageId)
                  << " repairable=" << (finding.repairable ? "yes" : "no")
                  << " message=" << finding.safeMessage << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    const auto parsed = parseArgs(argc, argv, std::cerr);
    if (!parsed.has_value()) {
        printUsage(std::cerr);
        return 2;
    }
    const DemoOptions opts = *parsed;

    if (!validateInputs(opts, std::cerr)) {
        return 2;
    }

    std::cout << "[demo] source:       " << opts.sourcePath << '\n';
    std::cout << "[demo] work folder:  " << opts.workFolder << '\n';
    std::cout << "[demo] corpus root:  " << opts.corpusRoot << '\n';
    std::cout << "[demo] volume id:    " << opts.volumeId << '\n';
    std::cout << "[demo] title:        " << opts.title << '\n';
    std::cout << "[demo] init only:    " << (opts.initOnly ? "yes" : "no") << '\n';
    std::cout << "[demo] ocr enabled:  " << ((!opts.initOnly && !opts.noOcr) ? "yes" : "no")
              << '\n';

    core::VolumeBootstrapRequest request;
    request.corpusRoot = opts.corpusRoot;
    request.sourcePdfPath = opts.sourcePath;
    request.workFolder = opts.workFolder;
    request.volumeId = opts.volumeId;
    request.title = opts.title;
    request.runCandidateExtraction = !opts.initOnly;
    request.generation.extractEmbedded = true;
    request.generation.extractOcr = !opts.initOnly && !opts.noOcr;

    const core::VolumeBootstrapResult outcome = core::VolumeBootstrapService().run(request);
    printBootstrapResult(outcome);
    if (!outcome.success) {
        std::cerr << "[demo] bootstrap failed; stopping before readiness/validation.\n";
        return 1;
    }

    const core::ReadinessSummary summary =
        core::ReadinessSummaryService().summarizeWorkFolder(opts.workFolder);
    printReadiness(summary);

    const core::ValidationReport report = core::WorkFolderValidator().validate(opts.workFolder);
    printValidation(report);

    std::cout << "[demo] done. Inspect artifacts under: " << opts.workFolder << '\n';
    std::cout << "[demo] source PDF was not modified: " << opts.sourcePath << '\n';
    return 0;
}
