# High-Level Architecture (HLA)

Project Name: pdf-text-extractor
Version: 0.1
Date (YYYY-MM-DD): 2026-05-01
Author(s): Chuck, Codex
Status: Approved; amended during DD draft
Requirement Version Reference: `docs/software-requirements-specification.md` v0.1
RTM Version Reference: `docs/requirements-traceability-matrix.md` v0.1

---

# 1. Architectural Authority Declaration

Confirm:

- Requirements phase approved? Yes
- Requirement IDs stable? Yes
- NFRs defined and measurable? Yes
- Advancement to Architecture authorized? Yes

Architecture is approved by the project owner. This HLA authorizes Detailed Design work, but does not authorize implementation, test planning, or package build work.

---

# 2. Architecture Overview

## 2.1 System Purpose

`pdf-text-extractor` is a local native application for inventorying one selected PDF, generating embedded-text and OCR candidates, supporting synchronized human review of page text, maintaining volume/page metadata, validating local work-folder artifacts, and producing downstream-ready extraction artifacts.

The architecture supports:

- one-PDF-at-a-time extraction and review
- local-only copyrighted source material handling
- deterministic work-folder contracts
- explicit containment of OCR as a probabilistic subsystem
- native review UI workflow without coupling extraction logic to UI presentation
- reproducible local tool dependency reporting
- cross-platform package targets

Primary requirement alignment: FR-001 through FR-033 and NFR-001 through NFR-016.

## 2.2 Architectural Drivers

Key functional drivers:

- FR-001 through FR-003: source PDF selection, inventory, duplicate detection
- FR-004 through FR-011: work-folder, metadata, page map, and reviewed page file contracts
- FR-012 through FR-016: embedded extraction, OCR candidates, comparison, and normalization
- FR-017 through FR-023: review state, navigation, save state, backup, and restore
- FR-024 through FR-027: diagnostics, validation, dry-run, and readiness summary
- FR-028 through FR-033: native app, output configuration, cleanup, dependency reporting, external modification detection, and theme selection

Key non-functional drivers:

- NFR-001 through NFR-003: local-only operation, source material protection, source PDF non-modification
- NFR-004 through NFR-005: reproducibility and auditability
- NFR-006 through NFR-009: sequential review usability, integrity, corpus agnosticism, native UX
- NFR-010 through NFR-012: observability, schema evolvability, safe reporting
- NFR-013 through NFR-016: platform support, package targets, accessibility/basic usability, and theme accessibility

Constraints:

- No hosted service, database requirement, or browser-hosted application.
- No source PDF modification.
- No source PDFs or full-text derivatives committed or published unless explicitly approved.
- Poppler-family tools, Tesseract, and OCRmyPDF are candidates or experimental references, not finalized dependencies.
- Native UI framework selection is deferred until architecture review or a focused technical spike authorizes it.

---

# 3. System Context and Boundaries

## 3.1 External Systems and Interfaces

External actors and systems:

- Local user: selects source PDFs, reviews, edits, and amends page text, edits metadata, accepts/skips/quarantines pages, triggers validation and cleanup.
- Local file system: stores source PDFs, work folders, page text, raw candidates, diagnostics, backups, configuration, and package artifacts.
- PDF rendering/inspection tools: provide PDF metadata, page count, rendering, or embedded text extraction through an adapter boundary.
- OCR engine: generates OCR candidates through an explicit probabilistic boundary.
- Downstream consumers: read approved work-folder artifacts; they do not depend on OCR internals.
- Operating system platform services: file dialogs, native accessibility APIs, process execution, file modification timestamps, and package runtime behavior.

Trust boundaries:

- Source PDFs and derived text remain local, restricted artifacts.
- External tool execution is outside direct application control and must be isolated behind capability and adapter components.
- Downstream consumers receive explicit artifacts and safe summaries, not hidden application state.

## 3.2 System Scope Boundaries

In scope:

- Native local review application shell.
- Work-folder artifact ownership and validation.
- Source PDF inventory and non-mutating inspection.
- Embedded-text candidate extraction.
- OCR candidate routing and generation.
- Human-reviewed text and metadata editing workflow.
- Diagnostics, quality flags, readiness summaries, and safe reports.
- Local output path configuration and cleanup workflow.
- Runtime capability/dependency reporting.

