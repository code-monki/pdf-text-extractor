// File: tests/core_tests.cpp
// Purpose: Exercise the first implementation slice for work-folder validation and review gating.
// Architectural context: HLA-INV, HLA-WORK, HLA-VALID, HLA-REVIEW, HLA-DEP, HLA-TEXT,
// HLA-DIAG, HLA-EXT, HLA-OCR, HLA-APP, HLA-CONFIG, HLA-SAFE.
// Requirement references: FR-001, FR-002, FR-003, FR-004, FR-005, FR-006, FR-007,
// FR-008, FR-009, FR-010, FR-011,
// FR-012, FR-013, FR-014, FR-015, FR-016, FR-017, FR-018, FR-022, FR-024, FR-025,
// FR-021, FR-026, FR-027, FR-029, FR-030, FR-031, FR-032, NFR-001, NFR-002,
// NFR-003, NFR-004, NFR-005, NFR-007, NFR-008, NFR-010, NFR-011, NFR-012,
// NFR-014.
// Test references: TC-FR-001, TC-FR-002, TC-FR-003, TC-FR-004, TC-FR-005,
// TC-FR-006, TC-FR-007, TC-FR-008, TC-FR-009, TC-FR-010, TC-FR-011,
// TC-FR-016, TC-FR-012, TC-FR-013, TC-FR-014, TC-FR-015,
// TC-FR-017, TC-FR-018, TC-FR-021, TC-FR-022, TC-FR-024, TC-FR-025,
// TC-FR-026,
// TC-FR-027, TC-FR-029, TC-FR-030, TC-FR-031, TC-NFR-001, TC-NFR-002,
// TC-NFR-003, TC-NFR-004, TC-NFR-005, TC-NFR-008, TC-NFR-010, TC-NFR-011,
// TC-NFR-012, TC-FR-032, TC-NFR-007, TC-NFR-014, TC-BND-OCR-001,
// TC-BND-OCR-002, TC-BND-OCR-005.
// Assumption: Synthetic fixture text is non-copyrighted and safe to commit.

#include "core/artifact_cleanup.hpp"
#include "core/candidate_text.hpp"
#include "core/dependency_capability.hpp"
#include "core/file_digest.hpp"
#include "core/local_path_intent.hpp"
#include "core/volume_bootstrap.hpp"
#include "core/file_change_detector.hpp"
#include "core/ocr_routing.hpp"
#include "core/output_config.hpp"
#include "core/page_id.hpp"
#include "core/page_quality.hpp"
#include "core/pdf_inspection_poppler.hpp"
#include "core/process_runner.hpp"
#include "core/candidate_generation_poppler.hpp"
#include "core/volume_extraction_pipeline.hpp"
#include "core/readiness_summary.hpp"
#include "core/reviewed_page_text.hpp"
#include "core/review_state.hpp"
#include "core/review_workflow.hpp"
#include "core/source_inventory.hpp"
#include "core/text_normalization.hpp"
#include "core/validation_repair.hpp"
#include "core/volume_metadata.hpp"
#include "core/work_folder_backup.hpp"
#include "core/work_folder_initializer.hpp"
#include "core/work_folder_validator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {

/** @brief Process-wide assertion failure count for the lightweight test harness. */
int failures = 0;

/** @brief Failure count at the start of the currently running test case. */
int testStartFailures = 0;

/**
 * @brief Records a failed assertion without aborting the test process.
 *
 * @param condition Assertion condition.
 * @param message Safe diagnostic message for the failed assertion.
 * Side effects: Increments failures and writes to stderr when condition is false.
 */
void require(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

/**
 * @brief Writes synthetic fixture content to a local test file.
 *
 * @param path Destination path.
 * @param content Non-restricted synthetic content to write.
 * Side effects: Creates parent directories and writes file content.
 * @throws std::runtime_error when the destination file cannot be opened.
 */
void writeText(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("unable to write fixture file");
    }
    output << content;
}

/**
 * @brief Marks a synthetic fixture file executable where the platform supports permissions.
 *
 * @param path Fixture path.
 * Side effects: Updates local synthetic fixture permissions.
 */
void makeExecutable(const fs::path& path) {
    std::error_code error;
    fs::permissions(path,
                    fs::perms::owner_exec | fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::add,
                    error);
}

/**
 * @brief Reads synthetic fixture content from a local test file.
 *
 * @param path Source path.
 * @return File content, or an empty optional when the file cannot be opened.
 * Side effects: Reads a local synthetic fixture file.
 */
std::optional<std::string> readFile(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::string content;
    input.seekg(0, std::ios::end);
    content.resize(static_cast<std::size_t>(input.tellg()));
    input.seekg(0, std::ios::beg);
    input.read(content.data(), static_cast<std::streamsize>(content.size()));
    return content;
}

/**
 * @brief Creates a clean temporary root for core tests.
 *
 * @return Fresh temporary directory path for this test process.
 * Side effects: Removes any prior test directory at the same location.
 */
fs::path makeTempRoot() {
    const auto root = fs::temp_directory_path() / "pdf-text-extractor-core-tests";
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

/**
 * @brief Writes a complete synthetic two-page work-folder fixture.
 *
 * @param root Work-folder root to populate.
 * Side effects: Writes JSON artifacts and page text files under root.
 * @note Fixture page text is synthetic and safe to commit.
 */
void writeValidWorkFolder(const fs::path& root) {
    writeText(root / "volume.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "title": "Synthetic Volume",
  "sourcePdf": {
    "path": "pdfs/synthetic.pdf",
    "filename": "synthetic.pdf",
    "pageCount": 2
  },
  "pageMap": {
    "0001": { "pdfPage": 1, "textPath": "pages/0001.txt", "printedPageLabel": "i" },
    "0002": { "pdfPage": 2, "textPath": "pages/0002.txt", "printedPageLabel": "1" }
  }
})JSON");
    writeText(root / "page-quality.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "pages": {
    "0001": { "pdfPage": 1, "selectedSource": "manual", "flags": [], "errors": [] },
    "0002": { "pdfPage": 2, "selectedSource": "empty", "flags": [], "errors": [] }
  }
})JSON");
    writeText(root / "review-state.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "pages": {
    "0001": {
      "pdfPage": 1,
      "status": "accepted",
      "reviewedAt": null,
      "reviewedBy": null,
      "notes": ""
    },
    "0002": { "pdfPage": 2, "status": "raw", "reviewedAt": null, "reviewedBy": null, "notes": "" }
  }
})JSON");
    writeText(root / "pages" / "0001.txt", "Synthetic searchable page text.\n");
    writeText(root / "pages" / "0002.txt", "");
}

/**
 * @brief Checks whether a validation report contains a finding code.
 *
 * @param report Validation report to inspect.
 * @param code Stable finding code to search for.
 * @return true when any finding has the requested code.
 * Side effects: None.
 */
bool hasCode(const pte::core::ValidationReport& report, const std::string& code) {
    for (const auto& finding : report.findings) {
        if (finding.code == code) {
            return true;
        }
    }
    return false;
}

/** @brief Builds a synthetic volume metadata model for HLA-META tests. */
pte::core::VolumeMetadata syntheticVolumeMetadata() {
    pte::core::VolumeMetadata metadata;
    metadata.volumeId = "synthetic-volume";
    metadata.title = "Synthetic Volume";
    metadata.subtitle = "";
    metadata.sortTitle = "Synthetic Volume";
    metadata.group = "Synthetic Group";
    metadata.sourcePath = "pdfs/synthetic.pdf";
    metadata.sourceFilename = "synthetic.pdf";
    metadata.sourcePageCount = 2;
    metadata.bibliographic = {
        {"publisher", ""},
        {"language", "en"},
        {"productCode", "SYN-001"}
    };
    metadata.pageMap = {
        {"0001", 1, "pages/0001.txt", "", true},
        {"0002", 2, "pages/0002.txt", "", true}
    };
    return metadata;
}

/**
 * @brief Runs one named test function and prints a stable result line.
 *
 * @param name Test case name.
 * @param testFunction Test function to execute.
 * Side effects: Writes PASS/FAIL status to stdout and may update failures through assertions.
 */
void runTest(std::string_view name, void (*testFunction)()) {
    testStartFailures = failures;
    std::cout << "RUN  " << name << '\n';
    try {
        testFunction();
    } catch (const std::exception& error) {
        ++failures;
        std::cerr << "FAIL: " << name << " threw exception: " << error.what() << '\n';
    } catch (...) {
        ++failures;
        std::cerr << "FAIL: " << name << " threw unknown exception\n";
    }

    if (failures == testStartFailures) {
        std::cout << "PASS " << name << '\n';
    } else {
        std::cout << "FAIL " << name << '\n';
    }
}

/** @brief Verifies stable page ID formatting and validation behavior. */
void testPageIds() {
    require(pte::core::pageIdForPdfPage(1) == "0001", "page 1 maps to 0001");
    require(pte::core::pageIdForPdfPage(49) == "0049", "page 49 maps to 0049");
    require(pte::core::isValidPageId("0001"), "0001 is valid page ID");
    require(!pte::core::isValidPageId("0000"), "0000 is not valid page ID");
    require(!pte::core::isValidPageId("49"), "unpadded page ID is invalid");
}

/** @brief Verifies review status vocabulary and default release eligibility gates. */
void testReviewEligibility() {
    require(pte::core::isAllowedReviewStatus("raw"), "raw status is allowed");
    require(pte::core::isAllowedReviewStatus("quarantined"), "quarantined status is allowed");
    require(!pte::core::isAllowedReviewStatus("ocr"), "candidate source is not a review status");
    require(pte::core::isDefaultReleaseEligibleStatus("accepted"),
            "accepted status is release eligible");
    require(!pte::core::isDefaultReleaseEligibleStatus("raw"),
            "raw status is not release eligible");
    require(!pte::core::isDefaultReleaseEligibleStatus("edited"),
            "edited status is not release eligible");
}

