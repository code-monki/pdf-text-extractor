# Test Plan

Project Name: pdf-text-extractor
Version: 0.1
Date (YYYY-MM-DD): 2026-05-02
Author(s): Chuck, Codex
Status: Approved
Requirement Version Reference: `docs/software-requirements-specification.md` v0.1 approved
Architecture Version Reference: `docs/high-level-architecture.md` v0.1 approved
RTM Version Reference: `docs/requirements-traceability-matrix.md` v0.1

**Lifecycle position (authoritative):** Gates 1–9 closed (2026-05-29); **Packaging Phase B** installers and orchestration deferred. See `docs/phase-gate-record.md`.

---

# 1. Test Authority Declaration

Confirm:

- Requirements approved? Yes
- Architecture approved? Yes
- Detailed Design approved? Yes
- RTM initialized? Yes
- Advancement to Test Planning authorized? Yes

Traceability Consolidation was approved by project-owner direction to proceed to Test Planning.

---

# 2. Test Scope

Requirement IDs in scope:

- FR-001 through FR-033
- NFR-001 through NFR-016

NFR categories in scope:

- Compliance
- Reliability
- Reproducibility
- Observability
- Usability
- Data integrity
- Maintainability
- Platform support
- Packaging targets
- Accessibility
- Theme accessibility

Packaging validation scope:

- Planned only in this phase.
- Package artifact targets are AppImage, `.deb`, `.rpm`, `.dmg`, and `.msi`.
- Execution and package-build validation remain Packaging phase work.

Orchestration validation scope:

- Planned only in this phase.
- CI/clean-build commands and package build orchestration remain later phase artifacts.

Deterministic-probabilistic containment validation scope:

- OCR-BND-001 must be validated.
- OCR and embedded candidates must remain raw candidates until reviewed text and review state make a page eligible.
- OCR output must not self-authorize accepted status or downstream release eligibility.
- OCR diagnostics must expose tool identity, candidate availability, character counts, flags, errors, and suspicious embedded/OCR deltas.

Out of scope:

- Executing tests.
- Writing implementation code.
- Packaging automation.
- Network/cloud-provider API validation.
- Multi-user collaboration or distributed locking.
- Storing copyrighted PDF source material or full extracted page text in test fixtures committed to git.

---

# 3. Test Objectives

Testing must prove that the system:

- initializes and maintains the approved work-folder contract
- preserves source PDFs unchanged
- keeps raw candidates, reviewed page text, review state, diagnostics, and metadata separated
- supports the native review workflow without stale page/text state
- protects user edits and reviewed text from silent loss or overwrite
- validates work folders without exposing substantive page text
- contains OCR uncertainty behind OCR-BND-001
- reports dependency/tool capability problems clearly
- remains local, reproducible, inspectable, and corpus-agnostic
- satisfies baseline platform, accessibility, and theme requirements before release

Release readiness is not established by this test plan alone. Release readiness requires implementation, executed tests, clean build validation, packaging validation, updated RTM evidence, and human approval.

---

# 4. Test Strategy

## 4.1 Testing Levels

- Unit Testing: validates domain services, schema rules, path rules, state transitions, normalization helpers, report-safety filters, and adapter contract boundaries.
- Integration Testing: validates PDF inventory, embedded extraction, OCR adapter behavior, work-folder generation, dependency reporting, and UI-facing application facades.
- System Testing: validates one-PDF review workflow from selection through initialization, extraction, editing, status update, validation, and readiness reporting.
- Regression Testing: repeats work-folder validation, OCR containment, save-state protection, and source-material safety checks after changes.
- Performance Testing: validates progress/observability and acceptable responsiveness for representative smaller and larger volumes.
- Security/Compliance Testing: validates local-only operation, `.gitignore` protection, source PDF non-modification, and safe diagnostics/reporting.
- Reliability/Resilience Testing: validates malformed PDFs, missing tools, damaged work folders, external file changes, interrupted writes, and cleanup confirmation behavior.
- User Acceptance Testing: validates sequential page review, metadata editing, keyboard shortcuts, tooltips, themes, and practical manual normalization flow.
- Packaging Validation: planned for Packaging phase against Linux, macOS, and Windows artifact targets.
- Orchestration/Clean Build Validation: planned for later phases using documented commands and CI where feasible.