Out of scope:

- Search index generation.
- Browser/search UI for downstream browsing.
- Full electronic-library management platform.
- Hosted, remote, multi-user, or application-managed cloud/sync operation.
- Authentication and authorization.
- Rights enforcement beyond local source/derived artifact protections.
- Final reusable library package/export format unless future requirements assign ownership.

---

# 4. Deterministic-Probabilistic Boundary Model

OCR is the only currently identified probabilistic subsystem.

Boundary ID: OCR-BND-001

Boundary declaration:

- The OCR subsystem receives page images or equivalent page render inputs and returns OCR text candidates plus optional confidence/diagnostic data.
- OCR output is never authoritative by itself.
- OCR output is stored as a raw candidate and may be copied into reviewed page text only through explicit system workflow or human editing.
- The reviewed `pages/NNNN.txt` file and review status remain the release/indexing authority.

Invocation contract:

- Inputs: source PDF reference, PDF page number, rendering/extraction parameters, output path context, and OCR mode policy.
- Outputs: raw OCR candidate text, candidate metadata, quality indicators, errors, and page-level diagnostics.
- Side effects: writes only to local work-folder raw/diagnostic artifacts or reviewed text when an authorized replace action occurs.

Validation harness strategy:

- Compare OCR candidates against page coverage, character counts, quality flags, optional confidence data, and human ground-truth samples when available.
- Preserve enough tool/version/configuration metadata to reproduce OCR candidate generation.
- Validate that OCR results do not enter release outputs unless page status is `accepted`.

Containment logic:

- OCR adapters cannot directly update release eligibility.
- OCR errors produce diagnostics and review-state implications; they do not silently fail open.
- Low-confidence, empty, suspicious, or errored OCR pages remain review-gated.

Fallback mechanisms:

- Embedded text candidate may be used when acceptable.
- Manual text remains possible when both embedded text and OCR are unsuitable.
- Pages may be marked `skip` or `quarantined` when output cannot be trusted.

Observability strategy:

- Page-level selected source, candidate availability, character counts, quality flags, and errors flow into `page-quality.json`.
- Readiness summaries aggregate raw, edited, accepted, skipped, quarantined, and OCR-used pages without reproducing full copyrighted text.

Reproducibility posture:

- OCR engine identity, version, invocation profile, and relevant runtime capability data must be reportable.
- Exact tool and mode selection remains a later architectural or detailed design decision.

---

# 5. Architectural Style and Structural Model

Selected architectural pattern: modular local application with layered dependency direction.

Structural layers:

- Presentation layer: native review UI and platform interaction surfaces.
- Application layer: workflow coordination, commands, state transitions, and use-case orchestration.
- Domain layer: volume, page, artifact, review-state, validation, and diagnostic concepts.
- Adapter layer: PDF inspection/rendering, embedded text extraction, OCR, file-system persistence, dependency detection, and platform packaging surfaces.

Dependency direction rules:

- Presentation depends on application interfaces, not extraction internals.
- Application coordinates domain services and adapters through explicit ports.
- Domain model does not depend on UI toolkit, OCR engine, PDF tool, package format, or downstream consumer.
- External tools are accessed only through adapters.
- Downstream integration depends on artifact contracts, not in-memory UI or engine state.

Rationale:

- The native UI is in scope, but extraction, validation, backup, and overwrite protection must be testable without driving the UI.
- OCR is probabilistic and must be isolated from deterministic reviewed artifacts.
- Local-only file handling and source-material protection require clear artifact ownership and egress controls.

Trade-offs:

- More component boundaries than a simple script, but lower risk of UI/extraction coupling.
- Adapter boundaries may add early design work, but they preserve future OCR/PDF tool substitution.
- A database-free architecture fits current requirements but may require re-evaluation if future library management scope is assigned.

Alternatives considered:

- Single monolithic GUI with embedded extraction logic: rejected because it weakens testability and boundary discipline.
- CLI-first product: rejected because the approved user-facing workflow is native GUI-centered.
- Browser-hosted app: rejected by NFR-009.
- Tool-first architecture around a selected OCR/PDF stack: rejected because final dependency selection is deferred.

---

# 6. Major Components

## HLA-UI Native Reviewer UI

