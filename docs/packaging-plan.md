# Packaging plan (Gate 8)

| Field | Value |
|-------|--------|
| Status | Approved baseline (Phase A); Phase B installers deferred |
| Date | 2026-05-05 |
| SRS | NFR-014; constraints §7 |
| RTM | `docs/requirements-traceability-matrix.md` Packaging Ref column |
| Outputs | `dist/` (gitignored) — CPack **TGZ** and **ZIP** archives |
| Phase Gate | Gate 8 Phase A **closed** (2026-05-07); **Gate 9 closed** (2026-05-29); **Phase B** installers **deferred** — `docs/phase-gate-record.md` |

---

## 1. Purpose

Produce **reproducible, inspectable install layouts** for handoff to operators and downstream systems (indexers, QA, packaging pipelines). Packaging shall **not** embed copyrighted PDFs or extracted full-page text from restricted corpora.

---

## 2. What ships (Phase A — implemented)

CMake **install rules** + **CPack** (`cmake/Packaging.cmake`) lay out:

| Component | Default headless build | With `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON` |
|-----------|------------------------|---------------------------------------------|
| `pte_bootstrap` | `bin/` | `bin/` |
| `pte_enrich` | `bin/` | `bin/` |
| `enrich_lint_manifest.sh` | `bin/` | `bin/` |
| Core library | `lib/` (`libpdf_text_extractor_core.a` static on typical Unix static link context; shared/DLL layout on platforms that build shared) | same |
| `pte_shell` | — | macOS: `pte_shell.app` bundle at archive root; other OS: `bin/pte_shell` |

**Version string:** `project(... VERSION 0.1.0)` in root `CMakeLists.txt` — bumps propagate to archive names. User-visible release notes: `CHANGELOG.md` (keep in sync when bumping the CMake project version).

**Archive naming:** `pdf-text-extractor-<version>-<CMAKE_SYSTEM_NAME>-<processor>.{tar.gz,zip}`

**Build commands:**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target package
# Artifacts: dist/pdf-text-extractor-*.tar.gz and *.zip
```

With Qt shell (separate tree recommended):

```bash
cmake -S . -B build-qt-shell -DCMAKE_BUILD_TYPE=Release \
  -DPDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/macos
cmake --build build-qt-shell --target package
```

Orchestration: `make package` runs **`cmake --build $(BUILD_DIR) --target package`** (see `Makefile`).

---

## 3. Runtime dependencies (host-provided)

The archives **do not bundle** Poppler or Tesseract. Operators install platform packages or rely on **PATH** for at least:

- `pdfinfo`, `pdftotext`, `pdftoppm` (Poppler)
- `tesseract`

Optional shell adds **Qt 6** at runtime (typically satisfied because **pte_shell** links Qt dynamically).

Document dependency expectations alongside **`docs/cli-reference.md`** and **`docs/ui-shell.md`**.

---

## 4. Phase B — native installers (deferred)

Aligned with **SRS NFR-014** measurement criteria and **`docs/test-plan.md`** TC-NFR-014:

| Format | OS family | Notes |
|--------|-----------|--------|
| AppImage | Linux | Bundle deps or document PATH; CI runner choice |
| `.deb` / `.rpm` | Linux | Declares dependencies on `poppler-utils`, `tesseract`, optional Qt |
| `.dmg` | macOS | Code signing / notarization policy TBD |
| `.msi` | Windows | Qt runtime redistribution rules |

Phase B requires CI secrets, signing policy, and dependency packaging decisions beyond Phase A archives.

---

## 5. Validation (smoke)

After extracting an archive:

1. **`pte_bootstrap --help`** (or documented switches per `docs/cli-reference.md`) exits zero.
2. **`core_tests`** — run from **build tree** in CI or developer workflow (tests are **not** shipped in CPack by default; optional future `install` of test binary is out of scope for Phase A).

Shell builds: launch **`pte_shell`** / open **`pte_shell.app`** once; optional **`tc_fr028_probe`** from build tree against packaged binary path.

Full **NFR-013** matrix validation (every OS/arch in SRS §7) remains **incremental** with Phase B installers.

---

## 6. Pre-flight enrichment lint (FR-034/035/036)

Before creating release archives that include `pte_enrich`, run a sidecar lint sweep so malformed map files do not enter packaging handoff.

Single volume:

```bash
make enrich-lint \
  ENRICH_SOURCE=/absolute/path/to/volume.pdf \
  ENRICH_OUTLINE_MAP=/absolute/path/to/outline-map.json \
  ENRICH_LINK_MAP=/absolute/path/to/link-map.json
```

Batch (preferred for pipelines): `scripts/enrich_lint_manifest.sh` with a tab-separated manifest (see script header). Example:

```bash
PTE_ENRICH=/path/to/pte_enrich scripts/enrich_lint_manifest.sh /path/to/manifest.tsv
```

Repository fixture (CI): `tests/fixtures/enrichment/lint.manifest.tsv` references `tests/fixtures/hello.pdf` and valid outline/link JSON.

Manual loop (equivalent):

```bash
while IFS=$'\t' read -r source outline link; do
  [ -z "$source" ] && continue
  make enrich-lint \
    ENRICH_SOURCE="$source" \
    ENRICH_OUTLINE_MAP="$outline" \
    ENRICH_LINK_MAP="$link"
done < /absolute/path/to/manifest.tsv
```

This lint path validates sidecar schema/version, rectangle geometry, and destination linkage without writing derived PDFs.

---

## 7. Downstream manifest (future)

`docs/extractor-output-contract-concept.md` requests a machine-readable install manifest for consumers — **not** required for Gate 8 Phase A; track when Phase B installers stabilize.

---

## 8. Gate 8 exit criteria (project owner)

Phase A satisfies **“approved packaging plan exists”** + **artifact formats produced** (TGZ/ZIP). **Gate 8 Phase A → Gate 9** (documentation closure) is authorized **without** Phase B native installers; Phase B remains **deferred** and does not block Gate 9 per **`docs/phase-gate-record.md`**. Any future Phase B kickoff requires separate owner approval and plan updates.

---

## 9. Traceability

- **NFR-014:** Phase A = CPack TGZ/ZIP + `docs/packaging-plan.md`; Phase B = native installer matrix.
- **NFR-013:** Smoke on packaged binary on **primary** host during Gate 8; full matrix with Phase B where applicable.
- **Navigation enrichment (FR-034/035/036):** see ADR-0006 (proposed status) and `docs/enrichment-sidecar-schema.md`; packaging must define whether enriched PDFs and mapping sidecars are distributable outputs.
- **FR-034/035/036 prototype inputs:** `docs/enrichment-sidecar-schema.md`; prototype CLI `pte_enrich` included in archives.