/** @brief Verifies approved review state transitions. */
void testReviewTransitionsAllowed() {
    require(pte::core::isAllowedReviewTransition("raw", "editing"),
            "raw can transition to editing");
    require(pte::core::isAllowedReviewTransition("raw", "accepted"),
            "raw can transition to accepted");
    require(pte::core::isAllowedReviewTransition("editing", "edited"),
            "editing can transition to edited");
    require(pte::core::isAllowedReviewTransition("edited", "accepted"),
            "edited can transition to accepted");
    require(pte::core::isAllowedReviewTransition("accepted", "editing"),
            "accepted can transition back to editing");
    require(pte::core::isAllowedReviewTransition("skip", "accepted"),
            "skip can transition to accepted");
    require(pte::core::isAllowedReviewTransition("quarantined", "skip"),
            "quarantined can transition to skip");
}

/** @brief Verifies rejected review state transitions. */
void testReviewTransitionsRejected() {
    require(!pte::core::isAllowedReviewTransition("raw", "edited"),
            "raw cannot transition directly to edited");
    require(!pte::core::isAllowedReviewTransition("accepted", "raw"),
            "accepted cannot transition back to raw");
    require(!pte::core::isAllowedReviewTransition("quarantined", "accepted"),
            "quarantined cannot transition directly to accepted");
    require(!pte::core::isAllowedReviewTransition("ocr", "accepted"),
            "candidate source is not a valid transition source");
}

/** @brief Verifies default release eligibility summary behavior. */
void testReleaseEligibilitySummary() {
    const auto accepted = pte::core::computeReleaseEligibility("accepted");
    const auto raw = pte::core::computeReleaseEligibility("raw");
    const auto invalid = pte::core::computeReleaseEligibility("ocr");
    require(accepted.eligible, "accepted status is eligible");
    require(accepted.reason == "accepted", "accepted eligibility reason is stable");
    require(!raw.eligible, "raw status is not eligible");
    require(raw.reason == "not-accepted", "raw eligibility reason is stable");
    require(!invalid.eligible, "invalid status is not eligible");
    require(invalid.reason == "invalid-status", "invalid eligibility reason is stable");

    const auto summary = pte::core::summarizeReleaseEligibility(
        {"accepted", "raw", "edited", "skip", "ocr"});
    require(summary.eligibleCount == 1, "release summary counts accepted pages");
    require(summary.ineligibleCount == 3, "release summary counts ineligible pages");
    require(summary.invalidStatusCount == 1, "release summary counts invalid statuses");
}

/** @brief Verifies review workflow applies approved transitions and dirty state. */
void testReviewWorkflowTransition() {
    const pte::core::ReviewPageState current{"0001", 1, "raw", "empty", "clean", ""};
    const auto result = pte::core::ReviewWorkflowService().transition(current, "editing");

    require(result.success, "review workflow transition succeeds");
    require(result.state.status == "editing", "review workflow updates status");
    require(result.state.dirtyState == "dirty", "review workflow marks metadata dirty");
}

/** @brief Verifies review workflow rejects candidate-source status transitions. */
void testReviewWorkflowRejectsCandidateStatus() {
    const pte::core::ReviewPageState current{"0001", 1, "raw", "empty", "clean", ""};
    const auto result = pte::core::ReviewWorkflowService().transition(current, "ocr");

    require(!result.success, "review workflow rejects OCR as review status");
    require(result.state.status == "raw", "rejected review workflow preserves status");
}

/** @brief Verifies review workflow finds the next page matching a status filter. */
void testReviewWorkflowNextFilteredPage() {
    const std::vector<pte::core::ReviewPageState> pages{
        {"0001", 1, "accepted", "manual", "clean", ""},
        {"0002", 2, "raw", "empty", "clean", ""},
        {"0003", 3, "quarantined", "empty", "clean", ""}
    };
    const pte::core::ReviewFilter filter{{"raw", "quarantined"}};
    const auto next = pte::core::ReviewWorkflowService().nextPage(pages, filter, "0001");

    require(next && *next == "0002", "review workflow finds next matching page");
}

/** @brief Verifies source PDF inventory records local identity metadata and SHA-256. */
void testSourceInventoryRecordsIdentity() {
    const auto root = makeTempRoot() / "source-inventory";
    const auto corpus = root / "corpus";
    const auto source = corpus / "pdfs" / "synthetic.pdf";
    writeText(source, "abc");

    const pte::core::SourceInventoryRequest request{
        source,
        corpus,
        {2, true, false, false, false}
    };
    const auto record = pte::core::SourceInventoryService().inventory(request);

    require(record.success, "source inventory succeeds");
    require(record.filename == "synthetic.pdf", "source inventory records filename");
    require(record.corpusRelativePath == "pdfs/synthetic.pdf",
            "source inventory records corpus-relative path");
    require(record.fileSize == 3, "source inventory records file size");
    require(record.sha256 == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
            "source inventory records SHA-256");
    require(record.pageCount && *record.pageCount == 2,
            "source inventory preserves adapter-supplied page count");
    require(record.readable, "source inventory preserves readability evidence");
}

/** @brief Verifies source inventory reports unavailable local paths safely. */
void testSourceInventoryMissingPathSafe() {
    const auto root = makeTempRoot() / "source-missing";
    const pte::core::SourceInventoryRequest request{
        root / "missing.pdf",
        root,
        {}
    };
    const auto record = pte::core::SourceInventoryService().inventory(request);

    require(!record.success, "missing source inventory fails");
    require(record.safeMessage.find("missing.pdf") == std::string::npos,
            "missing source inventory message omits source path");
}

/** @brief Verifies source inventory does not modify selected source bytes. */
void testSourceInventoryDoesNotModifySource() {
    const auto root = makeTempRoot() / "source-not-modified";
    const auto source = root / "synthetic.pdf";
    writeText(source, "synthetic source bytes");
    const auto before = readFile(source);

    const pte::core::SourceInventoryRequest request{
        source,
        root,
        {1, true, false, false, false}
    };
    const auto record = pte::core::SourceInventoryService().inventory(request);
    const auto after = readFile(source);

    require(record.success, "source inventory succeeds for non-modification test");
    require(before == after, "source inventory does not modify source file bytes");
}

/** @brief Verifies duplicate source PDFs are detected by checksum without deletion. */
void testSourceInventoryDuplicateDetection() {
    const auto root = makeTempRoot() / "source-duplicates";
    const auto first = root / "first.pdf";
    const auto second = root / "second.pdf";
    writeText(first, "duplicate source bytes");
    writeText(second, "duplicate source bytes");

    const pte::core::SourceInventoryRequest firstRequest{
        first,
        root,
        {1, true, false, false, false}
    };
    const pte::core::SourceInventoryRequest secondRequest{
        second,
        root,
        {1, true, false, false, false}
    };
    const auto firstRecord = pte::core::SourceInventoryService().inventory(firstRequest);
    const auto secondRecord = pte::core::SourceInventoryService().inventory(secondRequest);
    const auto report = pte::core::SourceInventoryService().findDuplicates(
        firstRecord,
        {secondRecord});

    require(report.hasDuplicates(), "duplicate checksum is detected");
    require(report.matches.size() == 1, "duplicate report has one match");
    require(fs::exists(first), "duplicate detection does not delete selected source");
    require(fs::exists(second), "duplicate detection does not delete matching source");
}

/** @brief Verifies pdfinfo stdout parsing extracts pages and encryption hints. */
void testParsePdfinfoStdoutPagesAndEncrypted() {
    const std::string out =
        "Producer:        TeX\n"
        "PDF version:     1.4\n"
        "Pages:           51\n"
        "Encrypted:       yes\n";
    const auto ev = pte::core::parsePdfinfoStdout(out);
    require(ev.pageCount.has_value() && *ev.pageCount == 51, "pdfinfo parser extracts pages");
    require(ev.encrypted == true, "pdfinfo parser extracts encrypted flag");
}

/** @brief Verifies pdfinfo parser tolerates absence of page metadata. */
void testParsePdfinfoStdoutMissingPages() {
    const auto ev = pte::core::parsePdfinfoStdout("Hello: world\n");
    require(!ev.pageCount.has_value(), "pdfinfo parser leaves pages absent");
}

/** @brief Verifies pdfinfo inspection rejects missing files without invoking tools. */
void testPdfInspectionMissingFileSafe() {
    pte::core::PopplerPdfInspectionService service;
    pte::core::PopplerPdfInspectionService::Options options;
    const auto result =
        service.inspect(fs::path("/nonexistent/pdf-text-extractor-core-test.pdf"), options);
    require(result.exitCode == -1, "missing file yields sentinel exit code");
    require(result.evidence.malformed == true, "missing file marks malformed");
    require(!result.safeMessage.empty(), "missing file returns safe message");
}

#if !defined(_WIN32)
/** @brief Verifies argv subprocess captures echo output. */
void testProcessRunnerEchoStdout() {
    const auto run =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"/bin/echo", "pte_ok"});
    require(!run.spawnFailed, "echo spawn succeeds");
    require(run.exitCode == 0, "echo exits zero");
    require(run.stdoutText.find("pte_ok") != std::string::npos, "echo captures stdout");
}

/** @brief Verifies non-zero exit surfaces from child processes. */
void testProcessRunnerFalseExit() {
    const auto run =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"/bin/false"});
    require(!run.spawnFailed, "false spawn succeeds");
    require(run.exitCode != 0, "false exits non-zero");
}