Responsibility: Present the local review workflow, including PDF/page view, synchronized text editor, metadata pane/dialog, status controls, navigation, diagnostics display, keyboard shortcuts, accessible controls, theme-aware UI presentation, and save-state indicators.

Related requirements: FR-001, FR-006, FR-007, FR-010, FR-015, FR-016, FR-017, FR-019, FR-020, FR-021, FR-022, FR-027, FR-028, FR-033, NFR-006, NFR-009, NFR-015, NFR-016

Key interfaces: application workflow commands, read models for current volume/page, validation/readiness summaries, platform UI services, theme preferences.

Dependency constraints: Must not call OCR, PDF tools, or file persistence directly except through application interfaces.

Data ownership: Owns transient presentation state only.

## HLA-THEME Theme and Appearance Service

Responsibility: Manage built-in application themes, per-user local theme preference persistence, theme application to native UI controls, and theme accessibility validation.

Related requirements: FR-033, NFR-009, NFR-015, NFR-016

Key interfaces: UI theme selection controls, configuration service, platform appearance signals where available, accessibility validation.

Dependency constraints: Must not modify source PDF rendering, extraction artifacts, reviewed page text, raw candidates, or metadata. Must not make theme choice part of downstream extraction output or project/work-folder metadata.

Data ownership: Owns per-user local UI appearance preference and built-in theme definitions.

## HLA-APP Application Workflow Coordinator

Responsibility: Coordinate user commands and system workflows for selection, inventory, extraction, review state changes, validation, cleanup, dry-run, backup, restore, and readiness reporting.

Related requirements: FR-001, FR-004, FR-017, FR-018, FR-022, FR-023, FR-026, FR-027, FR-030, FR-032, NFR-005, NFR-007, NFR-010

Key interfaces: UI command surface, domain services, adapter ports, validation results, event/progress reporting.

Dependency constraints: Coordinates components but does not implement OCR, PDF parsing, or UI rendering.

Data ownership: Owns workflow transaction boundaries and command outcomes.

## HLA-INV Source Inventory Service

Responsibility: Inspect selected source PDF identity, file hash, size, modified time, page count, readability, duplicate signals, encryption/damage/password/malformed status, and basic content/readability indicators.

Related requirements: FR-001, FR-002, FR-003, NFR-001, NFR-003, NFR-004, NFR-008

Key interfaces: file-system adapter, PDF inspection adapter, work-folder initialization, diagnostics.

Dependency constraints: Must not modify source PDFs.

Data ownership: Owns inventory result structures and duplicate detection evidence.

## HLA-WORK Work Folder Artifact Manager

Responsibility: Create, maintain, write, validate-safe paths for, and clean local work-folder artifacts, including `volume.json`, `page-quality.json`, `review-state.json`, `pages/`, `raw/`, backups, and configured output roots.

Related requirements: FR-004, FR-011, FR-023, FR-029, FR-030, FR-032, NFR-001, NFR-002, NFR-007, NFR-011

Key interfaces: file-system adapter, schema/version model, workflow coordinator, validation service.

Dependency constraints: Must enforce configured roots and local artifact classifications.

Data ownership: Owns local work-folder artifact lifecycle.

## HLA-META Volume Metadata and Page Map Service

Responsibility: Initialize and edit volume-level metadata, optional bibliographic fields, cover page metadata, and page map associations for PDF page numbers, page file IDs, text file paths, and printed page labels.

Related requirements: FR-005, FR-006, FR-007, FR-008, FR-009, FR-010, NFR-005, NFR-008, NFR-011

Key interfaces: metadata edit workflows, work-folder artifact manager, validation service.

Dependency constraints: Metadata structure must remain corpus-agnostic and schema-versioned.

Data ownership: Owns `volume.json` metadata model and page map semantics.

## HLA-PDF PDF View and Page Synchronization Adapter

Responsibility: Provide non-mutating PDF page access for display, page navigation synchronization, page count validation, and page-image rendering needed by OCR or preview workflows.

Related requirements: FR-001, FR-014, FR-019, FR-020, NFR-003, NFR-006, NFR-009

Key interfaces: UI page display surface, OCR boundary input rendering, inventory service.

Dependency constraints: Must not make the domain model depend on a specific PDF rendering library or UI toolkit.

Data ownership: Owns no persistent data beyond transient render/cache state.