## 4.2 Deterministic-Probabilistic Validation

Acceptance boundaries:

- Raw OCR output is never authoritative by itself.
- Release/indexing eligibility is controlled by reviewed page text plus review state.
- OCR failures must be visible in diagnostics and must not be hidden as successful extraction.
- Embedded/OCR character-count deltas greater than 5 percent must be flagged as suspicious.

Variability tolerance:

- OCR text content may vary by engine version and mode.
- Tests compare structural artifacts, candidate presence, diagnostics, status transitions, and containment behavior rather than requiring exact OCR transcription except for controlled synthetic fixtures.

Rejection criteria:

- OCR candidate directly changes accepted status.
- OCR candidate overwrites reviewed text without explicit user action.
- OCR failure is reported as success.
- Diagnostic reports expose substantive copyrighted page text.
- Missing tool produces silent failure.

Containment validation tests:

- TC-BND-OCR-001 through TC-BND-OCR-006.

Drift detection:

- Full OCR drift monitoring is not in current scope.
- Tool versions, OCR profile, and candidate diagnostics must be recorded so future drift analysis remains possible.

## 4.3 Functional Validation

Each FR maps to at least one test case. Functional tests cover nominal behavior, error behavior, local-file boundaries, state transitions, destructive-action confirmation, and UI workflow consistency.

## 4.4 Non-Functional Validation

Each NFR maps to at least one test case. NFR tests focus on measurable checks: local-only operation, source protection, source PDF hash stability, reproducibility metadata, auditability, usability, data integrity, corpus agnosticism, native UX, observability, schema versioning, safe reporting, platform targets, package target visibility, accessibility, and theme accessibility.

---

# 5. Traceability Enforcement

All test cases reference requirement IDs. No requirement may enter implementation without at least one planned test case in the RTM.

RTM update rules:

- Requirement rows map to TC-FR or TC-NFR identifiers during Test Planning.
- OCR-BND-001 rows map to TC-BND-OCR identifiers.
- Implementation Unit remains pending until Implementation phase.
- Packaging and Orchestration references remain pending until their phases.
- Validation Status remains Planned until tests exist and execute.

---

# 6. Test Environment

Initial local planning environment:

- Hardware: Apple Mac Mini M4 Pro, 64GB RAM
- OS: macOS, Apple Silicon
- Local tools under evaluation: Qt 6.10.3 or newer, Poppler-family PDF tooling, Tesseract OCR
- Representative corpus: local PDFs only, including known difficult scanned/table-heavy volumes
- Ground truth: local normalized text samples may be used for developer inspection, but substantive page text must not be committed or reproduced in reports

Target platform matrix:

| Operating System/Distro | Architectures    | Minimum Version |
| ----------------------- | ---------------- | --------------- |
| Linux/Fedora            | x86_64 / AArch64 | 44              |
| Linux/Ubuntu            | x86_64 / AArch64 | 26.04 LTS       |
| macOS                   | x86_64 / AArch64 | 26.4            |
| Windows                 | x86_64           | 11              |

Clean build validation:

- Required before Implementation may be considered complete.
- CI strategy is deferred to Implementation/Packaging but must support explicit Qt setup rather than assuming system packages provide the required Qt version.

---

# 7. Test Data Governance

Data sources:

- Synthetic PDFs generated specifically for tests.
- Small committed synthetic work folders with non-copyrighted text.
- Local-only representative PDFs for manual/exploratory validation.
- Local-only normalized ground-truth samples for OCR evaluation.

