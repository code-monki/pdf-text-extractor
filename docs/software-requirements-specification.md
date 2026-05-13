# Software Requirements Specification (SRS)

Project Name: pdf-text-extractor
Version: 0.1
Date (YYYY-MM-DD): 2026-05-01 (amended 2026-05-04 — Implementation closure scope; see §17)
Author(s): Chuck, Codex
Status: Approved; amended during DD draft
Project Primer Version Reference: `docs/pdf-text-extractor-ideation.md` v0.2
RTM Scaffold Version: `docs/requirements-traceability-matrix.md` v0.1

**Lifecycle position (authoritative):** Gates 1–8 Phase A are closed; **Gate 9 (Documentation closure → release)** is the active open gate; **Packaging Phase B** native installers remain deferred. See `docs/phase-gate-record.md`.

---

# 1. Requirements Authority Declaration

Confirm:

- Ideation phase approved? Yes
- Project Primer stable? Yes, for Requirements drafting
- Scope boundaries defined? Yes
- Advancement to Requirements authorized? Yes

Approval reference:

- `docs/ideation-stabilization-summary.md`
- Approval name: Chuck
- Approval role: Project owner
- Approval date: 2026-05-01

---

# 2. Introduction

## 2.1 Purpose

This SRS defines requirements for `pdf-text-extractor`, a local PDF text extraction, OCR candidate, review, metadata, and validation system.

The intended audience includes:

- project owner
- future architecture and design agents
- implementation agents
- test-planning agents
- downstream consumers such as `/Users/chuck/ct-cd`

This document begins the Requirements phase. It defines what the system must do and what constraints it must satisfy. It does not select implementation architecture, framework, libraries, UI toolkit, or component structure.

## 2.2 Scope

In scope:

- local PDF inventory for one selected PDF at a time
- PDF identity and readability metadata
- embedded text extraction candidate generation
- OCR candidate generation and routing
- reviewed page text workflow
- page-level quality diagnostics
- review-state tracking
- volume metadata management
- printed page-label mapping
- local work-folder validation
- native local application workflow requirements
- protection against accidental overwrite or publication of restricted artifacts
- downstream-ready extraction artifacts

Out of scope unless later requirements explicitly expand scope:

- static browser UI
- browser-based extractor application
- hosted service operation
- database-backed document management
- search index generation
- PDF editing or source PDF modification
- rights-management enforcement
- full electronic-library management platform ownership
- final package/export format for broader library distribution

Ownership boundaries:

- The extractor owns initialization, maintenance, validation, and user editing of extraction work folders.
- Downstream systems consume extraction outputs and must not depend on OCR internals.
- Broader library packaging remains adjacent until packaging/export ownership is explicitly assigned.

---

# 3. System Overview

## 3.1 Problem Summary

Local PDF corpora may contain missing, misleading, badly ordered, OCR-layered, image-only, or otherwise unreliable text. Downstream search, review, and packaging workflows cannot be trusted unless page-level text extraction is inspectable, validated, and correctable.

The originating operational need is reliable page text for Classic Traveller static PDF browsing and search-index work in `/Users/chuck/ct-cd`. The extractor must remain reusable for other local PDF corpora.

The primary stakeholder impact is reducing manual coordination mistakes, exposing extraction failures, supporting human normalization, and preventing accidental publication or commits of copyrighted source PDFs and derived full-text artifacts.

## 3.2 Definitions and Terminology

**Accepted page**: A page whose reviewed text is approved for downstream release/indexing.

**Corpus**: A local collection of related source PDFs and extraction work folders.

**Embedded text**: Text extracted from a PDF text layer without OCR.

**OCR candidate**: Text generated from optical character recognition for a PDF page.

**Page file ID**: Zero-padded PDF page index such as `0001`, used as stable page identity.

**Page map**: Metadata in `volume.json` that associates page file ID, PDF page number, page text file, and printed page label.

**PDF page number**: Physical page position inside the PDF, starting at 1.

**Printed page label**: Page number or label printed in the document itself; may differ from PDF page number.

**Reviewed page text**: Human-editable page text stored in `pages/NNNN.txt`.

**Raw candidate**: Diagnostic embedded or OCR text output retained separately from reviewed page text.

**Release/indexing rule**: Default policy that only `accepted` pages enter downstream release/indexing outputs.

**Volume**: One logical book, document, or PDF-backed work unit.

**Work folder**: Local extraction folder for one volume.

---

# 4. Functional Requirements

## FR-001 Source PDF Selection

**Requirement ID:** FR-001
**Title:** Select one source PDF
**Description:** The system shall allow the user to select one source PDF for inventory, extraction, review, and validation work.
**Acceptance Criteria:**

- Given no selected PDF, no page text is displayed for editing.
- Given a selected PDF, the system displays the selected PDF identity and enables applicable inventory or extraction actions.
- The system does not require the user to select multiple PDFs for the primary review workflow.

**Priority:** High
**Dependencies:** None
**Constraints:** Local files only.
**Notes:** Batch behavior is deferred.

## FR-002 Source PDF Inventory

**Requirement ID:** FR-002
**Title:** Record source PDF inventory metadata
**Description:** The system shall record source PDF identity metadata for the selected PDF.
**Acceptance Criteria:**

- The recorded metadata includes filename, corpus-relative path, file size, modified time, SHA-256 hash, page count, readable/unreadable status, encrypted status, and malformed/damaged status when detectable.
- The system records whether a PDF requires a password when detectable.
- Inventory metadata is available before reviewed page text is accepted.

**Priority:** High
**Dependencies:** FR-001
**Constraints:** Source PDF content must not be modified.
**Notes:** Password entry behavior may be refined later.

## FR-003 Duplicate PDF Detection

**Requirement ID:** FR-003
**Title:** Detect duplicate PDFs by checksum
**Description:** The system shall detect when a selected PDF has the same checksum as another known PDF in the current local context.
**Acceptance Criteria:**

- Given two PDFs with matching SHA-256 hashes, the system reports them as duplicates.
- The system does not automatically delete either duplicate.
- The user can decide whether to keep or remove duplicate source files outside automated deletion.