## HLA-EXT Embedded Text Extraction Adapter

Responsibility: Generate raw embedded-text candidates per page when a text layer is available, report extraction counts/errors, and isolate raw candidates from reviewed text.

Related requirements: FR-012, FR-015, FR-024, NFR-004, NFR-005, NFR-010

Key interfaces: PDF inspection/extraction tool port, candidate storage, diagnostic service.

Dependency constraints: Must not mark pages accepted or release-eligible.

Data ownership: Owns raw embedded candidate output until stored by HLA-WORK.

## HLA-OCR OCR Pipeline Adapter

Responsibility: Route pages requiring OCR, generate raw OCR candidates, capture OCR diagnostics, and preserve OCR as a probabilistic subsystem behind OCR-BND-001.

Related requirements: FR-013, FR-014, FR-015, FR-024, FR-031, NFR-004, NFR-005, NFR-010, NFR-012

Key interfaces: page render input, OCR engine process adapter, candidate storage, diagnostic service, dependency capability service.

Dependency constraints: OCR engine choice and mode policy remain replaceable behind the adapter.

Data ownership: Owns raw OCR candidate output until stored by HLA-WORK.

## HLA-TEXT Reviewed Text and Normalization Service

Responsibility: Maintain reviewed page text files, support manual normalization, candidate comparison/replacement, Unicode/control-character handling, and search-surrogate text semantics.

Related requirements: FR-011, FR-015, FR-016, FR-022, FR-023, NFR-007, NFR-011, NFR-012

Key interfaces: UI text editor state, work-folder artifact manager, backup/restore workflow, validation service.

Dependency constraints: Reviewed text is separate from raw candidate storage.

Data ownership: Owns `pages/NNNN.txt` semantics and normalization outcomes.

## HLA-REVIEW Review State and Release Gate Service

Responsibility: Manage page review status, reviewer notes, selected source, accepted/skip/quarantined semantics, release/indexing eligibility, and status-driven workflow filters.

Related requirements: FR-017, FR-018, FR-021, FR-022, FR-027, NFR-005, NFR-007

Key interfaces: UI status controls, readiness summary, work-folder artifact manager, validation service.

Dependency constraints: Release eligibility must default to accepted pages only.

Data ownership: Owns `review-state.json` status model and release gate policy.

## HLA-DIAG Diagnostics and Readiness Service

Responsibility: Generate and aggregate page-level quality diagnostics, suspicious deltas, quality flags, counts, safe reports, and readiness summaries.

Related requirements: FR-002, FR-024, FR-027, FR-031, NFR-004, NFR-005, NFR-010, NFR-012

Key interfaces: extraction adapters, OCR adapter, inventory service, validation service, UI diagnostics view.

Dependency constraints: Reports must not include substantive copyrighted text.

Data ownership: Owns `page-quality.json` semantics and derived safe summaries.

## HLA-VALID Validation and Dry-Run Service

Responsibility: Validate JSON, schema versions, page counts, page files, quality/review entries, stable page IDs, statuses, output roots, release eligibility, and dry-run/validate-only outcomes.

Related requirements: FR-025, FR-026, FR-027, NFR-004, NFR-005, NFR-007, NFR-011, NFR-012

Key interfaces: work-folder artifact manager, metadata service, review state service, diagnostics service, UI/reporting surfaces.

Dependency constraints: Validation must be runnable without UI interaction.

Data ownership: Owns validation result structures and safe validation reports.

## HLA-CONFIG Configuration and Output Policy Service

Responsibility: Manage configured source/output roots, user-directed artifact destinations, local-only policy, cleanup policy, and corpus-agnostic configuration.

Related requirements: FR-029, FR-030, NFR-001, NFR-002, NFR-008, NFR-011

Key interfaces: application workflow coordinator, work-folder artifact manager, package/runtime configuration.

Dependency constraints: Must not hard-code Classic Traveller or downstream project paths.

Data ownership: Owns application configuration and output policy state.

## HLA-DEP Dependency Capability Service

Responsibility: Detect and report local tool availability, versions, capability status, and missing dependency conditions for PDF/OCR/runtime tools.

Related requirements: FR-031, NFR-004, NFR-010, NFR-013, NFR-014

