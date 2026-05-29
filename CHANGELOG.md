# Changelog

All notable changes to this project are documented here.

The **authoritative release version** is `project(pdf_text_extractor VERSION …)` in the repository root `CMakeLists.txt` (currently **0.1.0**). CPack archive names and operator-facing “about” strings should track that value; this file summarizes user-visible changes by release.

Format is inspired by [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.1.0] — 2026-05-13

### Added

- **Documentation:** `docs/README.md` as the documentation index; root `README.md` entry point.
- **Integrators:** `docs/integration-tutorial.md` — in-process use of `pdf_text_extractor_core` via optional `pte_demo_app` (`PDF_TEXT_EXTRACTOR_BUILD_DEMO_APP`, default OFF).
- **Release hygiene:** This changelog for Gate 9 alignment with packaged version strings.

### Notes

- **Packaging (Phase A):** Portable **TGZ/ZIP** via CPack; see `docs/packaging-plan.md`. **Phase B** native installers (`.deb`, `.rpm`, `.dmg`, `.msi`, signing, expanded CI matrix) remain **deferred** per `docs/phase-gate-record.md`.
- **Headless CLI:** `pte_bootstrap` — `docs/cli-reference.md`.
- **Enrichment prototype:** `pte_enrich`, sidecar schema — `docs/cli-reference.md` (bottom), `docs/enrichment-sidecar-schema.md`.

### Verified (2026-05-29)

- Primary dev host: `make check`, `make enrich-lint-fixtures`, `pte_shell` Release build + `tc_fr028_probe` (Gate 9 exit).