Committed fixtures may include:

- Non-copyrighted synthetic PDFs.
- Synthetic `volume.json`, `page-quality.json`, `review-state.json`, and `pages/*.txt`.
- Corrupted or malformed synthetic JSON/work-folder artifacts.

Committed fixtures must not include:

- Source PDFs from restricted corpora.
- Extracted full-text derivatives from restricted PDFs.
- OCR/raw candidates from restricted PDFs.
- Diagnostic reports containing substantive page text from restricted PDFs.

Retention:

- Local representative corpus outputs remain ignored local artifacts.
- Cleanup tests use synthetic data only unless explicitly approved.

---

# 8. Entry Criteria

Testing definition may begin when:

- Detailed Design is approved.
- Traceability Consolidation is approved.
- RTM exists and maps requirements through DD.
- Test cases are drafted and reviewed.
- Source-material safety rules are understood.

Implementation may begin only after:

- Test Plan is approved.
- RTM contains requirement-to-test mappings.
- High-risk failure scenarios have planned coverage.
- OCR-BND-001 containment validation is planned.

---

# 9. Exit Criteria

Test Planning is complete when:

- Every FR maps to at least one test case.
- Every NFR maps to at least one test case.
- OCR-BND-001 maps to containment tests.
- Failure scenarios are identified.
- Automation expectations are defined.
- RTM is updated with Test Case IDs.
- Human approval is granted to proceed to Implementation.

Testing execution is complete only when:

- All high-priority FR tests pass.
- All critical NFR tests pass.
- OCR containment tests pass.
- No unresolved high-severity defects remain.
- Clean build validation passes.
- Packaging validation passes in Packaging phase.
- RTM evidence is updated.
- Human approval is granted.

---

# 10. Defect Governance

Severity levels:

- Critical: data loss, source PDF modification, copyrighted text leakage, unreviewed text release eligibility, broken work-folder contract, or inability to open/use the application.
- High: failed extraction/validation path without clear diagnostics, dirty edit loss, backup failure, inaccessible core workflow, or incorrect status gating.
- Medium: degraded workflow, incorrect readiness counts, incomplete dependency reporting, inconsistent tooltip/accessibility metadata, or unsupported-but-recoverable path handling.
- Low: cosmetic defects, nonblocking copy issues, or minor usability inconsistencies.

Priority levels:

- P0: blocks phase advancement or risks source/reviewed text integrity.
- P1: blocks first operational slice.
- P2: should be corrected before broader validation.
- P3: can be deferred with explicit waiting-room entry.

Retest procedure:

- Reproduce with the smallest safe fixture.
- Fix or revise requirement/design if behavior is invalid.
- Re-run the relevant test and adjacent regression group.
- Update RTM evidence when executable tests exist.

Phase rollback triggers:

- Requirement cannot be tested as written.
- Design cannot support required containment or safety behavior.
- Implementation requires changing approved architecture boundaries.
- OCR behavior invalidates OCR-BND-001 assumptions.

---

# 11. Automation and Orchestration Integration

Automated coverage goals:

- Unit tests for schema validation, page ID generation, path handling, review-state transitions, safe report filtering, dirty-state logic, and artifact write/backup behavior.
- Integration tests for PDF inventory, embedded extraction adapter, OCR adapter, dependency reporting, and work-folder validation.
- UI automation/smoke tests for native launch, page/text synchronization, keyboard shortcuts, metadata dialog save/cancel, tooltip/accessibility metadata, and theme switching.
- Packaging smoke tests after package artifacts exist.

Single-command goal:

- Implementation should provide one documented local test command and one documented CI test command once build tooling exists.

Failure gating:

- Source-material safety, source PDF non-modification, work-folder validation, and OCR containment tests are hard gates.
- UI and packaging smoke tests become hard gates once their implementation phase artifacts exist.

---

# 12. Metrics and Reporting

Planning metrics:

- Requirement-to-test mapping coverage: target 100 percent.
- NFR validation mapping coverage: target 100 percent.
- OCR-BND-001 containment mapping: target 100 percent for boundary rows.

Execution metrics, once implementation exists:

- Unit/integration/system pass rate.
- High-severity defect count.
- Source PDF hash-stability result.
- Safe-report leakage checks.
- Work-folder validation pass/fail count.
- OCR candidate availability and diagnostic completeness for representative local tests.
- UI smoke coverage across target platform families.

Pass/fail thresholds:

- Critical safety tests must pass.
- Any source PDF mutation, substantive text leak in generated reports, or unaccepted-page release eligibility is a blocking failure.

---

# 13. Risk Assessment

High-risk Requirement IDs:

- FR-012, FR-013, FR-014, FR-015, FR-016, FR-018, FR-022, FR-023, FR-024, FR-025, FR-031
- NFR-002, NFR-003, NFR-004, NFR-005, NFR-007, NFR-012, NFR-013, NFR-014, NFR-015, NFR-016

Architectural risk concentration points:

- HLA-OCR and OCR-BND-001
- HLA-WORK artifact creation and retention
- HLA-TEXT reviewed text persistence
- HLA-VALID structural validation
- HLA-SAFE source-material controls
- HLA-UI native workflow and accessibility
- HLA-PKG platform/package assumptions

Integration fragility:

- Qt PDF versus Poppler-family PDF behavior.
- Tesseract availability, versioning, performance, and OCR mode variability.
- GitHub Actions Qt version setup.
- Platform-specific path behavior and file modification timestamps.
- User-mounted or cloud-synced filesystem semantics.

Operational risk:

- Manual normalization is expected, so edit preservation and review-state correctness are more important than OCR perfection.
- Representative local PDFs cannot be committed as fixtures, so synthetic fixtures must cover deterministic behavior while local exploratory validation covers corpus-specific risk.

---

# 14. Test Case Inventory

