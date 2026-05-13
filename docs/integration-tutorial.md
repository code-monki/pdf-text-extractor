# Integration tutorial: linking `pdf_text_extractor_core` from a host app

| Field          | Value                                                                              |
| -------------- | ---------------------------------------------------------------------------------- |
| Status         | Active (Gate 9 documentation closure)                                              |
| Date           | 2026-05-13                                                                         |
| Audience       | C++ integrators who want to drive extraction in-process from another application   |
| Demo target    | `pte_demo_app` (`examples/demo_app/`)                                              |
| CMake flag     | `PDF_TEXT_EXTRACTOR_BUILD_DEMO_APP` (default **OFF**)                              |
| Library target | `pdf_text_extractor_core` (static library; public headers under `src/`)            |
| Source files   | `examples/demo_app/main.cpp`, `examples/demo_app/CMakeLists.txt`                   |
| Doc index      | [docs/README.md](README.md)                                                       |

This tutorial walks an integrator through linking the reusable extraction library `pdf_text_extractor_core` in-process and calling the documented service facades. It uses the bundled `pte_demo_app` example as the worked walkthrough; the same patterns transfer to any host C++ application that adds the library as a CMake dependency.

The demo is **opt-in** so the default and CI build stay light, mirroring the existing pattern used for the Qt reviewer shell (`PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL`).

---

## 1. Why a library demo, not a CLI demo?

The headless CLI `pte_bootstrap` (`docs/cli-reference.md`) is the right integration path when a host runs extraction as a subprocess and consumes the work-folder artifact contract from disk. The library demo covers the **in-process** path: when a host wants to call the same domain services that `pte_bootstrap` and `pte_shell` already drive, without spawning a child process or parsing CLI output.

Both paths share the same artifact contract (`docs/adrs/0002-work-folder-artifact-contract.md`), so downstream consumers see identical `volume.json`, `page-quality.json`, `review-state.json`, and `pages/NNNN.txt` regardless of how extraction was launched.

The demo is not a full reviewer UI; it intentionally stays headless so the integration story is unobscured by Qt or QML.

---

## 2. Architectural posture and limits

Read first, in order:

- `AGENTS.md` (mandatory operational contract, source material handling, defensive checks).
- `docs/adrs/0001-layered-native-architecture.md` (layering authority).
- `docs/adrs/0002-work-folder-artifact-contract.md` (downstream artifact contract).
- `docs/adrs/0003-ocr-probabilistic-boundary.md` (OCR-BND-001: OCR output is never authoritative on its own).
- `docs/adrs/0005-pdf-inspection-poppler-family.md` (PDF inspection adapter family).
- `docs/high-level-architecture.md` §6 for component IDs referenced below (HLA-INV, HLA-WORK, HLA-EXT, HLA-OCR, HLA-VALID, HLA-DIAG, HLA-APP).

The demo deliberately stays inside the **application layer** (HLA-APP): it calls already-published facades and never reaches around them to touch adapters, raw artifacts, or UI types.

Hard limits that hold for any host using this path:

- Source PDFs are never modified; the SHA-256 over source bytes is stable across bootstrap runs.
- The work-folder root is **user-configurable**; the demo requires `--work DIR` and does not hard-code one.
- No corpus, no rights system, and no copyrighted page text appears in stdout, stderr, or any safe report emitted by the demo.
- Raw OCR candidates remain probabilistic; only `pages/NNNN.txt` with reviewed status is release-eligible. The demo does not promote anything; review remains the operator's responsibility through `pte_shell` or another reviewer integration.

---

## 3. Prerequisites

- CMake 3.25 or newer (matches the top-level `CMakeLists.txt`).
- A C++20 compiler (Clang, GCC, or MSVC; Apple Clang 15+ tested).
- For the full extraction stage of the demo: Poppler-family `pdfinfo`, `pdftotext`, `pdftoppm`, and Tesseract (`tesseract`). The demo's `--init-only` mode skips all of these so the integration path stays exercisable on minimal hosts.

No additional dependencies beyond what `pdf_text_extractor_core` already requires; the demo does not pull in Qt.

---

## 4. Enabling the demo build

The top-level option is **off by default**. Configure with it on:

```bash
cmake -S . -B build-demo -DPDF_TEXT_EXTRACTOR_BUILD_DEMO_APP=ON
cmake --build build-demo --target pte_demo_app
```

The resulting binary is at `build-demo/examples/demo_app/pte_demo_app`. Toggling the flag back off (or omitting it) leaves the existing default targets — `pdf_text_extractor_core`, `core_tests`, `pte_bootstrap`, `pte_enrich` — unaffected.

The flag mirrors `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL`. CI does not need to enable either flag.

---

## 5. Running the demo

The demo accepts a user-supplied PDF or the bundled synthetic fixture (`tests/fixtures/hello.pdf`, the only PDF that is intentionally committed to the repository for tests). Either way, the **work-folder root** is user-supplied.