/** @brief Verifies pdfinfo rejects invalid PDF bytes deterministically. */
void testPdfInspectionInvalidPdfFile() {
    const auto root = makeTempRoot() / "pdf-inspection-invalid";
    const auto path = root / "not-valid.pdf";
    writeText(path, "not a pdf");

    pte::core::PopplerPdfInspectionService service;
    pte::core::PopplerPdfInspectionService::Options options;
    const auto result = service.inspect(path, options);
    require(result.exitCode != 0, "invalid pdf yields non-zero pdfinfo exit");
    require(result.evidence.malformed == true, "invalid pdf marks malformed evidence");
}

/**
 * @brief End-to-end Poppler embedded text + optional Tesseract OCR using a synthetic fixture PDF.
 *
 * Skips quietly when Poppler tools are unavailable; asserts OCR only when pdftoppm and tesseract
 * probes succeed.
 */
void testPopplerTesseractFixtureCandidates() {
#ifdef PTE_TEST_FIXTURE_ROOT
    const fs::path pdf =
        fs::path(PTE_TEST_FIXTURE_ROOT) / "hello.pdf";
#else
    const fs::path pdf;
#endif
    if (pdf.empty() || !fs::exists(pdf)) {
        return;
    }

    const auto probeTxt =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftotext", "-v"});
    if (probeTxt.spawnFailed) {
        return;
    }

    const auto probePpm =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftoppm", "-v"});
    const auto probeTess =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"tesseract", "--version"});
    const bool fullOcr =
        !probePpm.spawnFailed && probePpm.exitCode == 0 && !probeTess.spawnFailed
        && probeTess.exitCode == 0;

    pte::core::CandidateGenerationOptions options;
    options.extractEmbedded = true;
    options.extractOcr = fullOcr;

    const auto root = makeTempRoot() / "poppler-tesseract";
    fs::create_directories(root);

    const auto digestBefore = pte::core::sha256HexFile(pdf);
    require(digestBefore.has_value(), "TC-NFR-003: fixture SHA-256 before candidate generation");

    pte::core::PopplerTesseractCandidateGenerator generator;
    pte::core::PopplerTesseractToolPaths tools;
    const auto summary =
        generator.generateForPage(pdf, root, 1, "0001", tools, options);

    require(summary.embeddedStored, "fixture embedded candidate stored");
    const auto embeddedText = readFile(root / "raw" / "embedded" / "0001.txt");
    require(embeddedText.has_value(), "embedded artifact readable");
    require(embeddedText->find("Hello") != std::string::npos,
            "embedded text contains Hello from synthetic fixture");

    if (fullOcr) {
        require(summary.ocrStored, "OCR candidate stored when toolchain complete");
        const auto ocrText = readFile(root / "raw" / "ocr" / "0001.txt");
        require(ocrText.has_value() && !ocrText->empty(), "OCR artifact non-empty");
    }

    const auto digestAfter = pte::core::sha256HexFile(pdf);
    require(digestAfter.has_value() && *digestBefore == *digestAfter,
            "TC-NFR-003: source PDF hash unchanged after embedded/OCR candidate generation");
}

/** @brief Verifies inventory merges pdfinfo structural evidence for a synthetic PDF fixture. */
void testInventoryWithPopplerPdfinfo() {
#ifdef PTE_TEST_FIXTURE_ROOT
    const fs::path pdf = fs::path(PTE_TEST_FIXTURE_ROOT) / "hello.pdf";
#else
    const fs::path pdf;
#endif
    if (pdf.empty() || !fs::exists(pdf)) {
        return;
    }

    const auto corpus = makeTempRoot() / "inventory-pdfinfo-corpus";
    fs::create_directories(corpus);
    pte::core::PopplerPdfInspectionOptions pdfOpts;
    const auto record = pte::core::SourceInventoryService().inventoryWithPopplerInspection(
        pdf, corpus, pdfOpts);

    require(record.success, "inventory with pdfinfo completes");
    require(record.pageCount.has_value() && *record.pageCount == 1, "fixture page count is one");
    require(record.readable, "fixture marked readable");
}

/** @brief Initializes a work folder then runs volume extraction and checks page-quality rewrite. */
void testVolumeExtractRewritesPageQuality() {
#ifdef PTE_TEST_FIXTURE_ROOT
    const fs::path pdf = fs::path(PTE_TEST_FIXTURE_ROOT) / "hello.pdf";
#else
    const fs::path pdf;
#endif
    if (pdf.empty() || !fs::exists(pdf)) {
        return;
    }

    const auto probeTxt =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftotext", "-v"});
    if (probeTxt.spawnFailed) {
        return;
    }

    const auto root = makeTempRoot() / "volume-extract";
    fs::create_directories(root / "corpus");
    const fs::path localPdf = root / "corpus" / "hello.pdf";
    fs::copy_file(pdf, localPdf);

    const auto digestBeforeExtract = pte::core::sha256HexFile(localPdf);
    require(digestBeforeExtract.has_value(), "TC-NFR-003: SHA-256 before volume extraction");

    pte::core::WorkFolderInitRequest initRequest{
        root / "work",
        "fixture-volume",
        "Fixture",
        "",
        "",
        "",
        pte::core::SourcePdfSeed{localPdf, "hello.pdf", 1},
    };
    require(pte::core::WorkFolderInitializer().initialize(initRequest).success,
            "work folder ready for extraction");

    const auto probePpm =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftoppm", "-v"});
    const auto probeTess =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"tesseract", "--version"});
    const bool fullOcr = !probePpm.spawnFailed && probePpm.exitCode == 0 && !probeTess.spawnFailed
        && probeTess.exitCode == 0;

    pte::core::VolumeCandidateExtractRequest extractRequest;
    extractRequest.workFolder = root / "work";
    extractRequest.sourcePdfPath = localPdf;
    extractRequest.volumeId = "fixture-volume";
    extractRequest.pageCount = 1;
    extractRequest.generation.extractEmbedded = true;
    extractRequest.generation.extractOcr = fullOcr;

    const auto extracted =
        pte::core::VolumeCandidateExtractService().extractAllPages(extractRequest);
    require(extracted.success, "volume extraction completes");

    const auto digestAfterExtract = pte::core::sha256HexFile(localPdf);
    require(digestAfterExtract.has_value() && *digestBeforeExtract == *digestAfterExtract,
            "TC-NFR-003: source PDF hash unchanged after volume extraction");

    const auto pageQuality = readFile(root / "work" / "page-quality.json");
    require(pageQuality.has_value(), "page-quality.json readable");
    require(pageQuality->find("pdftotext") != std::string::npos,
            "page-quality records embedded tool metadata");
    require(pageQuality->find("\"available\": true") != std::string::npos,
            "page-quality marks embedded candidate available");
}

/** @brief End-to-end VolumeBootstrapService: inventory → init → extraction using hello.pdf. */
void testVolumeBootstrapEndToEnd() {
#ifdef PTE_TEST_FIXTURE_ROOT
    const fs::path pdf = fs::path(PTE_TEST_FIXTURE_ROOT) / "hello.pdf";
#else
    const fs::path pdf;
#endif
    if (pdf.empty() || !fs::exists(pdf)) {
        return;
    }

    const auto probeTxt =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftotext", "-v"});
    if (probeTxt.spawnFailed) {
        return;
    }

    const auto root = makeTempRoot() / "bootstrap-e2e";
    fs::create_directories(root / "corpus");
    const fs::path localPdf = root / "corpus" / "hello.pdf";
    fs::copy_file(pdf, localPdf);

    const auto digestBeforeBootstrap = pte::core::sha256HexFile(localPdf);
    require(digestBeforeBootstrap.has_value(), "TC-NFR-003: SHA-256 before volume bootstrap");

    const auto probePpm =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"pdftoppm", "-v"});
    const auto probeTess =
        pte::core::runProcessArgv(std::nullopt, std::vector<std::string>{"tesseract", "--version"});
    const bool fullOcr =
        !probePpm.spawnFailed && probePpm.exitCode == 0 && !probeTess.spawnFailed
        && probeTess.exitCode == 0;

    pte::core::VolumeBootstrapRequest bootstrapRequest;
    bootstrapRequest.corpusRoot = root / "corpus";
    bootstrapRequest.sourcePdfPath = localPdf;
    bootstrapRequest.workFolder = root / "work";
    bootstrapRequest.volumeId = "bootstrap-volume";
    bootstrapRequest.title = "Bootstrap Title";
    bootstrapRequest.subtitle = "";
    bootstrapRequest.sortTitle = "";
    bootstrapRequest.group = "";
    bootstrapRequest.generation.extractEmbedded = true;
    bootstrapRequest.generation.extractOcr = fullOcr;
    bootstrapRequest.runCandidateExtraction = true;

    const auto bootstrapOutcome = pte::core::VolumeBootstrapService().run(bootstrapRequest);
    require(bootstrapOutcome.success, "bootstrap succeeds");
    require(bootstrapOutcome.inventoryRecorded, "bootstrap inventory phase");
    require(bootstrapOutcome.workFolderInitialized, "bootstrap init phase");
    require(bootstrapOutcome.extractionCompleted, "bootstrap extraction phase");

    const auto digestAfterBootstrap = pte::core::sha256HexFile(localPdf);
    require(digestAfterBootstrap.has_value() && *digestBeforeBootstrap == *digestAfterBootstrap,
            "TC-NFR-003: source PDF hash unchanged after volume bootstrap with extraction");

    require(bootstrapOutcome.pageCount.has_value() && *bootstrapOutcome.pageCount == 1,
            "bootstrap page count one");

    const auto volumeJson = readFile(root / "work" / "volume.json");
    require(volumeJson.has_value() && volumeJson->find("bootstrap-volume") != std::string::npos,
            "volume.json written");

    const auto pq = readFile(root / "work" / "page-quality.json");
    require(pq.has_value() && pq->find("pdftotext") != std::string::npos,
            "bootstrap refreshed page-quality");
}