**Priority:** Medium
**Dependencies:** FR-002
**Constraints:** Deletion automation is out of scope for this requirement.
**Notes:** Corpus-wide duplicate awareness may require future corpus metadata.

## FR-004 Work Folder Initialization

**Requirement ID:** FR-004
**Title:** Initialize one work folder per volume
**Description:** The system shall initialize a local work folder for the selected volume.
**Acceptance Criteria:**

- The work folder contains `volume.json`, `page-quality.json`, `review-state.json`, `pages/`, and raw candidate folders as applicable.
- The work folder uses a stable `volumeId`.
- Initialization does not require committing generated artifacts to git.

**Priority:** High
**Dependencies:** FR-001, FR-002
**Constraints:** Work-folder location must be user-configurable or derived from configured local roots.
**Notes:** Exact directory layout is a requirement contract; implementation storage mechanics remain architectural.

## FR-005 Volume Metadata Initialization

**Requirement ID:** FR-005
**Title:** Initialize volume metadata
**Description:** The system shall initialize `volume.json` with volume-level metadata.
**Acceptance Criteria:**

- `volume.json` includes schema version, stable `volumeId`, title, subtitle, sort title, group, source PDF metadata, indexing intent, notes, and page map.
- `volume.json` supports optional bibliographic fields: publisher, publication year, original publication year, edition, language, ISBN, product code, creators, series, series number, description, subjects, rights, and cover metadata.
- Unknown optional metadata can be null, empty, or omitted according to the finalized schema.

**Priority:** High
**Dependencies:** FR-004
**Constraints:** Metadata fields are initialized by the extractor and may be modified by the user.
**Notes:** Required versus optional fields must be finalized before Requirements approval.

## FR-006 Volume Metadata Editing

**Requirement ID:** FR-006
**Title:** Edit volume metadata
**Description:** The system shall allow the user to edit user-modifiable volume metadata.
**Acceptance Criteria:**

- The metadata pane exposes an edit action.
- The edit action opens a metadata editing dialog or equivalent modal editing flow.
- The edit flow provides explicit save and cancel actions.
- Cancelling leaves persisted metadata unchanged.
- Saving persists modified metadata to the work-folder metadata.

**Priority:** High
**Dependencies:** FR-005
**Constraints:** Read-only diagnostics must be distinguishable from editable metadata.
**Notes:** Exact UI design is deferred.

## FR-007 Optional Metadata Field Management

**Requirement ID:** FR-007
**Title:** Manage optional bibliographic fields
**Description:** The system shall support optional bibliographic metadata without forcing every volume to populate every field.
**Acceptance Criteria:**

- The user can leave optional bibliographic fields blank or unset.
- The metadata edit flow can expose a base field set and additional optional fields.
- Persisted metadata preserves optional fields that the user supplies.

**Priority:** Medium
**Dependencies:** FR-006
**Constraints:** Optional fields must remain compatible with downstream consumers.
**Notes:** Base field set versus optional field set remains to be finalized.

## FR-008 Cover Page Metadata

**Requirement ID:** FR-008
**Title:** Specify cover page metadata
**Description:** The system shall allow cover metadata to identify a user-specified cover page when a useful cover exists.
**Acceptance Criteria:**

- Cover metadata can be absent or empty.
- When supplied, cover metadata can identify a PDF page number and/or page file ID.
- The system does not assume PDF page 1 is always the cover.

**Priority:** Medium
**Dependencies:** FR-005, FR-006
**Constraints:** Bookshelf or visual browse views are not required by this requirement.
**Notes:** Generated cover assets are a packaging/export concern unless later scoped in.

## FR-009 Page Map Initialization

**Requirement ID:** FR-009
**Title:** Initialize page map
**Description:** The system shall initialize `volume.json.pageMap` for every PDF page.
**Acceptance Criteria:**

- Each PDF page has one page-map entry keyed by zero-padded page file ID.
- Each page-map entry includes PDF page number and reviewed page text file path.
- Each page-map entry can store printed page label metadata.
- The page map remains in `volume.json` unless future evidence changes the requirement.

**Priority:** High
**Dependencies:** FR-002, FR-005
**Constraints:** PDF page number remains the stable internal page position.
**Notes:** Splitting page metadata is deferred.

## FR-010 Printed Page Label Editing

**Requirement ID:** FR-010
**Title:** Edit printed page labels
**Description:** The system shall allow the user to enter or correct printed document page labels.
**Acceptance Criteria:**

- The user can set printed page label per PDF page.
- The printed page label can differ from PDF page number.
- Empty or absent printed page labels are valid.
- User-entered printed page labels persist in the work-folder metadata.

**Priority:** High
**Dependencies:** FR-009
**Constraints:** Printed labels are indexing/reference metadata, not stable page identity.
**Notes:** Sort behavior for nonnumeric labels must be finalized.

## FR-011 Reviewed Page Text Files

**Requirement ID:** FR-011
**Title:** Maintain reviewed page text files
**Description:** The system shall maintain one reviewed UTF-8 text file per PDF page.
**Acceptance Criteria:**

- For each PDF page, a corresponding `pages/NNNN.txt` file exists.
- Page text files contain no metadata front matter.
- Empty page files are valid when represented in review state.
- Page file names are zero-padded based on PDF page index.

**Priority:** High
**Dependencies:** FR-004, FR-009
**Constraints:** Page text may contain copyrighted text and must be treated as a local artifact.
**Notes:** Text is a search surrogate, not layout reconstruction.

## FR-012 Embedded Text Candidate Extraction

**Requirement ID:** FR-012
**Title:** Generate embedded text candidates
**Description:** The system shall attempt embedded text extraction when the selected PDF exposes extractable text.
**Acceptance Criteria:**

- Embedded text candidates are stored separately from reviewed page text.
- Embedded extraction failures are recorded in diagnostics.
- Embedded candidate character counts are recorded per page when available.