| Test Case ID   | Requirement IDs          | Level               | Validation Focus                                                                                                          |
| -------------- | ------------------------ | ------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| TC-FR-001      | FR-001                   | System/UI           | One local PDF can be selected; no selected PDF leaves editor empty.                                                       |
| TC-FR-002      | FR-002                   | Integration         | Inventory records filename, path, size, modified time, SHA-256, page count, and readability flags.                        |
| TC-FR-003      | FR-003                   | Unit/Integration    | Matching SHA-256 files are reported as duplicates without automatic deletion.                                             |
| TC-FR-004      | FR-004                   | Integration         | Work folder initializes required JSON files, page files, and raw folders using stable volume ID.                          |
| TC-FR-005      | FR-005                   | Unit/Integration    | `volume.json` initializes required and optional metadata fields, including page map.                                      |
| TC-FR-006      | FR-006                   | UI/System           | Metadata edit dialog save persists changes and cancel leaves persisted metadata unchanged.                                |
| TC-FR-007      | FR-007                   | UI/System           | Optional bibliographic fields can be omitted, added, edited, and preserved.                                               |
| TC-FR-008      | FR-008                   | Unit/UI             | Cover metadata can be absent or set to explicit PDF page/page file ID without assuming page 1.                            |
| TC-FR-009      | FR-009                   | Unit/Integration    | Page map contains one stable zero-padded entry per PDF page with PDF page number and text path.                           |
| TC-FR-010      | FR-010                   | UI/System           | Printed page labels can differ from PDF page numbers and persist per page.                                                |
| TC-FR-011      | FR-011                   | Unit/Integration    | One UTF-8 `pages/NNNN.txt` file exists per PDF page, with no metadata front matter.                                       |
| TC-FR-012      | FR-012                   | Integration         | Embedded text candidates are stored separately and failures/counts are recorded in diagnostics.                           |
| TC-FR-013      | FR-013                   | Integration         | OCR candidates are stored separately; OCR failures/counts are recorded; source PDF is unchanged.                          |
| TC-FR-014      | FR-014                   | Integration         | Paper Capture routing schedules OCR for every page by default while preserving embedded candidates.                       |
| TC-FR-015      | FR-015                   | UI/System           | Candidate comparison exposes source selection and restore-from-candidate without external text leakage.                   |
| TC-FR-016      | FR-016                   | UI/System           | Reviewed page text can be edited, saved as UTF-8, and used as search-surrogate text.                                      |
| TC-FR-017      | FR-017                   | Unit/UI             | Review states exist for every page and allow only approved statuses with notes.                                           |
| TC-FR-018      | FR-018                   | Unit/System         | Only accepted pages are release/indexing eligible by default; other statuses are excluded.                                |
| TC-FR-019      | FR-019                   | UI/System           | PDF page changes automatically load matching reviewed text; missing/no selection states are explicit.                     |
| TC-FR-020      | FR-020                   | UI/System           | First/previous/next/last/jump navigation works and does not lose edits.                                                   |
| TC-FR-021      | FR-021                   | UI/System           | Accept, skip, and next-page shortcuts work without bypassing save protections.                                            |
| TC-FR-022      | FR-022                   | Unit/UI             | Dirty state is visible and navigation/close/selection changes save, prompt, or block according to policy.                 |
| TC-FR-023      | FR-023                   | Integration/System  | Bulk overwrite/regeneration creates backup and requires explicit confirmation before destructive changes.                 |
| TC-FR-024      | FR-024                   | Unit/Integration    | `page-quality.json` records selected source, counts, flags, errors, and greater-than-5-percent deltas.                    |
| TC-FR-025      | FR-025                   | Unit/Integration    | Work-folder validation detects JSON errors, volume ID mismatch, missing files, invalid status, and page-count mismatch.   |
| TC-FR-026      | FR-026                   | Unit/System         | Dry-run and validate-only report intended findings without modifying outputs or reviewed text.                            |
| TC-FR-027      | FR-027                   | Unit/UI             | Readiness summary reports status counts, OCR-used count, and validation error count without page text.                    |
| TC-FR-028      | FR-028                   | System/UI           | Primary user workflow launches as native local application, not browser-hosted.                                           |
| TC-FR-029      | FR-029                   | Unit/UI             | Output paths are configurable, local, and not hard-coded to one corpus.                                                   |
| TC-FR-030      | FR-030                   | System              | Cleanup reports planned removals and never removes reviewed text without explicit confirmation.                           |
| TC-FR-031      | FR-031                   | Integration/UI      | Missing or present PDF/OCR tools produce capability/version report and actionable errors.                                 |
| TC-FR-032      | FR-032                   | Unit/System         | External file modification before save is detected and offers reload, overwrite, or cancel.                               |
| TC-FR-033      | FR-033                   | UI/System           | Theme selection persists per user, applies across work folders, and does not alter extraction artifacts.                  |
| TC-FR-034      | FR-034                   | Integration/System  | Derived PDF contains H1/H2-equivalent outline nodes mapped to valid in-document destinations; source PDF unchanged.       |
| TC-FR-035      | FR-035                   | Integration/System  | Derived PDF link annotations support in-document, cross-document, and URL targets; invalid targets are diagnosed safely.  |
| TC-FR-036      | FR-036                   | UI/Integration      | Manual override sidecars for hierarchy/destination/rectangles are persisted and deterministically reapplied on rebuild.   |
| TC-NFR-001     | NFR-001                  | System              | Core workflows operate without required network access when files are locally available.                                  |
| TC-NFR-002     | NFR-002                  | Compliance          | Repository ignore rules and reports prevent source PDFs/full-text derivatives from accidental commit or disclosure.       |
| TC-NFR-003     | NFR-003                  | Reliability         | Source PDF hash remains unchanged after inventory, extraction, OCR, review, and validation.                               |
| TC-NFR-004     | NFR-004                  | Integration         | Diagnostics record source identity, configuration, tool versions, counts, flags, and errors.                              |
| TC-NFR-005     | NFR-005                  | System              | Page-level status, source, flags, and errors are inspectable without reading application internals.                       |
| TC-NFR-006     | NFR-006                  | UI/System           | Sequential review keeps PDF page, text page, review state, and metadata synchronized.                                     |
| TC-NFR-007     | NFR-007                  | Reliability         | Dirty state, backups, and overwrite confirmations protect reviewed text and metadata.                                     |
| TC-NFR-008     | NFR-008                  | Unit/System         | Non-Classic-Traveller synthetic corpus can initialize metadata and output paths without hard-coded assumptions.           |
| TC-NFR-009     | NFR-009                  | UI/System           | Native desktop workflow supports local file access, PDF review, metadata editing, and text editing.                       |
| TC-NFR-010     | NFR-010                  | Integration/System  | Long-running extraction/OCR work exposes progress, diagnostics, and readiness without hidden failures.                    |
| TC-NFR-011     | NFR-011                  | Unit                | Schema versions are present and validation handles known/current schema versions explicitly.                              |
| TC-NFR-012     | NFR-012                  | Compliance          | Error reports and validation reports omit substantive page text.                                                          |
| TC-NFR-013     | NFR-013                  | Platform            | Smoke launch and core workflow validation are planned for each supported OS/architecture target.                          |
| TC-NFR-014     | NFR-014                  | Packaging           | **Phase A:** CPack TGZ/ZIP (`cmake/Packaging.cmake`), `make package`, and CI (build + `core_tests` + `scripts/enrich_lint_manifest.sh` fixture). **Phase B:** AppImage, `.deb`, `.rpm`, `.dmg`, and `.msi` remain planned/deferred per `docs/packaging-plan.md` §4.                           |
| TC-NFR-015     | NFR-015                  | Accessibility       | Core controls have keyboard operation, visible focus, accessible names, and usable target sizing.                         |
| TC-NFR-016     | NFR-016                  | Accessibility/UI    | Light/default, dark or high-contrast-friendly, and sepia themes preserve contrast, focus, tooltip, and status legibility. |
| TC-BND-OCR-001 | FR-013, FR-018           | Boundary            | OCR candidates cannot self-authorize accepted status or release eligibility.                                              |
| TC-BND-OCR-002 | FR-012, FR-013, FR-015   | Boundary            | Embedded and OCR candidates remain raw diagnostic inputs until explicitly selected/restored.                              |
| TC-BND-OCR-003 | FR-014, FR-024           | Boundary            | Paper Capture OCR routing is observable and review-gated.                                                                 |
| TC-BND-OCR-004 | FR-024, NFR-004, NFR-005 | Boundary            | OCR diagnostics record candidate counts, flags, errors, selected source, and tool/version data.                           |
| TC-BND-OCR-005 | FR-024                   | Boundary            | Embedded/OCR delta greater than 5 percent creates suspicious flag.                                                        |
| TC-BND-OCR-006 | NFR-012                  | Boundary/Compliance | OCR and extraction failure reports omit substantive page text.                                                            |

---

# 15. Phase Gate Declaration

Confirm readiness to proceed to Implementation:

- Test Strategy defined? Yes
- Test Plan defined? Yes
- Requirement-to-Test mapping complete? Yes
- NFR validation strategy defined? Yes
- Failure scenario coverage defined? Yes
- Deterministic-probabilistic containment validation defined? Yes
- RTM updated with Test Case IDs? Yes
- Human approval granted? Yes

If any answer is “No,” remain in Test Planning.

---

# Approval

Approved By: Chuck
Role: Project owner
Date: 2026-05-02
Version Incremented: No

Implementation without approved Test Plan closure is prohibited.