### 5.1 Default fixture, init only (no Poppler/Tesseract needed)

```bash
WORK=$(mktemp -d)
build-demo/examples/demo_app/pte_demo_app \
    --default-fixture \
    --work "$WORK/demo" \
    --volume-id demo-fixture \
    --title "Demo Fixture" \
    --init-only
```

Expected output (excerpt):

```text
[bootstrap] volume work folder initialized without candidate extraction
[bootstrap] page_count: 1
[bootstrap] inventory:   yes
[bootstrap] initialized: yes
[bootstrap] extraction:  no
[readiness] readiness summary created
[readiness] status_counts: raw=1
[validate] ok: yes; error_count: 0; total_findings: 0
```

After the run, `$WORK/demo` contains `volume.json`, `page-quality.json`, `review-state.json`, `pages/`, and empty `raw/embedded/` and `raw/ocr/` directories. No bytes were written to the source fixture.

### 5.2 Full bootstrap (embedded + OCR) against a user PDF

```bash
WORK=$(mktemp -d)
build-demo/examples/demo_app/pte_demo_app \
    --source /path/to/your.pdf \
    --work "$WORK/demo" \
    --corpus /path/to/your-corpus-root \
    --volume-id your-volume-id \
    --title "Your Title"
```

When Poppler/Tesseract are on `PATH`, `[bootstrap] extraction: yes` and `raw/embedded/` and `raw/ocr/` populate. Use `--no-ocr` to run embedded extraction only when Tesseract is unavailable or unwanted.

### 5.3 Defensive failures

Invalid inputs exit with code 2 and a one-line safe message:

```text
source PDF not found: "/no/such/file.pdf"
```

The demo validates `--source` existence, regular-file status, and non-empty `--work`, `--volume-id`, and `--title` before touching the library, matching the defensive checks contract in `AGENTS.md` §15.

---

## 6. What the demo proves (FR coverage)

The demo exercises the same library entry points that the shell and CLI rely on. Each line below is the integration capability the demo confirms; full requirement text lives in `docs/software-requirements-specification.md` and `docs/requirements-traceability-matrix.md`.

- **FR-001 / FR-002**: One source PDF is selected and inventoried in-process via `VolumeBootstrapService`, which delegates to `SourceInventoryService::inventoryWithPopplerInspection`.
- **FR-004**: A user-supplied work-folder root is created and populated with the canonical artifact set (`volume.json`, `page-quality.json`, `review-state.json`, `pages/`, `raw/`).
- **FR-012 / FR-013 / FR-024**: When extraction is enabled, raw embedded and (optionally) raw OCR candidates are stored, and `page-quality.json` reflects tool metadata, character counts, and flags.
- **FR-025**: `WorkFolderValidator::validate` is called against the populated work folder and emits safe structural findings only.
- **FR-027**: `ReadinessSummaryService::summarizeWorkFolder` returns aggregate status, selected-source, and flag counts without any reviewed page text.
- **FR-029**: The `--work` path is user-configurable; no corpus or output destination is hard-coded.
- **FR-031**: Tool capability is reflected through `[bootstrap] extraction: yes|no` and the `--no-ocr`/`--init-only` toggles that mirror dependency-gated behavior.
- **NFR-001 / NFR-003**: Local-only operation; the source PDF is not modified (the demo prints the source path it preserved).
- **NFR-012**: All printed messages are safe (counts, identifiers, flag codes — no page text or source bytes).

The demo intentionally does **not** exercise review-state mutations (FR-017+), backup/restore (FR-023), or derived-PDF enrichment (FR-034..036). Those flows have dedicated facades (`ReviewStateService`, `WorkFolderBackupService`, `PdfEnrichmentService`) and are out of scope here; see `pte_shell` and `pte_enrich` for working integrations.

---

## 7. Lifting the patterns into another application

The relevant headers are public on `pdf_text_extractor_core`'s include path (`src/`). Add the library to your CMake project and link against it:

```cmake
target_link_libraries(your_app PRIVATE pdf_text_extractor_core)
target_compile_features(your_app PRIVATE cxx_std_20)
```