**Priority:** High
**Dependencies:** FR-002, FR-004
**Constraints:** Source PDFs must not be modified.
**Notes:** Extraction tooling is not selected by this requirement.

## FR-013 OCR Candidate Generation

**Requirement ID:** FR-013
**Title:** Generate OCR candidates
**Description:** The system shall generate OCR candidates for pages requiring OCR according to extraction routing rules.
**Acceptance Criteria:**

- OCR candidates are stored separately from reviewed page text.
- OCR failures are recorded in diagnostics.
- OCR candidate character counts are recorded per page when available.
- OCR use does not modify the source PDF.

**Priority:** High
**Dependencies:** FR-002, FR-004
**Constraints:** OCR output may vary and must be treated as candidate text.
**Notes:** OCR candidate generation shall use **Tesseract** as the engine (`src/core/tesseract_ocr.`*; subprocess/configuration per detailed design). Modes and CLI flags remain design-controlled.

## FR-014 Paper Capture OCR Routing

**Requirement ID:** FR-014
**Title:** OCR all pages for Paper Capture PDFs by default
**Description:** For PDFs identified as Paper Capture or similarly OCR-layered scans, the system shall run OCR on all pages by default as a candidate-generation rule.
**Acceptance Criteria:**

- Given a Paper Capture PDF, OCR candidate generation is scheduled for every page by default.
- Embedded text candidates are still preserved when available.
- The default behavior can be recorded in diagnostics.

**Priority:** Medium
**Dependencies:** FR-012, FR-013
**Constraints:** User override behavior must be defined before approval.
**Notes:** This reflects experiment findings for representative PDFs.

## FR-015 Candidate Comparison

**Requirement ID:** FR-015
**Title:** Compare embedded, OCR, and reviewed text
**Description:** The system shall allow comparison between raw candidates and reviewed page text.
**Acceptance Criteria:**

- The user can inspect whether reviewed text came from embedded, OCR, manual replacement, empty page, or quarantine handling.
- The system records selected source per page.
- The user can restore reviewed text from a raw candidate when available.

**Priority:** High
**Dependencies:** FR-011, FR-012, FR-013
**Constraints:** Candidate comparison must not expose text outside local artifacts.
**Notes:** Exact comparison UI is deferred.

## FR-016 Manual Text Normalization

**Requirement ID:** FR-016
**Title:** Support manual page text normalization
**Description:** The system shall allow the user to edit reviewed page text into readable search-surrogate text.
**Acceptance Criteria:**

- The user can edit `pages/NNNN.txt` content for the selected page.
- The system persists edits according to the selected save policy.
- Tables, captions, map labels, and important discovery terms can be represented as plain text.
- The system does not require visual layout reconstruction.

**Priority:** High
**Dependencies:** FR-011
**Constraints:** Copyrighted text remains local unless explicitly approved.
**Notes:** Manual normalization is expected for most pages in representative corpora.

## FR-017 Review State Tracking

**Requirement ID:** FR-017
**Title:** Track page review state
**Description:** The system shall track review state for every PDF page.
**Acceptance Criteria:**

- Each page has a review-state entry.
- Supported statuses include `raw`, `editing`, `edited`, `accepted`, `skip`, and `quarantined`.
- Each page may store reviewer notes.
- Review-state entries can be validated against allowed statuses.

**Priority:** High
**Dependencies:** FR-004, FR-011
**Constraints:** Status transitions must be defined before approval.
**Notes:** Review state is workflow metadata, not page-label metadata.

## FR-018 Release Indexing Eligibility

**Requirement ID:** FR-018
**Title:** Enforce release/indexing eligibility
**Description:** The system shall distinguish pages eligible for downstream release/indexing from pages not yet accepted.
**Acceptance Criteria:**

- By default, only `accepted` pages are eligible for downstream release/indexing.
- `skip` pages are excluded intentionally.
- `raw`, `editing`, `edited`, and `quarantined` pages are excluded by default.
- Eligibility counts are available for validation or readiness reporting.

**Priority:** High
**Dependencies:** FR-017
**Constraints:** Downstream export behavior is not required unless later scoped in.
**Notes:** This guards against accidental use of unreviewed text.

## FR-019 Page Navigation Synchronization

**Requirement ID:** FR-019
**Title:** Synchronize PDF page and text page
**Description:** The system shall keep displayed PDF page and displayed reviewed text page synchronized.
**Acceptance Criteria:**

- When the selected PDF page changes, the corresponding reviewed text page loads.
- When no PDF page is selected, the text editor is empty.
- The system does not display stale text from a previously selected page after navigation.
- Missing page text displays an explicit missing-text state.

**Priority:** High
**Dependencies:** FR-011
**Constraints:** Exact rendering method is deferred.
**Notes:** This is a core low-friction review requirement.

## FR-020 Page Navigation Controls

**Requirement ID:** FR-020
**Title:** Navigate pages
**Description:** The system shall provide page navigation controls for review.
**Acceptance Criteria:**

- The user can navigate to first, previous, next, and last page.
- The user can jump directly to a PDF page number.
- Navigation cannot silently lose unsaved edits.

**Priority:** High
**Dependencies:** FR-019
**Constraints:** Keyboard shortcuts are covered separately.
**Notes:** Current-page field behavior is a UI detail.

## FR-021 Keyboard Review Shortcuts

**Requirement ID:** FR-021
**Title:** Provide review keyboard shortcuts
**Description:** The system shall support keyboard shortcuts for common page review actions.
**Acceptance Criteria:**

- Shortcuts exist for accepting a page, skipping a page, and moving to the next page.
- Shortcuts do not bypass save/dirty-state protections.
- Shortcut behavior is discoverable through application help, menus, or equivalent native affordance.

**Priority:** Medium
**Dependencies:** FR-017, FR-020
**Constraints:** Exact key bindings are deferred.
**Notes:** Native convention should be considered later.

## FR-022 Save State Protection

**Requirement ID:** FR-022
**Title:** Protect unsaved edits
**Description:** The system shall protect edited page text and metadata from accidental loss.
**Acceptance Criteria:**

