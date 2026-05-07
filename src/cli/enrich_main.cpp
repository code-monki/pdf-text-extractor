// File: src/cli/enrich_main.cpp
// Purpose: Headless prototype CLI for derived PDF enrichment artifacts.
// Requirement references: FR-034, FR-035, FR-036.

#include "core/pdf_enrichment.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

void printUsage(std::ostream& out) {
    out << "pte_enrich — prototype derived-PDF enrichment (outline/link maps)\n\n"
           "Required:\n"
           "  --source PATH       Source PDF (never modified)\n"
           "  --output-pdf PATH   Derived PDF output path (required unless --lint-only)\n\n"
           "Optional:\n"
           "  --outline-map PATH  Outline sidecar JSON\n"
           "  --link-map PATH     Link sidecar JSON\n"
           "  --report PATH       Enrichment report JSON output path\n"
           "  --python PATH       Python executable override (default: python3)\n"
           "  --script PATH       Enrichment script override (default: <repo>/scripts/pdf_enrich_apply.py)\n"
           "  --lint-only         Validate maps/targets only; no PDF output write\n"
           "  --prototype-only    Disable annotation injection; produce copy+report only\n"
           "  -h, --help          Show this help\n";
}

bool needsValue(int argc, int i) {
    return i + 1 < argc;
}

} // namespace

int main(int argc, char** argv) {
    pte::core::PdfEnrichmentRequest request;
    std::optional<std::filesystem::path> source;
    std::optional<std::filesystem::path> output;
    bool prototypeOnly = false;
    bool lintOnly = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(std::cout);
            return 0;
        }
        if (arg == "--prototype-only") {
            prototypeOnly = true;
            continue;
        }
        if (arg == "--lint-only") {
            lintOnly = true;
            continue;
        }
        if (!needsValue(argc, i)) {
            std::cerr << "missing value after " << arg << '\n';
            printUsage(std::cerr);
            return 2;
        }
        const std::string value = argv[++i];
        if (arg == "--source") {
            source = value;
        } else if (arg == "--output-pdf") {
            output = value;
        } else if (arg == "--outline-map") {
            request.outlineMapPath = std::filesystem::path(value);
        } else if (arg == "--link-map") {
            request.linkMapPath = std::filesystem::path(value);
        } else if (arg == "--report") {
            request.reportPath = std::filesystem::path(value);
        } else if (arg == "--python") {
            request.pythonExecutable = std::filesystem::path(value);
        } else if (arg == "--script") {
            request.pythonScriptPath = std::filesystem::path(value);
        } else {
            std::cerr << "unknown option " << arg << '\n';
            printUsage(std::cerr);
            return 2;
        }
    }

    if (!source || (!output && !lintOnly)) {
        std::cerr << "missing required arguments\n";
        printUsage(std::cerr);
        return 2;
    }
    request.sourcePdfPath = *source;
    if (output.has_value()) {
        request.derivedPdfPath = *output;
    }
    request.enableAnnotationInjection = !prototypeOnly;
    request.lintOnly = lintOnly;
#if defined(PTE_REPO_ROOT)
    if (!request.pythonScriptPath.has_value()) {
        const auto repoDefault =
            std::filesystem::path(PTE_REPO_ROOT) / "scripts" / "pdf_enrich_apply.py";
        if (std::filesystem::exists(repoDefault)) {
            request.pythonScriptPath = repoDefault;
        }
    }
#endif
    if (!request.pythonScriptPath.has_value()) {
        const auto exePath = std::filesystem::absolute(std::filesystem::path(argv[0]));
        const auto sibling = exePath.parent_path() / "pdf_enrich_apply.py";
        if (std::filesystem::exists(sibling)) {
            request.pythonScriptPath = sibling;
        }
    }

    const pte::core::PdfEnrichmentResult result = pte::core::PdfEnrichmentService().run(request);
    if (!result.success) {
        std::cerr << result.safeMessage << '\n';
        return 1;
    }

    std::cout << result.safeMessage << '\n';
    std::cout << "outline_nodes: " << result.outlineNodeCount << '\n';
    std::cout << "links: " << result.linkCount << '\n';
    std::cout << "manual_overrides: " << result.manualOverrideCount << '\n';
    std::cout << "derived_pdf: " << (result.derivedPdfWritten ? "yes" : "no") << '\n';
    return 0;
}