/** @brief Bootstrap without extraction leaves initializer diagnostics only. */
void testVolumeBootstrapInitWithoutExtraction() {
#ifdef PTE_TEST_FIXTURE_ROOT
    const fs::path pdf = fs::path(PTE_TEST_FIXTURE_ROOT) / "hello.pdf";
#else
    const fs::path pdf;
#endif
    if (pdf.empty() || !fs::exists(pdf)) {
        return;
    }

    const auto root = makeTempRoot() / "bootstrap-init-only";
    fs::create_directories(root / "corpus");
    const fs::path localPdf = root / "corpus" / "hello.pdf";
    fs::copy_file(pdf, localPdf);

    pte::core::VolumeBootstrapRequest bootstrapRequest;
    bootstrapRequest.corpusRoot = root / "corpus";
    bootstrapRequest.sourcePdfPath = localPdf;
    bootstrapRequest.workFolder = root / "work";
    bootstrapRequest.volumeId = "init-only-volume";
    bootstrapRequest.title = "Init Only";
    bootstrapRequest.subtitle = "";
    bootstrapRequest.sortTitle = "";
    bootstrapRequest.group = "";
    bootstrapRequest.runCandidateExtraction = false;

    const auto outcome = pte::core::VolumeBootstrapService().run(bootstrapRequest);
    require(outcome.success, "init-only bootstrap succeeds");
    require(outcome.workFolderInitialized, "work folder initialized");
    require(!outcome.extractionCompleted, "extraction skipped");

    const auto pq = readFile(root / "work" / "page-quality.json");
    require(pq.has_value() && pq->find("not-run") != std::string::npos,
            "placeholder diagnostics before extraction");
}
#endif

/** @brief Verifies volume metadata serialization preserves required and optional fields. */
void testVolumeMetadataWrite() {
    const auto root = makeTempRoot() / "volume-metadata-write";
    const auto metadata = syntheticVolumeMetadata();

    const auto result = pte::core::VolumeMetadataService().writeVolumeMetadata(root, metadata);
    const auto written = readFile(root / "volume.json");

    require(result.success, "volume metadata write succeeds");
    require(written && written->find("\"volumeId\": \"synthetic-volume\"") != std::string::npos,
            "volume metadata writes stable volume ID");
    require(written && written->find("\"productCode\": \"SYN-001\"") != std::string::npos,
            "volume metadata preserves optional bibliographic field");
    require(written && written->find("\"0002\"") != std::string::npos,
            "volume metadata writes page map entries");
}

/** @brief Verifies printed page labels update independently from PDF page number. */
void testVolumeMetadataPrintedPageLabel() {
    const auto metadata = syntheticVolumeMetadata();
    const auto result = pte::core::VolumeMetadataService().setPrintedPageLabel(
        metadata,
        "0002",
        "A-1");

    require(result.success, "printed page label update succeeds");
    require(result.metadata.pageMap[1].pdfPage == 2, "printed label preserves PDF page");
    require(result.metadata.pageMap[1].printedPageLabel == "A-1",
            "printed label is updated");
}

/** @brief Verifies cover metadata can be set to an explicit page. */
void testVolumeMetadataCoverPage() {
    const auto metadata = syntheticVolumeMetadata();
    const auto result = pte::core::VolumeMetadataService().setCoverPage(metadata, "0002");

    require(result.success, "cover page update succeeds");
    require(result.metadata.cover.pageId && *result.metadata.cover.pageId == "0002",
            "cover page ID is stored");
    require(result.metadata.cover.pdfPage && *result.metadata.cover.pdfPage == 2,
            "cover PDF page is stored");
}

/** @brief Verifies invalid metadata edits fail safely and do not write artifacts. */
void testVolumeMetadataRejectsInvalidPageMap() {
    const auto root = makeTempRoot() / "volume-metadata-invalid";
    auto metadata = syntheticVolumeMetadata();
    metadata.pageMap[0].pageId = "1";

    const auto result = pte::core::VolumeMetadataService().writeVolumeMetadata(root, metadata);

    require(!result.success, "invalid page map metadata write fails");
    require(!fs::exists(root / "volume.json"), "invalid metadata does not write volume.json");
}

/** @brief Verifies output root configuration accepts local filesystem paths. */
void testOutputConfigAcceptsLocalPath() {
    const auto root = makeTempRoot() / "output-config";
    fs::create_directories(root);
    const pte::core::AppConfiguration current{{}, {}, {}};
    const auto result = pte::core::OutputConfigService().setOutputRoot(current, root);

    require(result.success, "local output root is accepted");
    require(result.configuration.outputRoot == root, "output root is stored");
    require(pte::core::artifactRetentionPolicyName(
                pte::core::ArtifactRetentionPolicy::RetainRawCandidates)
                == "retain-raw-candidates",
            "retention policy name is stable");
}

/** @brief Verifies output root configuration rejects URI-like paths. */
void testOutputConfigRejectsUriPath() {
    const pte::core::AppConfiguration current{{}, {}, {}};
    const auto result = pte::core::OutputConfigService().setOutputRoot(
        current,
        "https://example.invalid/output");

    require(!result.success, "URI output root is rejected");
    require(!result.findings.empty(), "URI output root has finding");
    require(result.findings[0].code == "non-filesystem-output-root",
            "URI output root finding code is stable");
}

/** @brief Verifies NFR-001: common remote-style scheme strings are not accepted as output roots. */
void testTnfr001OutputConfigRejectsHostedSchemePaths() {
    const pte::core::AppConfiguration current{{}, {}, {}};
    const char* uris[] = {
        "http://example.invalid/out",
        "https://example.invalid/out",
        "s3://bucket/key/out",
        "ftp://host.invalid/pub/out",
    };
    for (const char* uri : uris) {
        const auto result = pte::core::OutputConfigService().setOutputRoot(current, uri);
        require(!result.success, "TC-NFR-001: hosted URI output root rejected");
        require(!result.findings.empty(), "TC-NFR-001: finding recorded");
        require(result.findings[0].code == "non-filesystem-output-root",
                "TC-NFR-001: stable finding code");
    }
}

/** @brief Verifies local_path_intent distinguishes local paths and hosted schemes. */
void testTnfr001LocalPathIntentHeuristic() {
    using pte::core::pathTextLooksLikeHostedUriScheme;
    require(pathTextLooksLikeHostedUriScheme("https://a/b"),
            "TC-NFR-001: https is hosted scheme");
    require(pathTextLooksLikeHostedUriScheme("s3://x/y"),
            "TC-NFR-001: s3 is hosted scheme");
    require(!pathTextLooksLikeHostedUriScheme("/tmp/plain.pdf"),
            "TC-NFR-001: POSIX path not hosted");
    require(!pathTextLooksLikeHostedUriScheme(R"(C:\Temp\doc.pdf)"),
            "TC-NFR-001: Windows path not hosted");
    require(!pathTextLooksLikeHostedUriScheme("file:///tmp/plain.pdf"),
            "TC-NFR-001: file URL treated as local intent");
}

/** @brief Verifies bootstrap rejects hosted URIs before inventory (NFR-001). */
void testTnfr001BootstrapRejectsHostedSourceUri() {
    pte::core::VolumeBootstrapRequest request;
    request.corpusRoot = "/tmp";
    request.sourcePdfPath = "https://example.invalid/document.pdf";
    request.workFolder = "/tmp/pte-nfr001-work";
    request.volumeId = "vol";
    request.title = "Title";

    const auto outcome = pte::core::VolumeBootstrapService().run(request);
    require(!outcome.success, "TC-NFR-001: bootstrap rejects https source");
    require(outcome.safeMessage.find("local filesystem") != std::string::npos,
            "TC-NFR-001: bootstrap message explains local path requirement");
}

/** @brief Verifies backup dry-run reports protected artifacts without writing. */
void testWorkFolderBackupDryRun() {
    const auto root = makeTempRoot() / "backup-dry-run";
    writeValidWorkFolder(root);

    const pte::core::WorkFolderBackupRequest request{root, "before-regeneration", true};
    const auto report = pte::core::WorkFolderBackupService().createBackup(request);

    require(report.success, "backup dry-run succeeds");
    require(report.dryRun, "backup report is dry-run");
    require(!report.plannedArtifacts.empty(), "backup dry-run reports protected artifacts");
    require(!fs::exists(root / "backups" / "before-regeneration"),
            "backup dry-run does not write backup folder");
}

/** @brief Verifies backup creation copies reviewed text and metadata but not raw candidates. */
void testWorkFolderBackupCreatesProtectedCopy() {
    const auto root = makeTempRoot() / "backup-create";
    writeValidWorkFolder(root);
    writeText(root / "raw" / "ocr" / "0001.txt", "synthetic raw ocr");

    const pte::core::WorkFolderBackupRequest request{root, "before-regeneration", false};
    const auto report = pte::core::WorkFolderBackupService().createBackup(request);

    require(report.success, "backup creation succeeds");
    require(fs::exists(report.backupFolder / "volume.json"), "backup includes volume metadata");
    require(fs::exists(report.backupFolder / "pages" / "0001.txt"),
            "backup includes reviewed page text");
    require(!fs::exists(report.backupFolder / "raw" / "ocr" / "0001.txt"),
            "backup does not copy raw candidates in protected backup slice");
}

/** @brief Verifies backup restore requires confirmation before overwriting artifacts. */
void testWorkFolderRestoreRequiresConfirmation() {
    const auto root = makeTempRoot() / "backup-confirm";
    writeValidWorkFolder(root);
    const pte::core::WorkFolderBackupRequest backupRequest{root, "saved", false};
    const auto backup = pte::core::WorkFolderBackupService().createBackup(backupRequest);

    writeText(root / "pages" / "0001.txt", "changed reviewed text");
    const pte::core::WorkFolderRestoreRequest restoreRequest{
        root,
        backup.backupFolder,
        false,
        false
    };
    const auto restore = pte::core::WorkFolderBackupService().restoreBackup(restoreRequest);
    const auto current = readFile(root / "pages" / "0001.txt");

    require(!restore.success, "restore without confirmation fails");
    require(current && *current == "changed reviewed text",
            "unconfirmed restore preserves current reviewed text");
}