- The system tracks whether current page text or metadata has unsaved changes.
- Navigation, selection changes, or closing actions either save safely, prompt, or block until resolved.
- The chosen policy is visible to the user.
- A successful save is distinguishable from an unsaved state.

**Priority:** High
**Dependencies:** FR-006, FR-016, FR-020
**Constraints:** Auto-save is a candidate policy but must include visible state and recovery protections.
**Notes:** Final save policy must be stabilized before approval.

## FR-023 Backup and Restore Protections

**Requirement ID:** FR-023
**Title:** Provide backup and restore protections
**Description:** The system shall protect reviewed text and metadata before destructive regeneration or bulk overwrite.
**Acceptance Criteria:**

- Bulk regeneration does not overwrite reviewed text without explicit confirmation.
- A backup is created before bulk regeneration that could alter reviewed text or metadata.
- The user can restore reviewed text from retained raw candidates when available.
- Destructive cleanup actions require explicit warning.

**Priority:** High
**Dependencies:** FR-011, FR-015, FR-016
**Constraints:** Backup format is deferred.
**Notes:** This directly reduces manual review risk.

## FR-024 Quality Diagnostics

**Requirement ID:** FR-024
**Title:** Record page quality diagnostics
**Description:** The system shall record machine-maintained quality diagnostics per page.
**Acceptance Criteria:**

- `page-quality.json` includes one entry per PDF page.
- Diagnostics include selected source, embedded candidate availability and character count, OCR candidate availability and character count, flags, and errors.
- Diagnostics can flag empty, very short, high-symbol-ratio, high-numeric-ratio, OCR-used, embedded-text-suspicious, possible-reading-order-issue, extraction-error, page-count-mismatch, and unicode-suspect conditions.
- Embedded/OCR character-count deltas greater than 5% can be flagged as suspicious.

**Priority:** High
**Dependencies:** FR-012, FR-013
**Constraints:** Quality scores must not imply false certainty.
**Notes:** Exact scoring formula is deferred.

## FR-025 Work Folder Validation

**Requirement ID:** FR-025
**Title:** Validate work folder
**Description:** The system shall validate work-folder structural consistency.
**Acceptance Criteria:**

- Validation checks JSON parseability for `volume.json`, `page-quality.json`, and `review-state.json`.
- Validation checks matching `volumeId` across JSON files.
- Validation checks that every PDF page has page text, page-map, quality, and review-state entries.
- Validation checks allowed review statuses.
- Validation checks page-count mismatches.
- Validation reports errors without including full page text.

**Priority:** High
**Dependencies:** FR-004, FR-005, FR-009, FR-011, FR-017, FR-024
**Constraints:** Validation must be runnable without re-extraction.
**Notes:** Validate-only mode is covered by FR-026.

## FR-026 Dry-Run and Validate-Only Modes

**Requirement ID:** FR-026
**Title:** Support dry-run and validate-only operations
**Description:** The system shall support non-destructive dry-run and validate-only operations.
**Acceptance Criteria:**

- Dry-run reports intended changes without writing extraction outputs.
- Validate-only checks existing work folders without re-extracting or modifying reviewed text.
- Reports do not include substantive page text.

**Priority:** Medium
**Dependencies:** FR-025
**Constraints:** Exact invocation surface is deferred.
**Notes:** This may be exposed through native UI, internal service behavior, or both.

## FR-027 Readiness Summary

**Requirement ID:** FR-027
**Title:** Summarize volume readiness
**Description:** The system shall summarize extraction and review readiness for a volume.
**Acceptance Criteria:**

- The summary includes counts for raw, editing, edited, accepted, skip, and quarantined pages.
- The summary includes OCR-used count when known.
- The summary includes validation error count.
- The summary can be displayed without exposing full page text.

**Priority:** Medium
**Dependencies:** FR-017, FR-024, FR-025
**Constraints:** Corpus-wide readiness is deferred unless later scoped in.
**Notes:** Useful for review progress.

## FR-028 Native Local Application

**Requirement ID:** FR-028
**Title:** Provide native local application workflow
**Description:** The system shall provide a native local application workflow for the user-facing review experience.
**Acceptance Criteria:**

- The primary user-facing workflow runs locally.
- The primary user-facing workflow is not a browser-based application.
- The workflow supports local file access, PDF review, work-folder artifact management, metadata editing, and page text editing.
- The requirement does not select a native framework.

**Priority:** High
**Dependencies:** FR-001, FR-006, FR-016, FR-019
**Constraints:** Architecture and framework selection are deferred to later phases.
**Notes:** Rationale is reducing friction for local-file-centered review work.

## FR-029 Output Path Configuration

**Requirement ID:** FR-029
**Title:** Configure output paths
**Description:** The system shall allow extraction outputs to be directed to user-configured local paths.
**Acceptance Criteria:**

- The user can identify where work-folder outputs are written.
- Source PDF paths are stored relative to the corpus root when possible.
- The system does not hard-code output paths that prevent reuse across corpora.

**Priority:** High
**Dependencies:** FR-004
**Constraints:** Paths remain local.
**Notes:** Exact configuration UI is deferred.

## FR-030 Local Artifact Cleanup

**Requirement ID:** FR-030
**Title:** Clean up local diagnostic artifacts
**Description:** The system shall provide user-controlled cleanup of retained local diagnostic artifacts.
**Acceptance Criteria:**

- The user can request cleanup of raw candidates or diagnostic artifacts.
- Cleanup does not remove reviewed page text without explicit confirmation.
- Cleanup actions report what will be removed before removal.

**Priority:** Low
**Dependencies:** FR-012, FR-013, FR-023
**Constraints:** No automatic cleanup of raw candidates by default.
**Notes:** Raw candidates are retained by default.

## FR-031 Local Tool Dependency Reporting

**Requirement ID:** FR-031
**Title:** Report local tool dependency availability
**Description:** The system shall report the availability and versions of local PDF and OCR tools required for enabled extraction operations.
**Acceptance Criteria:**

