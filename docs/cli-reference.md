# CLI reference: `pte_bootstrap`


| Field          | Value                                                                                                             |
| -------------- | ----------------------------------------------------------------------------------------------------------------- |
| Status         | Active (implementation phase)                                                                                     |
| Date           | 2026-05-03                                                                                                        |
| Executable     | `pte_bootstrap` (CMake target; built beside `core_tests`)                                                         |
| Implementation | `src/cli/bootstrap_main.cpp`                                                                                      |
| Domain service | `pte::core::VolumeBootstrapService` (`docs` artifact: behavior described under volume bootstrap / FR-004, FR-024) |


This document is the **operator-facing catalog** of command-line switches. When in doubt, switches must match the parser in `src/cli/bootstrap_main.cpp`.

---

## Purpose

Create or populate an extraction **work folder** from a local **source PDF**: run pdfinfo-backed inventory, initialize `volume.json`, `review-state.json`, `page-quality.json`, and empty `pages/NNNN.txt`, then optionally run Poppler/Tesseract **candidate extraction** into `raw/embedded/` and `raw/ocr/`.

The CLI prints **safe summaries only** (messages, counts, yes/no flags). It does **not** print extracted page text.

---

## Building

After configuring CMake from the repository root:

```bash
cmake --build <build-dir> --target pte_bootstrap
```

Typical binary path: `<build-dir>/pte_bootstrap`.

---

## Synopsis

```text
pte_bootstrap --corpus DIR --source PATH --work DIR --volume-id ID --title TEXT [optional-flags...]
pte_bootstrap {-h|--help}
```

All long options use the form `--name value` for arguments that take a value (two separate argv tokens). Boolean flags take no value.

---

## Required switches


| Switch        | Argument | Description                                                                                  |
| ------------- | -------- | -------------------------------------------------------------------------------------------- |
| `--corpus`    | `DIR`    | Corpus root used when recording corpus-relative path in inventory (source PDF is not moved). |
| `--source`    | `PATH`   | Path to the source PDF file. The file is **never modified**.                                 |
| `--work`      | `DIR`    | Work folder root to create or fill (`volume.json`, `pages/`, `raw/`, etc.).                  |
| `--volume-id` | `ID`     | Stable volume identifier stored in work-folder artifacts.                                    |
| `--title`     | `TEXT`   | Volume title metadata.                                                                       |


---

## Optional switches


| Switch         | Argument | Description                                                                                                                                                 |
| -------------- | -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `--subtitle`   | `TEXT`   | Volume subtitle.                                                                                                                                            |
| `--sort-title` | `TEXT`   | Sort title metadata.                                                                                                                                        |
| `--group`      | `TEXT`   | Group metadata.                                                                                                                                             |
| *(none)*       | —        | `**--init-only`** — Initialize the work folder only; **skip** candidate extraction (embedded + OCR). Implies no OCR stage regardless of `--no-ocr`.         |
| *(none)*       | —        | `**--no-ocr`** — When extraction runs (default), run **embedded** candidate generation only; **do not** run Tesseract OCR. Ignored if `--init-only` is set. |
| `--pdfinfo`    | `PATH`   | Override the `pdfinfo` executable used for structural inspection.                                                                                           |
| `--pdftotext`  | `PATH`   | Override `pdftotext` for embedded extraction.                                                                                                               |
| `--pdftoppm`   | `PATH`   | Override `pdftoppm` for page rasterization before OCR.                                                                                                      |
| `--tesseract`  | `PATH`   | Override `tesseract` for OCR.                                                                                                                               |
| `-h`, `--help` | —        | Print usage and exit **0** (does not run bootstrap).                                                                                                        |


---

## Exit codes


| Code | Meaning                                                                                                               |
| ---- | --------------------------------------------------------------------------------------------------------------------- |
| `0`  | Success (`--help`, or bootstrap completed).                                                                           |
| `1`  | Bootstrap failed; one-line safe message on **stderr**.                                                                |
| `2`  | Usage error: unknown option, missing value after an option, missing required switch, or missing value where required. |


---

## Success output (stdout)

On success (exit `0`), the program prints:

1. One line: outcome safe message from `VolumeBootstrapService`.
2. If available: `page_count: <n>`
3. `inventory: yes|no`
4. `initialized: yes|no`
5. `extraction: yes|no` (whether candidate extraction completed; `no` when `--init-only` or when extraction was not run).

No PDF or extracted text content is printed.

---

## Examples

Full bootstrap (inventory, init, embedded + OCR when tools are available):

```bash
pte_bootstrap \
  --corpus /path/to/corpus \
  --source /path/to/document.pdf \
  --work /path/to/work-folder \
  --volume-id my-book \
  --title "My Book"
```

Initialize work folder only (no `raw/` candidate files from extraction; placeholder diagnostics from initializer):

```bash
pte_bootstrap \
  --corpus /path/to/corpus \
  --source /path/to/document.pdf \
  --work /path/to/work-folder \
  --volume-id my-book \
  --title "My Book" \
  --init-only
```

Embedded extraction only (no Tesseract):

```bash
pte_bootstrap \
  --corpus /path/to/corpus \
  --source /path/to/document.pdf \
  --work /path/to/work-folder \
  --volume-id my-book \
  --title "My Book" \
  --no-ocr
```

Pinned tool paths (reproducibility / non-default installs):

```bash
pte_bootstrap \
  --corpus /path/to/corpus \
  --source /path/to/document.pdf \
  --work /path/to/work-folder \
  --volume-id my-book \
  --title "My Book" \
  --pdfinfo /opt/homebrew/bin/pdfinfo \
  --pdftotext /opt/homebrew/bin/pdftotext \
  --pdftoppm /opt/homebrew/bin/pdftoppm \
  --tesseract /opt/homebrew/bin/tesseract
```

---

## Traceability

- Requirements: FR-004 (work folder / initialization path), FR-024 (diagnostics / extraction posture), FR-031 (dependency reporting; overrides relate to local tool discovery).
- RTM: `docs/requirements-traceability-matrix.md` (lineage entries for CLI).
- Related code: `src/core/volume_bootstrap.hpp`, `src/core/volume_extraction_pipeline.hpp`, `src/core/candidate_generation_poppler.hpp`.

---

## Prototype enrichment CLI: `pte_enrich`

Executable: `pte_enrich` (`src/cli/enrich_main.cpp`)  
Purpose: Gate 8 prototype for FR-034/035/036 sidecar-driven derived PDF enrichment workflow.

### Synopsis

```text
pte_enrich --source PATH --output-pdf PATH [--outline-map PATH] [--link-map PATH] [--report PATH]
pte_enrich --source PATH --lint-only [--outline-map PATH] [--link-map PATH]
pte_enrich {-h|--help}
```

### Current behavior (prototype)

- validates `outline-map.json` / `link-map.json` structure when provided,
- rejects in-place source mutation (`--output-pdf` must differ from `--source`),
- by default attempts bookmark/link annotation injection via Python script `scripts/pdf_enrich_apply.py`
  (requires local `pikepdf`; use `--prototype-only` to skip injection and emit copy+report only),
- `--lint-only` validates outline/link sidecars and destination registry consistency without writing a derived PDF,
- writes a safe `enrichment-report.json`.

Schema reference: `docs/enrichment-sidecar-schema.md`.

Batch pre-flight (many volumes): `scripts/enrich_lint_manifest.sh` — see `docs/packaging-plan.md` §6 and `make enrich-lint-fixtures`.