/** @brief Verifies confirmed backup restore replaces protected artifacts. */
void testWorkFolderRestoreConfirmed() {
    const auto root = makeTempRoot() / "backup-restore";
    writeValidWorkFolder(root);
    const pte::core::WorkFolderBackupRequest backupRequest{root, "saved", false};
    const auto backup = pte::core::WorkFolderBackupService().createBackup(backupRequest);

    writeText(root / "pages" / "0001.txt", "changed reviewed text");
    const pte::core::WorkFolderRestoreRequest restoreRequest{
        root,
        backup.backupFolder,
        true,
        false
    };
    const auto restore = pte::core::WorkFolderBackupService().restoreBackup(restoreRequest);
    const auto current = readFile(root / "pages" / "0001.txt");

    require(restore.success, "confirmed restore succeeds");
    require(current && *current == "Synthetic searchable page text.\n",
            "confirmed restore restores reviewed page text");
}

/** @brief Verifies raw candidate cleanup dry-run reports removals without deleting. */
void testArtifactCleanupDryRunRawCandidates() {
    const auto root = makeTempRoot() / "cleanup-dry-run";
    writeValidWorkFolder(root);
    writeText(root / "raw" / "embedded" / "0001.txt", "synthetic embedded");

    const pte::core::CleanupRequest request{
        root,
        {pte::core::CleanupScope::RawCandidates},
        false,
        false,
        true
    };
    const auto report = pte::core::ArtifactCleanupService().cleanup(request);

    require(report.success, "cleanup dry-run succeeds");
    require(!report.actions.empty(), "cleanup dry-run reports planned raw cleanup");
    require(fs::exists(root / "raw" / "embedded" / "0001.txt"),
            "cleanup dry-run does not remove raw candidate");
}

/** @brief Verifies confirmed cleanup removes raw candidates. */
void testArtifactCleanupConfirmedRawCandidates() {
    const auto root = makeTempRoot() / "cleanup-confirmed";
    writeValidWorkFolder(root);
    writeText(root / "raw" / "ocr" / "0001.txt", "synthetic ocr");

    const pte::core::CleanupRequest request{
        root,
        {pte::core::CleanupScope::RawCandidates},
        true,
        false,
        false
    };
    const auto report = pte::core::ArtifactCleanupService().cleanup(request);

    require(report.success, "confirmed cleanup succeeds");
    require(!fs::exists(root / "raw" / "ocr"), "confirmed cleanup removes raw OCR folder");
}

/** @brief Verifies reviewed text cleanup requires explicit authorization. */
void testArtifactCleanupProtectsReviewedText() {
    const auto root = makeTempRoot() / "cleanup-reviewed";
    writeValidWorkFolder(root);

    const pte::core::CleanupRequest request{
        root,
        {pte::core::CleanupScope::ReviewedText},
        true,
        false,
        false
    };
    const auto report = pte::core::ArtifactCleanupService().cleanup(request);

    require(!report.success, "reviewed text cleanup without authorization fails");
    require(report.confirmationRequired, "reviewed text cleanup requires confirmation");
    require(fs::exists(root / "pages" / "0001.txt"),
            "reviewed text cleanup protection preserves page file");
}

/** @brief Verifies that a complete synthetic work folder validates cleanly. */
void testValidWorkFolder() {
    const auto root = makeTempRoot() / "valid";
    writeValidWorkFolder(root);
    const auto report = pte::core::WorkFolderValidator().validate(root);
    require(report.ok(), "valid work folder has no validation errors");
}

/** @brief Verifies that missing reviewed page text files are reported as validation failures. */
void testMissingPageFile() {
    const auto root = makeTempRoot() / "missing-page";
    writeValidWorkFolder(root);
    fs::remove(root / "pages" / "0002.txt");
    const auto report = pte::core::WorkFolderValidator().validate(root);
    require(!report.ok(), "missing page file is validation failure");
    require(hasCode(report, "missing-page-file"), "missing page file finding code is present");
}

/** @brief Verifies that candidate-source strings cannot masquerade as review statuses. */
void testInvalidReviewStatus() {
    const auto root = makeTempRoot() / "bad-status";
    writeValidWorkFolder(root);
    writeText(root / "review-state.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "pages": {
    "0001": {
      "pdfPage": 1,
      "status": "accepted",
      "reviewedAt": null,
      "reviewedBy": null,
      "notes": ""
    },
    "0002": { "pdfPage": 2, "status": "ocr", "reviewedAt": null, "reviewedBy": null, "notes": "" }
  }
})JSON");
    const auto report = pte::core::WorkFolderValidator().validate(root);
    require(!report.ok(), "invalid review status is validation failure");
    require(hasCode(report, "invalid-review-status"),
            "invalid review status finding code is present");
}

/** @brief Verifies that invalid JSON diagnostics remain safe and omit substantive page text. */
void testInvalidJsonIsSafe() {
    const auto root = makeTempRoot() / "invalid-json";
    writeValidWorkFolder(root);
    writeText(root / "page-quality.json", "{ not-json: true");
    const auto report = pte::core::WorkFolderValidator().validate(root);
    require(!report.ok(), "invalid JSON is validation failure");
    require(hasCode(report, "invalid-json"), "invalid JSON finding code is present");
    for (const auto& finding : report.findings) {
        require(finding.safeMessage.find("Synthetic searchable page text") == std::string::npos,
                "safe message does not include page text");
    }
}

/** @brief Verifies that initialization creates the approved local artifact layout. */
void testWorkFolderInitializationCreatesValidLayout() {
    const auto root = makeTempRoot() / "initialized";
    const pte::core::WorkFolderInitRequest request{
        root,
        "synthetic-volume",
        "Synthetic Volume",
        "",
        "Synthetic Volume",
        "",
        {root / "source.pdf", "source.pdf", 3}
    };

    const auto result = pte::core::WorkFolderInitializer().initialize(request);
    require(result.success, "work folder initialization succeeds");
    require(fs::exists(root / "volume.json"), "volume.json is created");
    require(fs::exists(root / "page-quality.json"), "page-quality.json is created");
    require(fs::exists(root / "review-state.json"), "review-state.json is created");
    require(fs::is_directory(root / "raw" / "embedded"), "raw embedded directory is created");
    require(fs::is_directory(root / "raw" / "ocr"), "raw OCR directory is created");
    require(fs::exists(root / "pages" / "0001.txt"), "page 1 text file is created");
    require(fs::exists(root / "pages" / "0002.txt"), "page 2 text file is created");
    require(fs::exists(root / "pages" / "0003.txt"), "page 3 text file is created");

    const auto report = pte::core::WorkFolderValidator().validate(root);
    require(report.ok(), "initialized work folder validates cleanly");
}

/** @brief Verifies that initialization writes default raw review state entries. */
void testWorkFolderInitializationReviewDefaults() {
    const auto root = makeTempRoot() / "init-review-defaults";
    const pte::core::WorkFolderInitRequest request{
        root,
        "synthetic-volume",
        "Synthetic Volume",
        "",
        "Synthetic Volume",
        "",
        {root / "source.pdf", "source.pdf", 1}
    };

    const auto result = pte::core::WorkFolderInitializer().initialize(request);
    require(result.success, "work folder initialization for review defaults succeeds");

    const auto reviewState = readFile(root / "review-state.json");
    require(reviewState.has_value(), "review-state.json can be read");
    require(reviewState && reviewState->find("\"status\": \"raw\"") != std::string::npos,
            "initialized review state is raw");
    require(reviewState && reviewState->find("\"selectedSource\": \"empty\"") != std::string::npos,
            "initialized selected source is empty");
    require(reviewState && reviewState->find("\"dirtyState\": \"clean\"") != std::string::npos,
            "initialized dirty state is clean");
}

/** @brief Verifies initialization rejects invalid source page counts. */
void testWorkFolderInitializationRejectsInvalidPageCount() {
    const auto root = makeTempRoot() / "bad-init";
    const pte::core::WorkFolderInitRequest request{
        root,
        "synthetic-volume",
        "Synthetic Volume",
        "",
        "Synthetic Volume",
        "",
        {root / "source.pdf", "source.pdf", 0}
    };

    const auto result = pte::core::WorkFolderInitializer().initialize(request);
    require(!result.success, "initialization rejects non-positive page count");
    require(!fs::exists(root / "volume.json"),
            "rejected initialization does not write volume.json");
}

/** @brief Verifies initialization does not modify the source PDF path supplied as metadata. */
void testWorkFolderInitializationDoesNotModifySourcePdf() {
    const auto root = makeTempRoot() / "source-safe";
    const auto source = root / "source.pdf";
    writeText(source, "synthetic source bytes");

    const pte::core::WorkFolderInitRequest request{
        root / "work",
        "synthetic-volume",
        "Synthetic Volume",
        "",
        "Synthetic Volume",
        "",
        {source, "source.pdf", 1}
    };

    const auto before = readFile(source);
    const auto result = pte::core::WorkFolderInitializer().initialize(request);
    const auto after = readFile(source);
    require(result.success, "source-safe initialization succeeds");
    require(before == after, "initialization does not modify source PDF bytes");
}

/** @brief Verifies dependency capability detection finds a configured executable path. */
void testDependencyCapabilityConfiguredPath() {
    const auto root = makeTempRoot() / "dependency-configured";
    const auto tool = root / "fake-poppler-tool";
    writeText(tool, "synthetic executable");
    makeExecutable(tool);

    const pte::core::ToolCheckRequest request{
        "poppler-pdftotext",
        "pdftotext",
        tool,
        true,
        "synthetic-1.0",
        {"embedded-text"}
    };

    const auto capability = pte::core::DependencyCapabilityService().checkTool(request, "");
    require(capability.detected, "configured dependency path is detected");
    require(capability.path == tool, "configured dependency path is reported");
    require(capability.version == "synthetic-1.0", "caller-supplied version is preserved");
    require(capability.safeMessage.find("available") != std::string::npos,
            "available dependency has safe message");
}