- The system can report whether required local PDF extraction tools are available before running dependent operations.
- The system can report whether required local OCR tools are available before running dependent operations.
- The system records tool names and versions when available in diagnostics.
- Missing required tools produce actionable user-visible errors rather than silent extraction failure.

**Priority:** High
**Dependencies:** FR-012, FR-013, FR-024
**Constraints:** Poppler-family tools and Tesseract are known candidates, but final tool selection is deferred to Architecture.
**Notes:** OCRmyPDF remains reference/experimental unless later requirements assign it product responsibility.

## FR-032 External File Modification Detection

**Requirement ID:** FR-032
**Title:** Detect external work-file modification
**Description:** The system shall reduce accidental overwrite risk when reviewed text or metadata files are modified outside the application.
**Acceptance Criteria:**

- Before saving page text or metadata, the system can detect whether the persisted file changed since it was loaded or last saved by the application.
- If an external change is detected, the system warns the user before overwriting.
- The user can choose an explicit resolution path such as reload, overwrite, or cancel.

**Priority:** Low
**Dependencies:** FR-006, FR-016, FR-022
**Constraints:** This requirement targets accidental single-user external edits, not multi-user collaborative locking.
**Notes:** File locking beyond this behavior is deferred.

## FR-033 UI Theme Selection

**Requirement ID:** FR-033
**Title:** Select application theme
**Description:** The system shall allow the user to select an application visual theme for the native UI.
**Acceptance Criteria:**

- The user can select from available application themes through native UI settings or equivalent preferences.
- The selected theme is persisted as a per-user local application preference and restored when the application is reopened.
- The selected theme applies across different PDF projects/work folders for the same local user unless the user changes it.
- The system includes at least a light/default theme, a dark or high-contrast-friendly theme, and a warm sepia theme.
- Theme changes do not modify extraction artifacts, source PDFs, reviewed page text, or metadata.

**Priority:** Medium
**Dependencies:** FR-028, NFR-015
**Constraints:** Theme support applies to application UI chrome and controls, not source PDF rendering fidelity. Theme preference is not stored in project/work-folder metadata.
**Notes:** Exact theme names, palettes, and OS-theme integration are deferred to Detailed Design.

## FR-034 Derived PDF Outline Enrichment

**Requirement ID:** FR-034
**Title:** Generate derived PDF outline hierarchy
**Description:** The system shall support generating a derived PDF artifact that contains an outline/bookmark hierarchy aligned with the volume table of contents.
**Acceptance Criteria:**

- The enrichment pipeline can create at least two outline levels (H1/H2-equivalent) in a derived PDF.
- Each outline node resolves to a valid in-document destination.
- The source PDF remains unchanged; enriched output is written as a derived artifact.
- The system supports manual hierarchy overrides for PDFs without reliable semantic heading structure.

**Priority:** High
**Dependencies:** FR-006, FR-010, FR-028
**Constraints:** Source PDF bytes must not be modified unless future requirements explicitly authorize in-place mutation.
**Notes:** Heading hierarchy may be inferred and then curated manually.

## FR-035 Derived PDF Link Annotation Enrichment

**Requirement ID:** FR-035
**Title:** Inject link annotations for navigation
**Description:** The system shall support generating link annotations in a derived PDF for in-document, cross-document, and external URL navigation.
**Acceptance Criteria:**

- The enrichment pipeline can write in-document links that navigate to named destinations or page anchors.
- The enrichment pipeline can write cross-document links using package-relative or corpus-relative addressing.
- The enrichment pipeline can write external URL links.
- Invalid targets are reported in diagnostics and skipped safely without corrupting output.

**Priority:** High
**Dependencies:** FR-034, FR-028, FR-031
**Constraints:** Link annotation generation must remain deterministic from supplied mapping inputs.
**Notes:** Cross-document portability rules are finalized during Packaging phase.

## FR-036 Manual Fallback for Outline and Link Geometry

**Requirement ID:** FR-036
**Title:** Provide manual fallback editing inputs
**Description:** The system shall support manual override data for outline hierarchy, destinations, and link rectangle geometry when automatic detection is insufficient.
**Acceptance Criteria:**

- Operators can supply explicit level assignments and destination bindings for outline nodes.
- Operators can supply explicit link rectangle coordinates in PDF page space.
- Override data is saved in local sidecar artifacts and can be re-applied deterministically to regenerate enriched PDFs.
- Manual fallback usage is reflected in diagnostics.

**Priority:** High
**Dependencies:** FR-034, FR-035, FR-032
**Constraints:** Manual override artifacts are local build inputs unless explicitly approved for distribution.
**Notes:** Legacy Paper Capture PDFs are expected to require this fallback in some cases.

---

# 5. Non-Functional Requirements (NFRs)

## NFR-001 Local-Only Operation

**Requirement ID:** NFR-001
**Category:** Compliance
**Description:** The system shall operate through local filesystem paths without requiring application-owned hosted services, network services, remote APIs, or remote databases.
**Measurement Criteria:** Inventory, extraction, review, validation, and metadata editing can be performed with network access unavailable when source PDFs and work folders reside on locally available storage.
**Constraints:** External tools may be local dependencies. User-mounted or OS-presented paths backed by SharePoint, Google Drive, Dropbox, NFS, Samba/SMB, or similar sync/network filesystems may be selected if the operating system exposes them as normal filesystem paths, but consistency, locking, latency, offline availability, and sync-conflict behavior are user/environment responsibilities unless later requirements add explicit support.
**Dependencies:** FR-001 through FR-036

## NFR-002 Source Material Protection

**Requirement ID:** NFR-002
**Category:** Compliance
**Description:** The system shall treat source PDFs and derived full-text artifacts as restricted local artifacts unless explicitly approved otherwise.
**Measurement Criteria:** Generated docs, reports, and diagnostics omit substantive page text; source PDFs and generated work artifacts are covered by repository ignore rules.
**Constraints:** Copyrighted source content must not be reproduced in documentation or responses.
**Dependencies:** FR-011, FR-012, FR-013, FR-024, FR-025, FR-026

## NFR-003 Source PDF Non-Modification