Key interfaces: external process adapter, diagnostics service, UI capability display, packaging/orchestration surfaces.

Dependency constraints: Must not require network access for normal operation.

Data ownership: Owns dependency capability report data.

## HLA-PKG Platform Packaging Boundary

Responsibility: Define architectural packaging boundaries for native runtime delivery across Linux, macOS, and Windows package targets.

Related requirements: FR-028, FR-031, NFR-009, NFR-013, NFR-014, NFR-015

Key interfaces: application runtime boundary, dependency capability service, configuration and artifact roots.

Dependency constraints: Exact packaging implementation is deferred to Packaging phase.

Data ownership: Owns no runtime data; defines deployable artifact boundaries.

## HLA-SAFE Source Material Safety Boundary

Responsibility: Enforce local-only source/derived artifact posture, safe report behavior, source PDF non-modification, repository ignore expectations, and restricted egress assumptions.

Related requirements: FR-018, FR-025, FR-027, FR-030, NFR-001, NFR-002, NFR-003, NFR-012

Key interfaces: validation service, work-folder manager, diagnostics service, configuration service.

Dependency constraints: Must be treated as cross-cutting architecture, not optional UI behavior.

Data ownership: Owns policy definitions and validation findings, not source content.

---

# 7. Data Architecture

Persistence model:

- File-system-backed local artifacts, not a required database.
- One work folder per selected volume/PDF-backed work unit.
- Structured JSON for volume metadata, review state, and quality diagnostics.
- UTF-8 page text files for reviewed searchable page text.
- Raw embedded/OCR candidates retained as local diagnostics.

Primary persisted artifacts:

- `volume.json`: volume metadata, source PDF metadata, indexing intent, bibliographic fields, cover page metadata, and page map.
- `review-state.json`: page status, reviewer notes, selected source where applicable, and review workflow state.
- `page-quality.json`: extraction diagnostics, candidate availability, character counts, flags, and errors.
- `pages/NNNN.txt`: reviewed page text search surrogate.
- `raw/embedded/NNNN.txt`: optional raw embedded text candidate.
- `raw/ocr/NNNN.txt`: optional raw OCR candidate.

Data ownership boundaries:

- HLA-WORK owns artifact lifecycle.
- HLA-META owns volume metadata and page map semantics.
- HLA-TEXT owns reviewed page text semantics.
- HLA-REVIEW owns review status semantics.
- HLA-DIAG owns quality diagnostics semantics.
- HLA-VALID owns validation report semantics.

Consistency guarantees:

- PDF page index and zero-padded page file ID remain stable.
- One reviewed text file exists per PDF page after work-folder initialization or validation repair workflow.
- Release/indexing eligibility derives from review state, defaulting to `accepted` pages only.
- Raw candidates never replace reviewed text without explicit workflow action.

Validation checkpoints:

- PDF selection/inventory.
- Work-folder creation/update.
- Extraction candidate generation.
- Page save or navigation save boundary.
- Review status change.
- Validation-only and dry-run workflows.
- Cleanup and backup/restore workflows.

---

# 8. Non-Functional Architecture

Performance constraints:

- Long-running inventory, extraction, OCR, and validation workflows are coordinated through HLA-APP with progress and observability from HLA-DIAG and HLA-DEP.
- Architecture allows work to be processed page-wise so UI responsiveness and cancellation/degradation can be designed later.

Scalability posture:

- The approved workflow is one selected PDF at a time.
- Larger volumes are supported through page-wise artifacts and page map entries, not corpus-wide database assumptions.

Reliability mechanisms:

- Work-folder validation is independent of UI.
- Backup and restore boundaries protect reviewed text and metadata before bulk regeneration or cleanup.
- External file modification detection is handled as a single-user integrity concern.

Fault tolerance strategy:

- External tool failures are captured as diagnostics and do not silently update reviewed text or release eligibility.
- Corrupt/missing artifacts are validation findings, not hidden runtime assumptions.

Security controls:

- Local-only operation is the default boundary.
- Source PDFs are never modified.
- Safe reporting avoids substantive source text.
- Repository ignore expectations are validated as a source-material protection concern.

Audit logging:

- Page-quality, review-state, inventory metadata, tool capability reports, and validation summaries provide auditability.
- Exact audit-event schema is deferred to Detailed Design.