/** @brief Verifies dependency capability detection can search a supplied PATH value. */
void testDependencyCapabilitySearchPath() {
    const auto root = makeTempRoot() / "dependency-search";
    const auto tool = root / "fake-tesseract";
    writeText(tool, "synthetic executable");
    makeExecutable(tool);

    const pte::core::ToolCheckRequest request{
        "tesseract",
        "fake-tesseract",
        {},
        true,
        "",
        {"ocr"}
    };

    const auto capability = pte::core::DependencyCapabilityService().checkTool(
        request,
        root.string());
    require(capability.detected, "dependency is found on supplied search path");
    require(capability.path == tool, "search path dependency reports discovered path");
}

/** @brief Verifies missing required dependencies make aggregate capability reports fail. */
void testDependencyCapabilityMissingRequired() {
    const pte::core::ToolCheckRequest request{
        "missing-required-tool",
        "definitely-not-present",
        {},
        true,
        "",
        {}
    };

    const auto report = pte::core::DependencyCapabilityService().detectCapabilities(
        {request},
        "");
    require(!report.ok(), "missing required dependency makes report not OK");
    require(report.missingRequiredCount() == 1, "missing required dependency is counted");
    require(!report.tools.empty(), "missing dependency result is present");
    require(report.tools[0].safeMessage.find("required tool") != std::string::npos,
            "missing required dependency has actionable safe message");
}

/** @brief Verifies unchanged files can be saved without stale-write conflict handling. */
void testFileChangeDetectorUnchanged() {
    const auto root = makeTempRoot() / "file-change-unchanged";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "synthetic reviewed text");

    const pte::core::FileChangeDetector detector;
    const auto snapshot = detector.snapshot(file);
    const auto result = detector.compare(file, snapshot);

    require(snapshot.exists, "file snapshot exists");
    require(result.status == pte::core::FileChangeStatus::Unchanged,
            "unchanged file reports unchanged status");
    require(result.canSaveWithoutConflict(), "unchanged file can save without conflict");
}

/** @brief Verifies changed files are detected before overwrite. */
void testFileChangeDetectorChanged() {
    const auto root = makeTempRoot() / "file-change-changed";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "synthetic reviewed text");

    const pte::core::FileChangeDetector detector;
    const auto snapshot = detector.snapshot(file);
    writeText(file, "synthetic reviewed text changed outside app");
    const auto result = detector.compare(file, snapshot);

    require(result.status == pte::core::FileChangeStatus::Changed,
            "changed file reports changed status");
    require(!result.canSaveWithoutConflict(), "changed file requires conflict handling");
    require(result.safeMessage.find("changed") != std::string::npos,
            "changed file result has safe message");
}

/** @brief Verifies missing files are detected before overwrite. */
void testFileChangeDetectorMissing() {
    const auto root = makeTempRoot() / "file-change-missing";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "synthetic reviewed text");

    const pte::core::FileChangeDetector detector;
    const auto snapshot = detector.snapshot(file);
    fs::remove(file);
    const auto result = detector.compare(file, snapshot);

    require(result.status == pte::core::FileChangeStatus::Missing,
            "missing file reports missing status");
    require(!result.canSaveWithoutConflict(), "missing file requires conflict handling");
}

/** @brief Verifies change detector messages do not include file contents. */
void testFileChangeDetectorSafeMessages() {
    const auto root = makeTempRoot() / "file-change-safe";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "synthetic secret page text");

    const pte::core::FileChangeDetector detector;
    const auto snapshot = detector.snapshot(file);
    writeText(file, "synthetic secret page text modified");
    const auto result = detector.compare(file, snapshot);

    require(result.safeMessage.find("synthetic secret page text") == std::string::npos,
            "file change safe message does not include file contents");
    require(pte::core::fileChangeStatusName(result.status) == "changed",
            "file change status name is stable");
}

/** @brief Verifies reviewed page text loads with a stale-write snapshot. */
void testReviewedPageTextLoad() {
    const auto root = makeTempRoot() / "reviewed-load";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "Synthetic reviewed text.\n");

    const auto result = pte::core::ReviewedPageTextService().load(file);
    require(result.success, "reviewed page text load succeeds");
    require(result.text == "Synthetic reviewed text.\n", "reviewed page text content loads");
    require(result.snapshot.exists, "reviewed page text load captures snapshot");
}

/** @brief Verifies reviewed page text saves plain text without metadata front matter. */
void testReviewedPageTextSavePlainText() {
    const auto root = makeTempRoot() / "reviewed-save";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "old text");

    const auto load = pte::core::ReviewedPageTextService().load(file);
    const pte::core::ReviewedPageSavePolicy policy{load.snapshot, false};
    const auto save = pte::core::ReviewedPageTextService().save(file, "new plain text", policy);
    const auto saved = readFile(file);

    require(save.success, "reviewed page text save succeeds");
    require(saved && *saved == "new plain text", "reviewed page text saves exact plain text");
    require(saved && saved->find("schemaVersion") == std::string::npos,
            "reviewed page text save adds no metadata front matter");
}

/** @brief Verifies reviewed page text save rejects external changes by default. */
void testReviewedPageTextSaveRejectsConflict() {
    const auto root = makeTempRoot() / "reviewed-conflict";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "loaded text");

    const auto load = pte::core::ReviewedPageTextService().load(file);
    writeText(file, "external edit");
    const pte::core::ReviewedPageSavePolicy policy{load.snapshot, false};
    const auto save = pte::core::ReviewedPageTextService().save(file, "new edit", policy);
    const auto current = readFile(file);

    require(!save.success, "reviewed page text conflicting save fails");
    require(save.conflict, "reviewed page text conflicting save reports conflict");
    require(current && *current == "external edit", "conflicting save preserves external edit");
}

/** @brief Verifies explicit overwrite policy can resolve a reviewed text conflict. */
void testReviewedPageTextSaveOverwriteConflict() {
    const auto root = makeTempRoot() / "reviewed-overwrite";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "loaded text");

    const auto load = pte::core::ReviewedPageTextService().load(file);
    writeText(file, "external edit");
    const pte::core::ReviewedPageSavePolicy policy{load.snapshot, true};
    const auto save = pte::core::ReviewedPageTextService().save(file, "explicit overwrite", policy);
    const auto current = readFile(file);

    require(save.success, "reviewed page text explicit overwrite succeeds");
    require(current && *current == "explicit overwrite",
            "explicit overwrite writes requested text");
}

/** @brief Verifies reviewed page text save conflict messages omit page contents. */
void testReviewedPageTextSafeMessages() {
    const auto root = makeTempRoot() / "reviewed-safe";
    const auto file = root / "pages" / "0001.txt";
    writeText(file, "secret local page text");

    const auto load = pte::core::ReviewedPageTextService().load(file);
    writeText(file, "secret local page text externally modified");
    const pte::core::ReviewedPageSavePolicy policy{load.snapshot, false};
    const auto save = pte::core::ReviewedPageTextService().save(file, "replacement", policy);

    require(save.safeMessage.find("secret local page text") == std::string::npos,
            "reviewed page text save message omits page contents");
}

/** @brief Verifies text normalization produces plain line endings and removes controls. */
void testTextNormalizationPlainTextCleanup() {
    const pte::core::NormalizationProfile profile{};
    const auto result = pte::core::TextNormalizationService().normalize(
        "alpha\r\nbeta\rcontrol\x01  \n",
        profile);

    require(result.changed, "text normalization reports changed text");
    require(result.text == "alpha\nbeta\ncontrol\n", "text normalization cleans text");
    require(!result.flags.empty(), "text normalization records flags");
}

/** @brief Verifies text normalization reports safe quality flags. */
void testTextNormalizationQualityFlags() {
    const pte::core::NormalizationProfile profile{};
    const auto result = pte::core::TextNormalizationService().normalize("1234567890!!", profile);

    require(!result.flags.empty(), "text normalization records quality flags");
    require(result.safeMessage.find("1234567890") == std::string::npos,
            "text normalization safe message omits input text");
}

/** @brief Verifies raw candidate storage leaves reviewed page text untouched. */
void testCandidateStorageIsRawOnly() {
    const auto root = makeTempRoot() / "candidate-raw-only";
    writeValidWorkFolder(root);

    const auto stored = pte::core::CandidateTextService().storeCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Embedded,
        "synthetic embedded candidate");
    const auto reviewed = readFile(root / "pages" / "0001.txt");
    const auto raw = readFile(root / "raw" / "embedded" / "0001.txt");

    require(stored.success, "embedded candidate storage succeeds");
    require(stored.charCount == 28, "embedded candidate count is reported");
    require(raw && *raw == "synthetic embedded candidate", "embedded candidate is stored raw");
    require(reviewed && *reviewed == "Synthetic searchable page text.\n",
            "candidate storage does not modify reviewed page text");
}

/** @brief Verifies OCR candidate storage and loading use separate raw artifacts. */
void testCandidateOcrLoadSeparateArtifact() {
    const auto root = makeTempRoot() / "candidate-ocr";
    writeValidWorkFolder(root);

    const auto stored = pte::core::CandidateTextService().storeCandidate(
        root,
        "0002",
        pte::core::CandidateSource::Ocr,
        "synthetic ocr candidate");
    const auto loaded = pte::core::CandidateTextService().loadCandidate(
        root,
        "0002",
        pte::core::CandidateSource::Ocr);

    require(stored.success, "OCR candidate storage succeeds");
    require(loaded.success, "OCR candidate load succeeds");
    require(loaded.text == "synthetic ocr candidate", "OCR candidate loads exact local text");
    require(!fs::exists(root / "raw" / "embedded" / "0002.txt"),
            "OCR candidate does not create embedded candidate artifact");
}