**Requirement ID:** NFR-003
**Category:** Reliability
**Description:** The system shall not modify source PDFs during inventory, extraction, OCR, review, validation, or metadata editing.
**Measurement Criteria:** Source PDF hash remains unchanged after system operations that do not explicitly authorize source modification.
**Constraints:** Source PDF editing is out of scope.
**Dependencies:** FR-002, FR-012, FR-013

## NFR-004 Reproducibility

**Requirement ID:** NFR-004
**Category:** Reproducibility
**Description:** The system shall record enough metadata to make extraction outcomes explainable and repeatable for the same source PDF, configuration, and local tool versions.
**Measurement Criteria:** Diagnostics include source identity, page count, selected source, candidate availability, character counts, flags, and errors.
**Constraints:** Tool-version capture requirements must be finalized before approval.
**Dependencies:** FR-002, FR-024, FR-025

## NFR-005 Auditability

**Requirement ID:** NFR-005
**Category:** Observability
**Description:** The system shall make page-level extraction, OCR, review, and validation state inspectable.
**Measurement Criteria:** For any page, a user or validation process can identify review status, selected source, quality flags, and errors without reading application internals.
**Constraints:** Full page text must not be required in diagnostic reports.
**Dependencies:** FR-017, FR-024, FR-025, FR-027

## NFR-006 Usability For Sequential Review

**Requirement ID:** NFR-006
**Category:** Usability
**Description:** The system shall support low-friction sequential page review.
**Measurement Criteria:** User can navigate page-to-page while PDF page, reviewed text page, review state, and page metadata remain synchronized.
**Constraints:** Exact UI layout is deferred.
**Dependencies:** FR-019, FR-020, FR-022

## NFR-007 Data Integrity

**Requirement ID:** NFR-007
**Category:** Reliability
**Description:** The system shall protect reviewed text and user-edited metadata from accidental overwrite or loss.
**Measurement Criteria:** Destructive or bulk overwrite actions require confirmation and backup; unsaved edits cannot be silently lost.
**Constraints:** Backup mechanism is deferred.
**Dependencies:** FR-022, FR-023

## NFR-008 Corpus Agnosticism

**Requirement ID:** NFR-008
**Category:** Maintainability
**Description:** The system shall avoid hard-coding Classic Traveller-specific behavior into reusable extraction requirements.
**Measurement Criteria:** Volume metadata, output paths, and corpus identity can be configured for another local PDF corpus.
**Constraints:** Classic Traveller remains the initial validation corpus.
**Dependencies:** FR-005, FR-029

## NFR-009 Native Local UX

**Requirement ID:** NFR-009
**Category:** Usability
**Description:** The primary user-facing workflow shall be a native local application experience rather than a browser-based application.
**Measurement Criteria:** Requirements and later architecture preserve native local workflow expectations without requiring a browser-hosted app.
**Constraints:** Native framework selection is deferred.
**Dependencies:** FR-028

## NFR-010 Performance Observability

**Requirement ID:** NFR-010
**Category:** Performance
**Description:** The system shall expose enough progress and duration information for long-running extraction or OCR operations to be observable.
**Measurement Criteria:** Long-running operations report progress, completion, failure, and summary timing when available.
**Constraints:** Final performance budgets are deferred until implementation approach is known.
**Dependencies:** FR-013, FR-024, FR-027

## NFR-011 Schema Evolvability

**Requirement ID:** NFR-011
**Category:** Maintainability
**Description:** Structured output artifacts shall include schema version information where applicable.
**Measurement Criteria:** `volume.json`, `page-quality.json`, and `review-state.json` include schema version fields.
**Constraints:** Final schema versioning policy is deferred.
**Dependencies:** FR-005, FR-017, FR-024

## NFR-012 Error Report Safety

**Requirement ID:** NFR-012
**Category:** Security
**Description:** Reports, logs, and validation outputs shall avoid including substantive source PDF text or reviewed page text.
**Measurement Criteria:** Validation and diagnostic reports use filenames, page IDs, counts, hashes, flags, statuses, and errors rather than full page content.
**Constraints:** Developer debugging must follow copyrighted-source handling rules.
**Dependencies:** FR-024, FR-025, FR-026, FR-027

## NFR-013 Platform Support

**Requirement ID:** NFR-013
**Category:** Portability
**Description:** The native local application shall support Linux, macOS, and Windows target platforms at the minimum versions and architectures defined in the platform support matrix.
**Measurement Criteria:** A release candidate can be installed or launched and can perform core local workflow operations on each target platform and architecture in the platform support matrix.
**Implementation measurement (Gate 7 — Implementation):** The codebase shall be structured for those targets (Qt 6 Widgets shell optional build, CMake, portable core library). **Primary development validation** on at least one host OS/architecture shall be recorded with RTM evidence (build + automated tests). This satisfies Implementation-phase portability obligations before installable release artifacts exist.
**Packaging measurement (Gate 8 — Packaging):** Full-matrix **install/launch smoke** and packaged artifact validation per row of the platform matrix occur when release packaging exists; see NFR-014.
**Constraints:** The platform support matrix is a project target requirement supplied by the project owner; exact packaging mechanisms are deferred to Packaging.
**Dependencies:** FR-028, FR-029

## NFR-014 Package Artifact Targets

**Requirement ID:** NFR-014
**Category:** Packaging
**Description:** The system shall target native installer/package artifact formats appropriate to each supported operating system family.
**Measurement Criteria:** Release packaging plans account for AppImage, `.deb`, and `.rpm` on Linux; `.dmg` on macOS; and `.msi` on Windows.
**Constraints:** Exact packaging tools, signing, notarization, update strategy, and installer implementation are deferred to Packaging and Architecture phases.
**Dependencies:** NFR-013

## NFR-015 Accessibility and Basic Usability