Observability mechanisms:

- Page-level diagnostics and readiness summaries expose extraction quality, OCR use, skipped/quarantined state, and validation outcomes.
- Tool/version/capability data is reportable.

Configuration management:

- Output paths, source roots, retention/cleanup policy, and tool capability settings are controlled through HLA-CONFIG and not hard-coded.

Reproducibility controls:

- Tool availability/version reporting, extraction diagnostics, raw candidate retention, and schema-versioned artifacts support repeatability.

---

# 9. Failure Posture

Deterministic failure modes:

- Source PDF missing, unreadable, encrypted, password-protected, damaged, or malformed.
- Duplicate PDF detected by checksum.
- Work-folder artifacts missing, malformed, schema-incompatible, or page-count mismatched.
- Reviewed text file missing for a page.
- External modification detected while a file is loaded.
- Output root unavailable or unsafe.
- Backup or cleanup operation incomplete.

Probabilistic failure modes:

- OCR text incorrect, incomplete, out of order, symbol-heavy, numeric-heavy, or low confidence.
- Multi-column pages, tables, maps, or unusual typography degrade OCR quality.
- Embedded text exists but is misleading or worse than OCR.

Integration failure scenarios:

- PDF rendering/extraction tool missing or incompatible.
- OCR engine missing, incompatible, or produces no usable candidate.
- Platform-specific file path, permission, package, or accessibility behavior differs across targets.

Degradation strategy:

- Record diagnostics and continue where safe.
- Leave pages in raw/editing/quarantined status instead of making them release eligible.
- Permit manual text normalization when extraction candidates are poor.
- Support validate-only and dry-run modes before modifying artifacts.

Recovery strategy:

- Restore reviewed text from backups or raw candidates when appropriate.
- Re-run extraction for a page or volume through controlled workflow.
- Rebuild missing page files or review entries through validation-guided repair in later design.

Rollback posture:

- Bulk regeneration and cleanup require backup/protection boundaries.
- Source PDFs are never modified, so source rollback is not required by this system.

Risk concentration points:

- OCR quality and mode selection.
- Native cross-platform UI/tool packaging.
- File-system artifact integrity during simultaneous external edits.
- Broadening into library packaging before scope is assigned.

---

# 10. Deployment and Packaging Alignment

Target runtime environments:

- Linux/Fedora on X86_64 and AArch64, minimum Fedora 44.
- Linux/Ubuntu on X86_64 and AArch64, minimum Ubuntu 26.04 LTS.
- macOS on X86_64 and AArch64, minimum macOS 26.4.
- Windows on X86_64, minimum Windows 11.

Infrastructure assumptions:

- Runs as a local native desktop application.
- Does not require hosted services, remote APIs, application-managed network services, or a database.
- Uses operating-system filesystem access to source PDFs and work folders.
- User-mounted or cloud-synced paths such as SharePoint, Google Drive, Dropbox, NFS, Samba/SMB, or similar storage are treated as filesystem paths when exposed by the OS, but sync conflicts, locking semantics, latency, and offline availability are user/environment responsibilities unless later requirements add explicit support.
- Uses local external tools or bundled/runtime dependencies where packaging later defines them.

Containerization:

- Not required by current requirements.

Networking model:

- No application-owned network access required for normal operation.
- The application does not manage remote storage protocols or cloud provider APIs.

Artifact boundaries:

- Runtime application package.
- Local configuration.
- Work folders and extraction artifacts.
- Safe diagnostic/report artifacts.
- External tool dependencies or capability declarations.

Package target alignment:

- Linux: AppImage, `.deb`, `.rpm`.
- macOS: `.dmg`.
- Windows: `.msi`.

Exact package build tooling, installer behavior, signing/notarization policy, dependency bundling, and update behavior are deferred to Packaging and Orchestration phases.

---

# 11. Orchestration Alignment

Build entry point:

- Not selected in HLA.
- Architecture requires separate build boundaries for application runtime, domain/application logic, adapter integrations, validation harnesses, and packaging.

Dependency management strategy:

- External PDF/OCR tools are represented by adapter/capability boundaries.
- Final decision to bundle, require, or detect each tool is deferred.

Clean build expectations:

- Build outputs must not include source PDFs or derived full-text artifacts.
- Generated package/index/test outputs must remain separated from source-controlled documentation and source code.