/** @brief Verifies candidate comparison reports counts and suspicious deltas without text. */
void testCandidateComparisonCountsAndFlags() {
    const auto root = makeTempRoot() / "candidate-comparison";
    writeValidWorkFolder(root);
    pte::core::CandidateTextService().storeCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Embedded,
        "short");
    pte::core::CandidateTextService().storeCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Ocr,
        "synthetic OCR candidate with more characters");

    const auto comparison = pte::core::CandidateTextService().comparePage(root, "0001");

    require(comparison.success, "candidate comparison succeeds");
    require(comparison.reviewedCharCount == 32, "comparison counts reviewed text");
    require(comparison.embedded.available, "comparison reports embedded availability");
    require(comparison.ocr.available, "comparison reports OCR availability");
    require(!comparison.flags.empty(), "comparison flags large embedded/OCR delta");
    require(comparison.flags[0] == "embedded-ocr-delta-suspicious",
            "comparison uses stable delta flag");
    require(comparison.safeMessage.find("synthetic OCR candidate") == std::string::npos,
            "comparison safe message omits candidate text");
}

/** @brief Verifies restore from candidate is explicit and leaves review status unchanged. */
void testCandidateRestoreExplicitOnly() {
    const auto root = makeTempRoot() / "candidate-restore";
    writeValidWorkFolder(root);
    pte::core::CandidateTextService().storeCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Embedded,
        "restored synthetic candidate");

    const auto load = pte::core::ReviewedPageTextService().load(root / "pages" / "0001.txt");
    const pte::core::CandidateRestorePolicy policy{load.snapshot, false};
    const auto restored = pte::core::CandidateTextService().restoreReviewedTextFromCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Embedded,
        policy);
    const auto reviewed = readFile(root / "pages" / "0001.txt");
    const auto reviewState = readFile(root / "review-state.json");

    require(restored.success, "explicit candidate restore succeeds");
    require(reviewed && *reviewed == "restored synthetic candidate",
            "restore writes candidate text into reviewed page text");
    require(reviewState && reviewState->find("\"status\": \"accepted\"") != std::string::npos,
            "restore does not change review status metadata");
}

/** @brief Verifies restore from candidate honors stale-write conflict detection. */
void testCandidateRestoreRejectsConflict() {
    const auto root = makeTempRoot() / "candidate-restore-conflict";
    writeValidWorkFolder(root);
    pte::core::CandidateTextService().storeCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Ocr,
        "candidate replacement");

    const auto load = pte::core::ReviewedPageTextService().load(root / "pages" / "0001.txt");
    writeText(root / "pages" / "0001.txt", "external reviewed edit");
    const pte::core::CandidateRestorePolicy policy{load.snapshot, false};
    const auto restored = pte::core::CandidateTextService().restoreReviewedTextFromCandidate(
        root,
        "0001",
        pte::core::CandidateSource::Ocr,
        policy);
    const auto reviewed = readFile(root / "pages" / "0001.txt");

    require(!restored.success, "conflicting candidate restore fails");
    require(restored.conflict, "conflicting candidate restore reports conflict");
    require(reviewed && *reviewed == "external reviewed edit",
            "conflicting restore preserves external reviewed edit");
}

/** @brief Verifies Paper Capture routing schedules OCR for every page. */
void testOcrRoutingPaperCaptureAllPages() {
    const pte::core::OcrRoutingDocumentProfile profile{true, true};
    const std::vector<pte::core::OcrRoutingPageSignal> pages{
        {"0001", 1, true, 200},
        {"0002", 2, true, 300}
    };

    const auto plan = pte::core::OcrRoutingService().plan(profile, pages);

    require(plan.success, "Paper Capture OCR routing succeeds");
    require(plan.reviewGated, "OCR routing remains review-gated");
    require(plan.decisions.size() == 2, "Paper Capture routes every page");
    require(plan.decisions[0].scheduleOcr, "Paper Capture schedules page 1");
    require(plan.decisions[1].reason == "paper-capture-default",
            "Paper Capture routing reason is stable");
}

/** @brief Verifies OCR routing schedules missing or weak embedded candidates. */
void testOcrRoutingWeakEmbeddedText() {
    const pte::core::OcrRoutingDocumentProfile profile{false, true};
    const std::vector<pte::core::OcrRoutingPageSignal> pages{
        {"0001", 1, false, 0},
        {"0002", 2, true, 5},
        {"0003", 3, true, 200}
    };

    const auto plan = pte::core::OcrRoutingService().plan(profile, pages);

    require(plan.success, "standard OCR routing succeeds");
    require(plan.decisions[0].scheduleOcr, "missing embedded text schedules OCR");
    require(plan.decisions[1].scheduleOcr, "very short embedded text schedules OCR");
    require(!plan.decisions[2].scheduleOcr, "sufficient embedded text does not schedule OCR");
}

/** @brief Verifies unavailable OCR tool disables routing with safe reasons. */
void testOcrRoutingToolUnavailable() {
    const pte::core::OcrRoutingDocumentProfile profile{true, false};
    const std::vector<pte::core::OcrRoutingPageSignal> pages{{"0001", 1, false, 0}};

    const auto plan = pte::core::OcrRoutingService().plan(profile, pages);

    require(plan.success, "OCR routing succeeds with unavailable tool");
    require(!plan.decisions[0].scheduleOcr, "unavailable OCR tool does not schedule OCR");
    require(plan.decisions[0].reason == "ocr-tool-unavailable",
            "unavailable OCR tool reason is stable");
}

/** @brief Verifies page-quality diagnostics include candidate counts, flags, and tool data. */
void testPageQualityWritesCandidateDiagnostics() {
    const auto root = makeTempRoot() / "page-quality-write";
    const std::vector<pte::core::PageQualityEntry> entries{
        {
            "0001",
            1,
            "ocr",
            {true, 20, "pdftotext synthetic", "embedded"},
            {true, 40, "tesseract synthetic", "psm-auto"},
            {"ocr-used", "embedded-ocr-delta-suspicious"},
            {}
        }
    };

    const auto result = pte::core::PageQualityService().writePageQuality(
        root,
        "synthetic-volume",
        entries);
    const auto written = readFile(root / "page-quality.json");

    require(result.success, "page-quality write succeeds");
    require(written && written->find("\"charCount\": 40") != std::string::npos,
            "page-quality writes OCR character count");
    require(written && written->find("tesseract synthetic") != std::string::npos,
            "page-quality writes tool metadata");
    require(written && written->find("embedded-ocr-delta-suspicious") != std::string::npos,
            "page-quality writes diagnostic flag");
}

/** @brief Verifies page-quality diagnostics reject invalid page identities safely. */
void testPageQualityRejectsInvalidPageId() {
    const auto root = makeTempRoot() / "page-quality-invalid";
    const std::vector<pte::core::PageQualityEntry> entries{
        {"1", 1, "empty", {}, {}, {}, {}}
    };

    const auto result = pte::core::PageQualityService().writePageQuality(
        root,
        "synthetic-volume",
        entries);

    require(!result.success, "page-quality rejects invalid page id");
    require(!fs::exists(root / "page-quality.json"),
            "page-quality invalid input does not write artifact");
}

/** @brief Verifies readiness summary counts review statuses and selected sources. */
void testReadinessSummaryCounts() {
    const auto root = makeTempRoot() / "readiness-counts";
    writeValidWorkFolder(root);
    writeText(root / "review-state.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "pages": {
    "0001": { "pdfPage": 1, "status": "accepted" },
    "0002": { "pdfPage": 2, "status": "raw" }
  }
})JSON");
    writeText(root / "page-quality.json", R"JSON({
  "schemaVersion": 1,
  "volumeId": "synthetic-volume",
  "pages": {
    "0001": {
      "pdfPage": 1,
      "selectedSource": "manual",
      "flags": ["ocr-used", "unicode-suspect"],
      "errors": []
    },
    "0002": {
      "pdfPage": 2,
      "selectedSource": "empty",
      "flags": ["empty"],
      "errors": []
    }
  }
})JSON");

    const auto summary = pte::core::ReadinessSummaryService().summarizeWorkFolder(root);
    require(summary.success, "readiness summary succeeds");
    require(summary.statusCounts.at("accepted") == 1, "readiness counts accepted pages");
    require(summary.statusCounts.at("raw") == 1, "readiness counts raw pages");
    require(summary.selectedSourceCounts.at("manual") == 1, "readiness counts manual source");
    require(summary.selectedSourceCounts.at("empty") == 1, "readiness counts empty source");
    require(summary.flagCounts.at("unicode-suspect") == 1, "readiness counts flags");
    require(summary.ocrUsedCount == 1, "readiness counts OCR-used pages");
}

/** @brief Verifies readiness summary includes validation error counts. */
void testReadinessSummaryValidationErrors() {
    const auto root = makeTempRoot() / "readiness-validation";
    writeValidWorkFolder(root);
    fs::remove(root / "pages" / "0002.txt");

    const auto summary = pte::core::ReadinessSummaryService().summarizeWorkFolder(root);
    require(summary.success, "readiness summary still succeeds with validation errors");
    require(summary.validationErrorCount > 0, "readiness includes validation error count");
}

/** @brief Verifies readiness summary safe messages omit page text. */
void testReadinessSummarySafeMessages() {
    const auto root = makeTempRoot() / "readiness-safe";
    writeValidWorkFolder(root);
    writeText(root / "page-quality.json", "{ not-json: true");

    const auto summary = pte::core::ReadinessSummaryService().summarizeWorkFolder(root);
    require(!summary.success, "readiness summary fails for invalid page-quality JSON");
    require(summary.safeMessage.find("Synthetic searchable page text") == std::string::npos,
            "readiness safe message omits page text");
}