**Requirement ID:** NFR-015
**Category:** Usability
**Description:** The native user interface shall provide accessible basic interaction behavior, using WCAG 2.2 AA as an applicability benchmark where native desktop conventions allow.
**Measurement Criteria:** Core workflows support keyboard operation, visible focus, focus not being obscured, readable text scaling, sufficient text and non-text contrast, labeled inputs, identifiable validation errors, non-drag alternatives for drag-like actions, and reasonable pointer target sizing.
**Implementation measurement (Gate 7 — Implementation):** Core workflows use **Qt Widgets** standard controls (`pte_shell`), documented shortcuts and actions (`docs/shell-user-guide.md`), and built-in themes with contrast sanity checks (**FR-033 / NFR-016**, `app_theme`). This documents the baseline native-desktop UX slice required before packaging-level audits.
**Deferred measurement:** Formal evidence against **every** WCAG-oriented bullet above on **each** supported platform (including assistive technology matrix testing) is **out of scope for Gate 7** unless separately scheduled; track under Packaging / hardening or a future usability milestone.
**Constraints:** WCAG is web-focused; conformance language must be adapted to native desktop UI conventions and platform accessibility APIs.
**Dependencies:** FR-006, FR-019, FR-020, FR-021, FR-028

## NFR-016 Theme Accessibility

**Requirement ID:** NFR-016
**Category:** Usability
**Description:** Application themes shall preserve readable contrast, visible focus, understandable status states, and usable diagnostics presentation.
**Measurement Criteria:** Each built-in theme preserves sufficient text/non-text contrast, visible focus indicators, readable diagnostics/status indicators, and tooltip readability for core workflows.
**Constraints:** Theme customization must not make source PDF rendering appear modified or imply changes to source content.
**Dependencies:** FR-033, NFR-015

---

# 6. Deterministic-Probabilistic Requirements

OCR is a probabilistic or quality-variable subsystem. Requirements for this boundary:

- OCR output is candidate text and shall not be treated as trusted final text without review.
- Embedded text output may be deterministic from a tool perspective but shall still be treated as unreviewed candidate text until accepted.
- The system shall preserve raw candidates separately from reviewed text.
- The system shall expose OCR failures and suspicious quality signals.
- The system shall allow human review to override machine-generated candidates.
- The system shall not silently release/index raw, editing, edited, or quarantined pages.
- OCR confidence, if recorded, shall be treated as one diagnostic signal, not sole proof of correctness.

Related requirements:

- FR-012
- FR-013
- FR-014
- FR-015
- FR-016
- FR-017
- FR-018
- FR-024
- NFR-004
- NFR-005
- NFR-012
- NFR-015
- NFR-016

---

# 7. Constraints

- The system shall run locally.
- Local storage support means paths accessible through the operating system filesystem API. The application does not provide, require, or manage its own network protocol, remote document service, sync service, or cloud storage integration.
- User-mounted or cloud-synced paths may work when presented as filesystem paths, but they are treated as user-managed storage with possible consistency and conflict risks.
- The primary user-facing product shall be a native local application, not a browser-based application.
- The native application shall support the following minimum target platforms:


| Operating System/Distro | Architectures    | Minimum Version |
| ----------------------- | ---------------- | --------------- |
| Linux/Fedora            | X86_64 / AArch64 | 44              |
| Linux/Ubuntu            | X86_64 / AArch64 | 26.04 LTS       |
| macOS                   | X86_64 / AArch64 | 26.4            |
| Windows                 | X86_64           | 11              |


- The system shall not require a hosted service, remote database, remote API, or application-managed network service for core operation.
- Source PDFs shall not be modified.
- Source PDFs and derived full-text artifacts shall remain local unless explicitly approved otherwise.
- Output destinations shall remain user-configurable.
- Source PDF paths shall be recorded relative to the corpus root when possible.
- The system shall not hard-code one corpus' metadata into reusable behavior.
- **Resolved for Implementation (Gate 7):** Presentation uses **Qt 6 Widgets** (`pte_shell`). PDF inspection and embedded text use **Poppler-family** utilities. OCR candidate generation uses **Tesseract**. Further packaging and installer choices remain **Packaging (Gate 8)**.
- Requirements must remain traceable to architecture, design, implementation, and tests.

---

# 8. Assumptions

- The primary operator is a local user reviewing and correcting one PDF at a time.
- Many representative PDFs will require OCR candidate generation and manual normalization.
- Printed page labels often cannot be reliably inferred automatically.
- Raw candidates are useful for recovery and diagnostics and are retained by default.
- Optional bibliographic metadata may vary widely by corpus and volume.
- A broader reusable library package format may be needed later, but ownership is not yet assigned to this project.
- **Product OCR path:** Tesseract for OCR candidates (see FR-013). OCRmyPDF remains an optional reference/experiment unless later scoped.
- Native local application expectation is driven by local-file workflow friction reduction.

---

# 9. Interfaces and External Dependencies

User interfaces:

- native local review workflow
- metadata editing workflow
- page navigation workflow
- validation/readiness reporting workflow

Data exchanges:

- source PDFs as local input
- `volume.json`
- `page-quality.json`
- `review-state.json`
- reviewed page text files
- raw embedded text candidates
- raw OCR candidates
- validation and diagnostic reports

External dependencies:

- local PDF parsing/extraction capabilities
- local OCR capabilities
- local filesystem access
- downstream consumers such as `/Users/chuck/ct-cd`

Known candidate local tools:

- Poppler-family PDF utilities for PDF inspection, rendering, or embedded text extraction
- Tesseract OCR for OCR candidate generation
- OCRmyPDF as a reference or preprocessing candidate unless later scoped differently

No API, framework, or component architecture is selected by this section.

---

# 10. Data Requirements

Data types:

- source PDF identity metadata
- volume metadata
- page-map metadata
- reviewed page text
- raw candidate text
- page quality diagnostics
- review state
- validation reports
- optional package/export metadata

Persistence expectations:

- Work-folder artifacts persist locally.
- Reviewed page text persists as one UTF-8 text file per PDF page.
- Raw candidates persist locally by default.
- User-edited metadata persists in structured JSON.

Retention expectations:

- Source PDFs are retained outside system modification.
- Raw candidates are retained until user-controlled cleanup.
- Reviewed text is retained unless explicitly deleted by the user.