(`pdf_text_extractor_core` is consumed from this repository's CMake tree; a formal export/installed-package surface is **not** authorized yet — see `docs/high-level-architecture.md` §3.2 and HLA-RISK-005 — so embed this repo as a submodule or vendor it for now.)

Then call the three services demonstrated above, in roughly the same order:

```cpp
#include "core/volume_bootstrap.hpp"
#include "core/readiness_summary.hpp"
#include "core/work_folder_validator.hpp"

pte::core::VolumeBootstrapRequest request;
request.corpusRoot       = userCorpusRoot;       // never hard-code
request.sourcePdfPath    = userSourcePdf;        // local file selected by operator
request.workFolder       = userWorkFolder;       // user-configurable output root
request.volumeId         = stableVolumeId;       // your application's stable ID scheme
request.title            = userFacingTitle;
request.runCandidateExtraction      = wantExtraction;
request.generation.extractEmbedded  = true;
request.generation.extractOcr       = wantExtraction && hasTesseract;

const auto outcome = pte::core::VolumeBootstrapService().run(request);
if (!outcome.success) {
    // Surface outcome.safeMessage; do NOT log source page text.
    return;
}

const auto readiness =
    pte::core::ReadinessSummaryService().summarizeWorkFolder(userWorkFolder);
const auto report =
    pte::core::WorkFolderValidator().validate(userWorkFolder);
```

Patterns to copy verbatim:

- **User-configurable work folder**: take it from configuration or a path picker; never derive it from a corpus you don't control. (FR-029, NFR-008.)
- **Defensive input checks before calling services**: `fs::exists` and `fs::is_regular_file` on the source PDF, non-empty volume identifiers, etc. (AGENTS.md §15 defensive checks contract.)
- **Safe error surfaces**: only propagate `outcome.safeMessage`, finding codes, and counts to UI/logs. Reviewed page text and source PDF bytes stay inside the work folder.
- **Skip OCR cleanly when Tesseract is absent**: probe with `DependencyCapabilityService` or follow the demo's `--no-ocr` / `--init-only` toggles. OCR remains probabilistic (ADR-0003) — your application is responsible for keeping it review-gated.
- **Read back artifacts, do not maintain a parallel cache**: `ReadinessSummary` and `ValidationReport` re-read disk. Treat the work folder as the source of truth; downstream consumers (e.g. `/Users/chuck/ct-cd`) do the same.

Anti-patterns to avoid:

- Calling adapters (`PopplerEmbeddedTextExtractor`, `TesseractOcr`, etc.) directly to "speed things up". The facades preserve the artifact contract; reaching past them risks duplicating logic and breaking review-state safety.
- Persisting reviewed page text into your own data store before review status is `accepted`. Release/indexing eligibility comes from `review-state.json` only.
- Hard-coding tool paths, languages, or DPI in your shipped configuration. Use `PopplerTesseractToolPaths` and `CandidateGenerationOptions` exactly as the demo and `pte_bootstrap` do.

---

## 8. Build matrix summary

| Flag                                    | Default | Enables                                                                |
| --------------------------------------- | ------- | ---------------------------------------------------------------------- |
| _(none)_                                | —       | `pdf_text_extractor_core`, `core_tests`, `pte_bootstrap`, `pte_enrich` |
| `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON`  | OFF     | + `pte_shell` and the `TC-FR-028` native-binary probe                  |
| `PDF_TEXT_EXTRACTOR_BUILD_DEMO_APP=ON`  | OFF     | + `pte_demo_app` (this tutorial)                                       |

The demo flag is composable with the Qt shell flag; you can enable both in the same configure call without conflicts.

---

## 9. Traceability

- Requirements (illustrative; full lineage in `docs/requirements-traceability-matrix.md`): FR-001, FR-002, FR-004, FR-012, FR-013, FR-024, FR-025, FR-027, FR-029, FR-031, NFR-001, NFR-003, NFR-012.
- Architecture: HLA-APP (consumer), HLA-INV, HLA-WORK, HLA-EXT, HLA-OCR, HLA-VALID, HLA-DIAG (see `docs/high-level-architecture.md` §6).
- ADRs: ADR-0001 (layering), ADR-0002 (work-folder contract), ADR-0003 (OCR boundary), ADR-0005 (Poppler-family).
- Related operator docs: `docs/cli-reference.md` (subprocess integration path), `docs/ui-shell.md` and `docs/shell-user-guide.md` (Qt reviewer shell).
- Phase: Gate 9 documentation closure (`docs/phase-gate-record.md`). Phase B installers and a formal exported library package remain deferred; this demo intentionally stays within the in-tree library surface.

---

## 10. Known limitations and follow-ups

- The library is consumed in-tree only; there is no installed CMake config package yet (`find_package(pdf_text_extractor_core)`). When/if a packaging directive authorizes that, this tutorial should be extended with a `find_package`-based integration section.
- The demo does not exercise review-state edits, backup/restore, or derived-PDF enrichment. Each of those has dedicated services that follow the same defensive style; see `src/core/review_state.hpp`, `src/core/work_folder_backup.hpp`, and `src/core/pdf_enrichment.hpp` for entry points.
- Cross-platform packaging of the demo binary itself is not authorized; CPack archives (Phase A) currently ship `pte_bootstrap`, `pte_enrich`, and (when built) `pte_shell`. Adding `pte_demo_app` to the install set is out of scope for Gate 9 and should ride with Phase B if reuse demands it.

---

End of integration tutorial.