CI/CD alignment:

- Later phases should validate schema, RTM coverage, artifact contracts, and unit/integration boundaries without requiring copyrighted PDFs.
- OCR/PDF integration tests should use approved fixtures or local-only corpus material with safe reporting.

Validation harness execution:

- Validation service and OCR boundary tests must be runnable without UI driving.
- Safe reports must avoid substantive source text.

---

# 12. Risk Assessment

<table style="font-size: 0.9em;">
  <thead>
    <tr>
      <th style="white-space: nowrap;">Risk ID</th>
      <th>Risk</th>
      <th style="white-space: nowrap;">Class</th>
      <th>Mitigation</th>
      <th style="white-space: nowrap;">Blocks DD?</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-001</td>
      <td>OCR quality may be insufficient for table-heavy, multi-column, or degraded scans.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Keep OCR probabilistic, preserve raw candidates, require human acceptance, and maintain diagnostics.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-002</td>
      <td>Native UI framework selection may constrain cross-platform packaging or accessibility.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Run focused framework/package/accessibility spike before final HLA approval or early DD.</td>
      <td style="white-space: nowrap;">Yes</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-003</td>
      <td>PDF and OCR tool dependency behavior may vary across operating systems.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Keep tools behind adapters and capability reporting; decide bundling/detection later with platform evidence.</td>
      <td style="white-space: nowrap;">Yes</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-004</td>
      <td>Work-folder schema may grow beyond comfortable volume-level JSON size for future corpora.</td>
      <td style="white-space: nowrap;">Moderate</td>
      <td>Keep schema versioned; retain pageMap in volume metadata for now; revisit if evidence shows pressure.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-005</td>
      <td>Broader library package ambitions may leak into extractor scope.</td>
      <td style="white-space: nowrap;">Moderate</td>
      <td>Keep reusable package/export format in waiting room until requirements assign ownership.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">HLA-RISK-006</td>
      <td>Accidental exposure of copyrighted source text through diagnostics, reports, or commits.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Maintain HLA-SAFE boundary, safe report rules, ignore coverage, and local-only artifact classification.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
  </tbody>
</table>

---

# 13. Traceability Summary

RTM reference: `docs/requirements-traceability-matrix.md` v0.1.

Confirm:

- All Architectural Components map to Requirement IDs: Yes
- No orphan requirements exist: Yes, all approved FR/NFR IDs map to at least one HLA component in the RTM
- No untraceable structural elements exist: Yes, all HLA components cite supporting requirements

Architecture-to-RTM mapping is initialized in the RTM. Detailed Design, implementation, test, packaging, and orchestration mappings remain pending until their lifecycle phases.

---

# 14. Open Questions

No remaining open question is considered an HLA blocker. The following items are carried forward as Detailed Design resolution items:

- Select the native UI framework and validate the choice against native UX, cross-platform packaging, accessibility, and maintainable UI implementation requirements.
- Decide whether Qt/C++ with QML/Quick presentation remains the preferred UI approach or whether another native framework is justified.
- Select the PDF inspection/rendering/extraction library or toolchain and define which capabilities are productized versus replaceable external tooling.
- Decide whether Tesseract is the default OCR engine and whether OCR mode selection is fixed, adaptive, or comparative behind the scenes.
- Keep OCRmyPDF in reserve as a reference or fallback preprocessing option unless later evidence justifies productizing it.
- Define the dependency packaging model per platform: bundled tools, detected system tools, or hybrid.
- Define the audit/event schema for review actions, saves, backups, and extraction runs.
- Decide what package/export concept, if any, belongs to this extractor versus a broader library system.

The DD phase must resolve UI framework/package feasibility and external PDF/OCR dependency strategy before DD approval.

---

# 15. Phase Gate Declaration

Confirm readiness to proceed to Detailed Design:

- All mandatory sections completed? Yes
- Deterministic-probabilistic boundaries defined? Yes
- NFR-driven structure demonstrated? Yes
- RTM updated? Yes
- Human approval granted? Yes

Architecture phase is approved. Detailed Design may begin as the next lifecycle phase.

---

# Approval

Approved By: Chuck
Role: Project Owner
Date: 2026-05-01
Version Incremented: No

---

End of High-Level Architecture