Compliance constraints:

- Source PDFs and derived full-text artifacts are local-only unless explicitly approved.
- Documentation and reports avoid full source/reviewed text reproduction.

Audit expectations:

- Diagnostics preserve enough metadata to explain extraction status and review readiness.
- Validation reports identify structural problems without full page text.

---

# 11. Error Handling and Edge Conditions

Expected failure conditions:

- unreadable PDF
- encrypted or password-protected PDF
- malformed or damaged PDF
- page-count mismatch
- missing page text file
- missing page-map entry
- missing review-state entry
- missing page-quality entry
- embedded extraction failure
- OCR failure
- suspicious embedded/OCR delta
- invalid review status
- failed metadata save
- unsaved edit navigation conflict
- external modification of reviewed page text or metadata while loaded in the application

Expected behavior:

- Errors are visible to the user.
- Errors are recorded in diagnostics or validation output.
- Failures do not silently produce accepted pages.
- Missing page files do not display stale text.
- The system avoids destructive recovery actions without explicit user confirmation.

---

# 12. Security and Compliance Requirements

- Authentication is not required for the current local single-user application workflow.
- Authentication should remain out of scope unless future requirements introduce multi-user operation, shared libraries, remote access, or hosted services.
- The system shall not publish, upload, or transmit source PDFs or derived text as part of core operation.
- Repository ignore coverage shall protect source PDFs, work folders, OCR caches, extracted text, raw candidates, diagnostics, package outputs, generated indexes, editor files, and OS noise.
- Reports shall avoid substantive PDF or reviewed page text.
- Cleanup, regeneration, and overwrite actions shall require explicit warnings when reviewed text or metadata may be affected.
- Source PDFs and derived text shall not be treated as training data.

---

# 13. Waiting Room

Deferred items:

- final native UI framework selection
- architecture/component decomposition
- final OCR candidate scoring algorithm
- alternate OCR engine evaluation beyond current evidence
- broader library-system package/export format
- corpus-wide manifest
- bookshelf or visual browse view
- generated cover asset workflow
- search index generation
- static browser UI
- hosted or database-backed document management
- automated duplicate deletion
- batch corpus extraction
- package distribution workflows
- authentication or authorization for multi-user, shared-library, remote, or hosted operation

---

# 14. Risk Assessment


| Risk ID | Risk                                                          | Classification | Mitigation Direction                                                               |
| ------- | ------------------------------------------------------------- | -------------- | ---------------------------------------------------------------------------------- |
| `R-001` | OCR output may be plausible but wrong.                        | High           | Treat OCR as candidate text, require review, preserve diagnostics.                 |
| `R-002` | Embedded text may be present but badly ordered or misleading. | High           | Preserve embedded candidate separately, compare candidates, flag suspicious pages. |
| `R-003` | Derived full text may be accidentally committed or exposed.   | High           | Maintain ignore rules, local-only artifact policy, safe reports.                   |
| `R-004` | Manual review workload may be high.                           | Moderate       | Native synchronized review workflow, keyboard shortcuts, readiness summaries.      |
| `R-005` | Native UI expectations may over-constrain architecture.       | Moderate       | Requirements define workflow, not framework; HLA preserves engine boundaries.      |
| `R-006` | Broader library packaging could expand extractor scope.       | Moderate       | Keep packaging/export ownership explicit and defer full library scope.             |
| `R-007` | Metadata model may grow too broad.                            | Moderate       | Base/optional metadata model, schema versioning, user-editable fields.             |
| `R-008` | OCR tool dependencies may reduce reproducibility.             | Moderate       | Capture diagnostics and tool metadata requirements; defer tool finalization.       |
| `R-009` | Page identity and printed labels may be conflated.            | Moderate       | Use PDF page index/page file ID as stable identity; printed label is metadata.     |
| `R-010` | Requirement set may still include design bias.                | Low            | Review before approval; defer architecture and framework choices.                  |


---

# 15. Traceability Readiness Declaration

Confirm:

- All requirements uniquely identified: Yes
- Acceptance criteria defined: Yes
- No architectural decisions embedded: Yes, to be reviewed
- No implementation bias present: Yes, to be reviewed
- Waiting room separated: Yes
- Probabilistic requirements bounded: Yes
- RTM scaffold updated: Yes

Traceability reference:

- `docs/requirements-traceability-matrix.md` v0.1

---

# 16. Phase Gate Declaration

Confirm readiness to proceed to Architecture:

- Requirements stable? Yes
- NFRs measurable? Yes
- Scope boundaries explicit? Yes
- Traceability scaffold prepared? Yes
- Human approval granted? Yes

Requirements phase is approved. High-Level Architecture may begin as the next lifecycle phase.

---

# 17. Implementation closure amendments (Gate 7)

**Effective:** 2026-05-04  
**Authority:** Project owner amendment under maintained SRS traceability (RTM updated same date).

This section records **scope explicitly satisfied at Implementation exit** versus work deferred to **Packaging (Gate 8)** or later hardening:


| Topic             | Gate 7 (Implementation)                                                          | Deferred                                               |
| ----------------- | -------------------------------------------------------------------------------- | ------------------------------------------------------ |
| OCR engine        | **Tesseract** for OCR candidates (FR-013); Poppler family for PDF/embed/raster   | —                                                      |
| UI toolkit        | Qt 6 Widgets shell (`pte_shell`)                                                 | Installer-branded UX                                   |
| **NFR-013**       | Portable codebase + recorded primary validation + automated tests (RTM evidence) | Full-matrix install/launch smoke on packaged artifacts |
| **NFR-015**       | Qt Widgets baseline, shortcuts/docs, themes + NFR-016 contrast hints             | Full WCAG-oriented audit per OS / AT matrix            |
| Packaging formats | —                                                                                | NFR-014; artifact validation (test plan)               |


Downstream artifacts: `docs/requirements-traceability-matrix.md` (§3, §9), `docs/phase-gate-record.md`.

---

# Approval

Approved By: Chuck
Role: Project Owner
Date: 2026-05-01
Version Incremented: No