/** @brief Verifies repair planning proposes safe actions for repairable findings. */
void testValidationRepairPlanRepairableFindings() {
    const auto root = makeTempRoot() / "repair-plan";
    writeValidWorkFolder(root);
    fs::remove(root / "pages" / "0002.txt");

    const auto report = pte::core::WorkFolderValidator().validate(root);
    const auto plan = pte::core::ValidationRepairPlanner().planRepair(report);

    require(plan.hasActions(), "repair plan has actions for repairable finding");
    require(plan.dryRunOnly, "repair plan is dry-run only");
    require(plan.actions[0].actionCode == "create-empty-page-file",
            "repair plan proposes empty page file creation");
    require(plan.actions[0].pageId == "0002", "repair plan preserves page ID metadata");
}

/** @brief Verifies repair planning counts non-repairable validation errors. */
void testValidationRepairPlanNonRepairableFindings() {
    const pte::core::ValidationReport report{
        {
            {
                pte::core::Severity::Error,
                "invalid-review-status",
                "review-state.json",
                "0002",
                "review status is not allowed",
                false
            }
        }
    };

    const auto plan = pte::core::ValidationRepairPlanner().planRepair(report);
    require(!plan.hasActions(), "non-repairable finding does not create repair action");
    require(plan.nonRepairableCount == 1, "non-repairable finding is counted");
}

/** @brief Verifies repair planning does not modify artifacts. */
void testValidationRepairPlanDoesNotWrite() {
    const auto root = makeTempRoot() / "repair-dry-run";
    writeValidWorkFolder(root);
    fs::remove(root / "pages" / "0002.txt");

    const auto report = pte::core::WorkFolderValidator().validate(root);
    const auto plan = pte::core::ValidationRepairPlanner().planRepair(report);

    require(plan.hasActions(), "dry-run repair plan has action");
    require(!fs::exists(root / "pages" / "0002.txt"),
            "dry-run repair planning does not create missing file");
}

/** @brief Verifies repair planning safe messages omit page text. */
void testValidationRepairPlanSafeMessages() {
    const auto root = makeTempRoot() / "repair-safe";
    writeValidWorkFolder(root);
    fs::remove(root / "pages" / "0002.txt");

    const auto report = pte::core::WorkFolderValidator().validate(root);
    const auto plan = pte::core::ValidationRepairPlanner().planRepair(report);

    require(plan.hasActions(), "safe repair plan has action");
    require(plan.actions[0].safeMessage.find("Synthetic searchable page text") == std::string::npos,
            "repair plan safe message omits page text");
}

} // namespace

/**
 * @brief Runs the lightweight core test suite.
 *
 * @return Zero when all assertions pass; non-zero otherwise.
 * Side effects: Writes test status to stdout/stderr and uses a temporary fixture directory.
 */
int main() {
    runTest("page IDs", testPageIds);
    runTest("review eligibility", testReviewEligibility);
    runTest("review transitions allowed", testReviewTransitionsAllowed);
    runTest("review transitions rejected", testReviewTransitionsRejected);
    runTest("release eligibility summary", testReleaseEligibilitySummary);
    runTest("review workflow transition", testReviewWorkflowTransition);
    runTest("review workflow rejects candidate status", testReviewWorkflowRejectsCandidateStatus);
    runTest("review workflow next filtered page", testReviewWorkflowNextFilteredPage);
    runTest("source inventory records identity", testSourceInventoryRecordsIdentity);
    runTest("source inventory missing path safe", testSourceInventoryMissingPathSafe);
    runTest("source inventory preserves source PDF", testSourceInventoryDoesNotModifySource);
    runTest("source inventory duplicate detection", testSourceInventoryDuplicateDetection);
    runTest("pdfinfo stdout parses pages and encrypted", testParsePdfinfoStdoutPagesAndEncrypted);
    runTest("pdfinfo stdout tolerates missing pages", testParsePdfinfoStdoutMissingPages);
    runTest("pdf inspection missing file safe", testPdfInspectionMissingFileSafe);
#if !defined(_WIN32)
    runTest("process runner echo stdout", testProcessRunnerEchoStdout);
    runTest("process runner false exit", testProcessRunnerFalseExit);
    runTest("pdf inspection invalid file rejected", testPdfInspectionInvalidPdfFile);
    runTest("Poppler Tesseract fixture candidates", testPopplerTesseractFixtureCandidates);
    runTest("inventory with Poppler pdfinfo", testInventoryWithPopplerPdfinfo);
    runTest("volume extract rewrites page-quality", testVolumeExtractRewritesPageQuality);
    runTest("volume bootstrap end-to-end", testVolumeBootstrapEndToEnd);
    runTest("volume bootstrap init without extraction", testVolumeBootstrapInitWithoutExtraction);
#endif
    runTest("volume metadata write", testVolumeMetadataWrite);
    runTest("volume metadata printed page label", testVolumeMetadataPrintedPageLabel);
    runTest("volume metadata cover page", testVolumeMetadataCoverPage);
    runTest("volume metadata rejects invalid page map",
            testVolumeMetadataRejectsInvalidPageMap);
    runTest("output config accepts local path", testOutputConfigAcceptsLocalPath);
    runTest("output config rejects URI path", testOutputConfigRejectsUriPath);
    runTest("TC-NFR-001 output config rejects hosted schemes",
            testTnfr001OutputConfigRejectsHostedSchemePaths);
    runTest("TC-NFR-001 local path intent heuristic", testTnfr001LocalPathIntentHeuristic);
    runTest("TC-NFR-001 bootstrap rejects hosted source URI", testTnfr001BootstrapRejectsHostedSourceUri);
    runTest("work-folder backup dry-run", testWorkFolderBackupDryRun);
    runTest("work-folder backup creates protected copy",
            testWorkFolderBackupCreatesProtectedCopy);
    runTest("work-folder restore requires confirmation",
            testWorkFolderRestoreRequiresConfirmation);
    runTest("work-folder restore confirmed", testWorkFolderRestoreConfirmed);
    runTest("artifact cleanup dry-run raw candidates", testArtifactCleanupDryRunRawCandidates);
    runTest("artifact cleanup confirmed raw candidates",
            testArtifactCleanupConfirmedRawCandidates);
    runTest("artifact cleanup protects reviewed text", testArtifactCleanupProtectsReviewedText);
    runTest("valid work folder", testValidWorkFolder);
    runTest("missing page file", testMissingPageFile);
    runTest("invalid review status", testInvalidReviewStatus);
    runTest("invalid JSON safe diagnostics", testInvalidJsonIsSafe);
    runTest("work-folder initialization layout", testWorkFolderInitializationCreatesValidLayout);
    runTest("work-folder initialization review defaults",
            testWorkFolderInitializationReviewDefaults);
    runTest("work-folder initialization rejects invalid page count",
            testWorkFolderInitializationRejectsInvalidPageCount);
    runTest("work-folder initialization preserves source PDF",
            testWorkFolderInitializationDoesNotModifySourcePdf);
    runTest("dependency configured path", testDependencyCapabilityConfiguredPath);
    runTest("dependency search path", testDependencyCapabilitySearchPath);
    runTest("dependency missing required", testDependencyCapabilityMissingRequired);
    runTest("file change unchanged", testFileChangeDetectorUnchanged);
    runTest("file change changed", testFileChangeDetectorChanged);
    runTest("file change missing", testFileChangeDetectorMissing);
    runTest("file change safe messages", testFileChangeDetectorSafeMessages);
    runTest("reviewed page text load", testReviewedPageTextLoad);
    runTest("reviewed page text save plain text", testReviewedPageTextSavePlainText);
    runTest("reviewed page text save rejects conflict", testReviewedPageTextSaveRejectsConflict);
    runTest("reviewed page text save overwrite conflict",
            testReviewedPageTextSaveOverwriteConflict);
    runTest("reviewed page text safe messages", testReviewedPageTextSafeMessages);
    runTest("text normalization plain text cleanup", testTextNormalizationPlainTextCleanup);
    runTest("text normalization quality flags", testTextNormalizationQualityFlags);
    runTest("candidate storage raw only", testCandidateStorageIsRawOnly);
    runTest("candidate OCR load separate artifact", testCandidateOcrLoadSeparateArtifact);
    runTest("candidate comparison counts and flags", testCandidateComparisonCountsAndFlags);
    runTest("candidate restore explicit only", testCandidateRestoreExplicitOnly);
    runTest("candidate restore rejects conflict", testCandidateRestoreRejectsConflict);
    runTest("OCR routing Paper Capture all pages", testOcrRoutingPaperCaptureAllPages);
    runTest("OCR routing weak embedded text", testOcrRoutingWeakEmbeddedText);
    runTest("OCR routing tool unavailable", testOcrRoutingToolUnavailable);
    runTest("page-quality writes candidate diagnostics",
            testPageQualityWritesCandidateDiagnostics);
    runTest("page-quality rejects invalid page ID", testPageQualityRejectsInvalidPageId);
    runTest("readiness summary counts", testReadinessSummaryCounts);
    runTest("readiness summary validation errors", testReadinessSummaryValidationErrors);
    runTest("readiness summary safe messages", testReadinessSummarySafeMessages);
    runTest("validation repair plan repairable findings",
            testValidationRepairPlanRepairableFindings);
    runTest("validation repair plan non-repairable findings",
            testValidationRepairPlanNonRepairableFindings);
    runTest("validation repair plan does not write", testValidationRepairPlanDoesNotWrite);
    runTest("validation repair plan safe messages", testValidationRepairPlanSafeMessages);

    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed\n";
        return 1;
    }
    std::cout << "core_tests passed\n";
    return 0;
}
