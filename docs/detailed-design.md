# Detailed Design (DD)

Project Name: pdf-text-extractor
Version: 0.1
Date (YYYY-MM-DD): 2026-05-01
Author(s): Chuck, Codex
Status: Approved
Architecture Version Reference: `docs/high-level-architecture.md` v0.1 approved
Requirement Version Reference: `docs/software-requirements-specification.md` v0.1 approved
RTM Version Reference: `docs/requirements-traceability-matrix.md` v0.1

**Lifecycle position (authoritative):** Gates 1–8 Phase A are closed; **Gate 9 (Documentation closure)** is active; **Packaging Phase B** installers deferred. See `docs/phase-gate-record.md`.

---

# 1. Design Authority Declaration

Confirm:

- Architecture phase approved? Yes
- Architectural Component IDs stable? Yes
- NFR structural intent documented? Yes
- Advancement to Detailed Design authorized? Yes

This DD refines the approved HLA. It does not authorize implementation, test planning, packaging execution, or lifecycle advancement.

---

# 2. Scope of This Design

Architectural Component IDs refined:

- HLA-UI
- HLA-APP
- HLA-INV
- HLA-WORK
- HLA-META
- HLA-PDF
- HLA-EXT
- HLA-OCR
- HLA-TEXT
- HLA-REVIEW
- HLA-DIAG
- HLA-VALID
- HLA-CONFIG
- HLA-DEP
- HLA-PKG
- HLA-SAFE
- HLA-THEME

Associated Requirement IDs:

- FR-001 through FR-033
- NFR-001 through NFR-016

Document boundaries:

- Defines component contracts, data models, state transitions, failure behavior, and design-level technology posture.
- Defines planned implementation units and planned test identifiers for traceability continuity.
- Does not define source code, final UI layout, final package scripts, installer logic, CI pipeline, or executable test cases.

Explicit exclusions:

- No implementation code.
- No test plan execution.
- No packaging execution.
- No source PDF or extracted full-text artifact inclusion.
- No broader electronic-library package format beyond extractor-owned artifacts.

---

# 3. Architectural Conformance

Approved HLA version: `docs/high-level-architecture.md` v0.1 approved.

Structural constraints preserved:

- Native local application.
- Modular layered design with presentation, application workflow, domain services, and adapter boundaries.
- Domain and application logic remain independent of UI toolkit.
- PDF/OCR tools are isolated behind adapters.
- OCR remains contained by OCR-BND-001.
- Work-folder artifacts remain the downstream contract.
- Local-only source-material safety rules are cross-cutting.

Dependency rules:

- QML/Quick presentation calls C++ application facades, not extraction tools directly.
- C++ application layer coordinates domain services through explicit interfaces.
- Domain services do not depend on QML, Qt Quick controls, Poppler, Tesseract, OCRmyPDF, package tools, or downstream consumers.
- External process/tool calls are isolated behind adapter interfaces.

Deterministic-probabilistic boundaries:

- OCR-BND-001 is preserved.
- OCR candidates cannot self-authorize release/indexing eligibility.
- Reviewed text plus review status remains the deterministic release authority.

Framework and tool posture for this DD draft:

- Native UI design baseline to validate: Qt 6.10.3 or newer C++ application core with QML/Qt Quick presentation.
- Native UI fallback if QML/Qt Quick is not viable: standard Qt/C++ Widgets presentation over the same C++ application/domain services.
- Backend/domain design baseline: C++ services exposed to QML only through narrow application facades.
- PDF tool design baseline: Poppler-family capabilities or equivalent adapter-backed PDF library/tooling.
- OCR design baseline: Tesseract behind HLA-OCR adapter.
- OCRmyPDF posture: reserve/reference preprocessing option only; not a primary product dependency unless later evidence justifies it.

No architectural boundaries are altered. No new architectural components are introduced.

---

# 4. Component Decomposition

## 4.1 HLA-UI Native Reviewer UI

Component ID: HLA-UI

Associated Requirement IDs: FR-001, FR-006, FR-007, FR-010, FR-015, FR-016, FR-017, FR-019, FR-020, FR-021, FR-022, FR-027, FR-028, FR-033, NFR-006, NFR-009, NFR-015, NFR-016

Responsibilities:

- Present native desktop workflow using QML/Qt Quick controls.
- Display selected PDF/page, synchronized reviewed page text, metadata summary, diagnostics, review status, and readiness summary.
- Open metadata edit dialog with save/cancel behavior.
- Provide a conventional application toolbar or equivalent command surface for common review actions expected in a modern native UI.
- Provide theme selection access through settings/preferences or equivalent native UI affordance.
- Provide navigation controls, jump-to-page, keyboard shortcuts, status controls, save indicators, and validation/cleanup triggers.
- Provide hover tooltips for icon-only controls, abbreviated status indicators, diagnostics flags, and other controls whose purpose is not obvious from visible text.
- Provide accessible labels, focus order, visible focus, keyboard alternatives, and reasonable target sizing.

Shall not:

- Invoke PDF/OCR tools directly.
- Write work-folder files directly.
- Determine release eligibility.
- Own durable state beyond presentation state.

Interface contracts:

- `selectPdf(path: LocalPath) -> CommandResult<VolumeSession>`
  - Validates path exists and is a PDF-like local file.
  - Opens or initializes application session through HLA-APP.
- `navigateToPage(pageId: PageId) -> CommandResult<PageViewModel>`
  - Requires selected volume.
  - Saves or rejects dirty text according to HLA-APP save policy before navigation.
- `updateReviewedText(pageId: PageId, text: Utf8Text) -> CommandResult<SaveState>`
  - Sends text to HLA-TEXT through HLA-APP.
- `openMetadataEditor(volumeId: VolumeId) -> MetadataDraft`
  - Produces editable draft; save commits through HLA-META, cancel discards.
- `setPageStatus(pageId: PageId, status: ReviewStatus, notes: string) -> CommandResult<ReviewState>`
- `setTheme(themeId: ThemeId) -> CommandResult<ThemePreference>`
  - Sends theme preference changes to HLA-THEME through HLA-APP or equivalent UI settings facade.

UI/UX contract:

- Every icon-only action control has a concise tooltip.
- Tooltip text names the action and, where useful, states the current keyboard shortcut.
- Diagnostics/status tooltips explain flags without quoting source text.
- Tooltips are supplementary; core workflows remain usable by visible labels, menus, keyboard focus, and accessible names.
- Tooltip availability must not be the only way to discover a destructive, irreversible, or high-risk action.
- The existing wireframe should be revisited during UI design to evaluate adding a toolbar for high-frequency commands such as open/select PDF, previous/next page, accept, skip, validate, compare candidates, restore, and save/status actions.
- Toolbar buttons should prefer recognizable icons over visible text labels for common commands.
- Icon-only toolbar buttons must provide tooltips, accessible names, keyboard/menu equivalents, and visible enabled/disabled state.
- Toolbar command placement must not replace menu access, keyboard shortcuts, or accessible command names.
- Theme controls apply to application chrome and controls, not source PDF rendering fidelity or extracted content.

Internal structure:

- QML shell: main window, toolbar/command surface, split PDF/text review surface, metadata pane, diagnostics pane, status bar.
- C++ `ReviewSessionFacade`: QML-facing command and read-model boundary.
- C++ `ViewModelStore`: current session, current page, save state, validation summary, capability summary, theme preference.

Concurrency:

- UI thread owns presentation state.
- Long-running work is dispatched through HLA-APP worker tasks with progress events.
- UI receives immutable read-model snapshots.

Determinism:

- UI commands are deterministic requests; extraction/OCR outcomes are reported, not assumed.

## 4.2 HLA-APP Application Workflow Coordinator

Component ID: HLA-APP

Associated Requirement IDs: FR-001, FR-004, FR-017, FR-018, FR-022, FR-023, FR-026, FR-027, FR-030, FR-032, NFR-005, NFR-007, NFR-010

Responsibilities:

- Coordinate all application use cases.
- Enforce command ordering, dirty-state handling, backup-before-bulk-change policy, dry-run behavior, cleanup confirmation, and external modification detection response.
- Publish progress, warnings, and command results to UI.

Shall not:

- Implement PDF extraction, OCR, schema validation, or UI rendering.
- Bypass HLA-SAFE source-material policy.

Primary interfaces:

- `openVolume(sourcePdf: LocalPath, outputRoot?: LocalPath) -> CommandResult<VolumeSession>`
- `initializeWorkFolder(request: InitWorkFolderRequest) -> CommandResult<WorkFolderRef>`
- `extractCandidates(scope: ExtractionScope, policy: ExtractionPolicy) -> TaskHandle`
- `saveReviewedText(pageId: PageId, text: Utf8Text, savePolicy: SavePolicy) -> CommandResult<SaveState>`
- `changeReviewState(pageId: PageId, transition: ReviewTransition) -> CommandResult<ReviewState>`
- `validateWorkFolder(mode: ValidationMode) -> CommandResult<ValidationReport>`
- `cleanupArtifacts(request: CleanupRequest) -> CommandResult<CleanupReport>`

Preconditions:

- Commands requiring a selected volume must have an active `VolumeSession`.
- Mutating commands must pass local-path and source-material safety checks.

Postconditions:

- Command outcomes include success/failure, safe message, affected artifact IDs, and recoverability.

Error semantics:

- Recoverable errors return structured `CommandError`.
- Fatal session errors leave source PDFs untouched and work-folder state unchanged where possible.

Internal structure:

- `WorkflowController`
- `TaskScheduler`
- `ProgressEventBus`
- `DirtyStateCoordinator`
- `CommandAuditSink`

Audit persistence:

- Audit events are appended as timestamped entries to a local application logfile.
- Audit events record operation type, volume ID, page ID when applicable, status/result, safe message, and artifact identifiers.
- Audit log entries must not include substantive PDF or reviewed page text.

## 4.3 HLA-INV Source Inventory Service

Component ID: HLA-INV

Associated Requirement IDs: FR-001, FR-002, FR-003, NFR-001, NFR-003, NFR-004, NFR-008

Responsibilities:

- Inspect selected source PDF without modifying it.
- Record hash, size, modified time, page count, readability, duplicate evidence, and structural warnings.
- Classify encrypted, damaged, password-protected, malformed, readable, and unreadable states.

Interface contracts:

- `inspectPdf(path: LocalPath) -> InventoryRecord`
- `computePdfIdentity(path: LocalPath) -> PdfIdentity`
- `detectDuplicate(identity: PdfIdentity, knownVolumes: VolumeIndex) -> DuplicateFinding[]`

Validation:

- Path must be local and readable.
- Hash calculation reads bytes only.
- Duplicate detection reports evidence and does not delete files.

Data ownership:

- Owns `InventoryRecord` and `PdfIdentity` models until persisted into `volume.json`.

## 4.4 HLA-WORK Work Folder Artifact Manager

Component ID: HLA-WORK

Associated Requirement IDs: FR-004, FR-011, FR-023, FR-029, FR-030, FR-032, NFR-001, NFR-002, NFR-007, NFR-011

Responsibilities:

- Create and maintain work-folder layout.
- Read/write JSON artifacts atomically.
- Read/write UTF-8 page text and raw candidate files.
- Create backups before protected bulk operations.
- Detect external modification using file metadata and content hashes where needed.

Interface contracts:

- `createWorkFolder(plan: WorkFolderPlan) -> CommandResult<WorkFolderRef>`
- `readVolume(ref: WorkFolderRef) -> Result<VolumeMetadata>`
- `writeVolume(metadata: VolumeMetadata, policy: WritePolicy) -> CommandResult<ArtifactWrite>`
- `readReviewedPage(pageId: PageId) -> Result<PageText>`
- `writeReviewedPage(pageId: PageId, text: Utf8Text, policy: WritePolicy) -> CommandResult<ArtifactWrite>`
- `writeRawCandidate(pageId: PageId, source: CandidateSource, text: Utf8Text, metadata: CandidateMetadata) -> CommandResult<ArtifactWrite>`
- `createBackup(scope: BackupScope) -> CommandResult<BackupRef>`
- `cleanup(scope: CleanupScope, dryRun: bool) -> CommandResult<CleanupReport>`

Invariants:

- One `pages/NNNN.txt` exists for every PDF page after initialization.
- Raw candidates are separate from reviewed text.
- Writes are atomic: write temp, flush, rename.
- Source PDFs are never modified.

## 4.5 HLA-META Volume Metadata and Page Map Service

Component ID: HLA-META

Associated Requirement IDs: FR-005, FR-006, FR-007, FR-008, FR-009, FR-010, NFR-005, NFR-008, NFR-011

Responsibilities:

- Initialize and edit `volume.json`.
- Manage bibliographic metadata, source PDF metadata, cover page metadata, indexing intent, and page map.
- Maintain page map as volume-level metadata unless future evidence requires a schema change.

Interface contracts:

- `initializeVolume(record: InventoryRecord, options: VolumeInitOptions) -> VolumeMetadata`
- `createMetadataDraft(volumeId: VolumeId) -> MetadataDraft`
- `validateMetadataDraft(draft: MetadataDraft) -> ValidationFinding[]`
- `commitMetadataDraft(draft: MetadataDraft) -> CommandResult<VolumeMetadata>`
- `setPrintedPageLabel(pageId: PageId, label: PrintedPageLabel?) -> CommandResult<PageMapEntry>`
- `setCoverPage(pageId: PageId?) -> CommandResult<VolumeMetadata>`

Invariants:

- `volumeId` is stable after initialization unless explicit future migration workflow exists.
- `pageMap` keys are zero-padded page IDs.
- PDF page number and printed page label are distinct fields.

## 4.6 HLA-PDF PDF View and Page Synchronization Adapter

Component ID: HLA-PDF

Associated Requirement IDs: FR-001, FR-014, FR-019, FR-020, NFR-003, NFR-006, NFR-009

Responsibilities:

- Provide non-mutating PDF page rendering and page-count access.
- Provide page images to OCR adapter.
- Keep PDF page and reviewed page text synchronized through HLA-APP.

Interface contracts:

- `openPdf(path: LocalPath) -> Result<PdfDocumentHandle>`
- `getPageCount(handle: PdfDocumentHandle) -> Result<PageCount>`
- `renderPage(page: PdfPageRef, options: RenderOptions) -> Result<PageImage>`
- `getPageLabelCandidate(page: PdfPageRef) -> Result<PrintedPageLabel?>`

Error semantics:

- Rendering failure returns page-specific error and does not block non-render validation if metadata remains readable.

Dependency posture:

- PDF adapter baseline is Qt PDF for native page display and page rendering where viable, with Poppler-family tooling used behind HLA-INV/HLA-EXT for PDF inspection and embedded text extraction.
- The adapter must hide whether page images/text come from Qt PDF, Poppler tools/libraries, or a later substitute.

## 4.7 HLA-EXT Embedded Text Extraction Adapter

Component ID: HLA-EXT

Associated Requirement IDs: FR-012, FR-015, FR-024, NFR-004, NFR-005, NFR-010

Responsibilities:

- Extract embedded text candidates per PDF page.
- Record character count, availability, quality flags, and extraction errors.
- Store raw embedded candidates separately from reviewed page text.

Interface contracts:

- `extractEmbeddedText(page: PdfPageRef) -> CandidateResult`
- `extractEmbeddedText(volume: PdfDocumentHandle, scope: PageScope) -> CandidateBatchResult`

Candidate output:

- `source = embedded`
- `text`
- `charCount`
- `qualitySignals`
- `errors`
- `toolInfo`

Invariants:

- Embedded text never directly changes review status.
- Embedded text never directly overwrites reviewed text.

## 4.8 HLA-OCR OCR Pipeline Adapter

Component ID: HLA-OCR

Associated Requirement IDs: FR-013, FR-014, FR-015, FR-024, FR-031, NFR-004, NFR-005, NFR-010, NFR-012

Responsibilities:

- Route OCR-eligible pages.
- Invoke Tesseract behind OCR-BND-001.
- Capture OCR text, confidence data where available, quality flags, tool version, mode policy, and errors.
- Keep OCRmyPDF as reserve/reference preprocessing, not primary workflow.

Invocation contract:

- `generateOcrCandidate(page: PdfPageRef, image: PageImage, policy: OcrPolicy) -> CandidateResult`
- `generateOcrBatch(volume: PdfDocumentHandle, scope: PageScope, policy: OcrPolicy) -> CandidateBatchResult`

OCR policy fields:

- `engine = tesseract`
- `modePolicy = default | adaptive | comparative`
- `language`
- `dpi`
- `preprocessProfile`
- `retainRawCandidate`

Validation harness logic:

- Compare OCR character count against embedded candidate when both exist.
- Flag embedded/OCR delta greater than 5 percent as suspicious.
- Flag empty, very-short, high-symbol-ratio, high-numeric-ratio, unicode-suspect, and extraction-error conditions.
- Preserve per-page errors and tool/version metadata.
- Comparative OCR mode is included in the first implementation slice as a precautionary candidate-generation policy, subject to runtime controls so users are not forced to choose Tesseract page segmentation modes manually.

Acceptance criteria:

- OCR candidate may become reviewed text only through explicit compare/replace or manual edit workflow.
- Page enters release/indexing only when review status is `accepted`.

Rejection and fallback:

- Poor OCR leaves page raw/editing/quarantined.
- User may restore from embedded candidate, OCR candidate, or manual text.
- OCR failure is diagnostic, not silent.

Reproducibility:

- Record engine name/version, language, mode policy, page render DPI, and preprocessing profile.

## 4.9 HLA-TEXT Reviewed Text and Normalization Service

Component ID: HLA-TEXT

Associated Requirement IDs: FR-011, FR-015, FR-016, FR-022, FR-023, NFR-007, NFR-011, NFR-012

Responsibilities:

- Own reviewed `pages/NNNN.txt` semantics.
- Preserve plain UTF-8 text with no front matter.
- Normalize control characters and Unicode where appropriate.
- Support candidate comparison, replace-from-candidate, restore, and manual editing.

Interface contracts:

- `loadReviewedText(pageId: PageId) -> Result<PageText>`
- `saveReviewedText(pageId: PageId, text: Utf8Text, policy: SavePolicy) -> CommandResult<SaveState>`
- `compareCandidates(pageId: PageId) -> CandidateComparison`
- `replaceFromCandidate(pageId: PageId, source: CandidateSource, backup: bool) -> CommandResult<PageText>`
- `normalizeText(text: Utf8Text, profile: NormalizationProfile) -> NormalizationResult`

Invariants:

- No metadata headers in reviewed page text.
- Empty pages are allowed and represented by review state.
- Tables may be flattened for search and snippet usability.

## 4.10 HLA-REVIEW Review State and Release Gate Service

Component ID: HLA-REVIEW

Associated Requirement IDs: FR-017, FR-018, FR-021, FR-022, FR-027, NFR-005, NFR-007

Responsibilities:

- Own review state transitions.
- Enforce release/indexing rule: only `accepted` pages by default.
- Manage statuses: `raw`, `editing`, `edited`, `accepted`, `skip`, `quarantined`.
- Manage reviewer notes and selected source.

Interface contracts:

- `getReviewState(pageId: PageId) -> ReviewPageState`
- `transition(pageId: PageId, target: ReviewStatus, context: TransitionContext) -> CommandResult<ReviewPageState>`
- `getNextPage(filter: ReviewFilter, from: PageId) -> PageId?`
- `computeReleaseEligibility(pageId: PageId) -> ReleaseEligibility`

Transition rules:

- `raw -> editing | accepted | skip | quarantined`
- `editing -> edited | accepted | skip | quarantined`
- `edited -> accepted | editing | skip | quarantined`
- `accepted -> editing | skip | quarantined`
- `skip -> editing | accepted | quarantined`
- `quarantined -> editing | skip`

## 4.11 HLA-DIAG Diagnostics and Readiness Service

Component ID: HLA-DIAG

Associated Requirement IDs: FR-002, FR-024, FR-027, FR-031, NFR-004, NFR-005, NFR-010, NFR-012

Responsibilities:

- Own page-quality semantics and readiness summary.
- Aggregate counts by status, selected source, OCR use, flags, and validation findings.
- Generate safe reports without substantive page text.

Interface contracts:

- `recordCandidateDiagnostics(pageId: PageId, candidate: CandidateResult) -> CommandResult<PageQualityEntry>`
- `flagPage(pageId: PageId, flag: QualityFlag, details?: SafeDetails) -> CommandResult<PageQualityEntry>`
- `summarizeReadiness(volumeId: VolumeId) -> ReadinessSummary`
- `exportQualityReport(format: ReportFormat) -> SafeReport`

Safety:

- Reports may include filenames, IDs, counts, flags, hashes, statuses, and error classes.
- Reports must not include full page text or substantive copyrighted excerpts.

## 4.12 HLA-VALID Validation and Dry-Run Service

Component ID: HLA-VALID

Associated Requirement IDs: FR-025, FR-026, FR-027, NFR-004, NFR-005, NFR-007, NFR-011, NFR-012

Responsibilities:

- Validate work-folder schema, IDs, JSON syntax, page counts, page files, review entries, quality entries, statuses, and release eligibility.
- Support dry-run and validate-only behavior.

Interface contracts:

- `validateWorkFolder(ref: WorkFolderRef, mode: ValidationMode) -> ValidationReport`
- `validateReleaseEligibility(ref: WorkFolderRef, policy: ReleasePolicy) -> ValidationReport`
- `planRepair(report: ValidationReport) -> RepairPlan`
- `executeRepair(plan: RepairPlan, dryRun: bool) -> CommandResult<RepairReport>`

Validation finding fields:

- `severity = info | warning | error | blocker`
- `code`
- `artifact`
- `pageId?`
- `safeMessage`
- `repairable`

## 4.13 HLA-CONFIG Configuration and Output Policy Service

Component ID: HLA-CONFIG

Associated Requirement IDs: FR-029, FR-030, NFR-001, NFR-002, NFR-008, NFR-011

Responsibilities:

- Own source root, output root, artifact retention, cleanup, and local-only policy configuration.
- Prevent hard-coded corpus-specific output paths.

Interface contracts:

- `loadConfiguration() -> AppConfiguration`
- `validateOutputRoot(path: LocalPath) -> ValidationFinding[]`
- `setOutputRoot(path: LocalPath) -> CommandResult<AppConfiguration>`
- `getArtifactPolicy() -> ArtifactPolicy`

Invariants:

- Output roots must be paths accessible through the operating system filesystem API.
- Configuration must not require application-owned network services, remote APIs, or remote databases.
- User-mounted or cloud-synced filesystem paths are allowed only as normal filesystem paths; sync conflicts, remote availability, latency, and locking semantics are not guaranteed by the application.

## 4.13.1 Validation Repair Clarification

Validation repair means structural artifact repair proposed by HLA-VALID, not the user manually editing reviewed `pages/NNNN.txt` content.

Examples of validation repair:

- Create a missing empty `pages/NNNN.txt` file.
- Add a missing `review-state.json` page entry with safe default status.
- Add a missing `page-quality.json` page entry with diagnostic defaults.
- Rebuild or normalize page map entries from known PDF page count.
- Report schema mismatch requiring manual migration when automated repair is unsafe.

Validation repair shall not:

- Rewrite reviewed page text content except to create a missing empty file.
- Accept, skip, or quarantine pages without explicit user action.
- Replace reviewed text with embedded/OCR candidates.

## 4.14 HLA-DEP Dependency Capability Service

Component ID: HLA-DEP

Associated Requirement IDs: FR-031, NFR-004, NFR-010, NFR-013, NFR-014

Responsibilities:

- Detect runtime capabilities and versions for PDF/OCR tools and application runtime.
- Report missing or incompatible dependency conditions.
- Support packaging decisions.

Interface contracts:

- `detectCapabilities() -> CapabilityReport`
- `checkTool(toolId: ToolId) -> ToolCapability`
- `getRuntimeInfo() -> RuntimeInfo`

Tool capability fields:

- `toolId`
- `detected`
- `version`
- `path`
- `supportedFeatures`
- `safeMessage`

## 4.15 HLA-PKG Platform Packaging Boundary

Component ID: HLA-PKG

Associated Requirement IDs: FR-028, FR-031, NFR-009, NFR-013, NFR-014, NFR-015

Responsibilities:

- Define deployable application boundary and dependency packaging assumptions.
- Preserve package target expectations for Linux AppImage/deb/rpm, macOS dmg, and Windows msi.

Design posture:

- Qt 6.10.3 or newer C++/QML is the DD baseline subject to validation and approval, with Qt/C++ Widgets retained as the fallback presentation path.
- Platform package implementation is deferred to Packaging phase.
- Poppler and Tesseract should be bundled with distributable packages where licensing and platform packaging allow, because user systems cannot be assumed to have them installed.
- Runtime detection remains useful for diagnostics and fallback, but normal packaged operation should not depend on preinstalled Poppler/Tesseract.

## 4.16 HLA-SAFE Source Material Safety Boundary

Component ID: HLA-SAFE

Associated Requirement IDs: FR-018, FR-025, FR-027, FR-030, NFR-001, NFR-002, NFR-003, NFR-012

Responsibilities:

- Enforce source PDF non-modification.
- Classify source PDFs, reviewed text, raw candidates, OCR outputs, caches, and full-text derivatives as local restricted artifacts.
- Keep diagnostics/report exports safe.
- Validate ignore coverage expectations.

Interface contracts:

- `classifyArtifact(path: LocalPath) -> ArtifactClassification`
- `authorizeWrite(operation: WriteOperation) -> PolicyDecision`
- `authorizeReport(report: SafeReport) -> PolicyDecision`
- `validateRepositorySafety(repoRoot: LocalPath) -> ValidationFinding[]`

Invariants:

- Source PDFs are read-only to this application.
- Reports omit substantive page text.
- Restricted artifacts are not intentionally published by normal workflows.

## 4.17 HLA-THEME Theme and Appearance Service

Component ID: HLA-THEME

Associated Requirement IDs: FR-033, NFR-009, NFR-015, NFR-016

Responsibilities:

- Own built-in theme definitions.
- Persist the user's selected theme locally.
- Apply theme tokens to application chrome, controls, toolbars, diagnostics, text editor chrome, metadata panes, dialogs, and status indicators.
- Validate built-in themes against contrast, visible focus, tooltip readability, and diagnostics/status legibility expectations.

Shall not:

- Change source PDF rendering fidelity or imply changes to source content.
- Modify extraction artifacts, reviewed page text, raw candidates, `volume.json`, `review-state.json`, or `page-quality.json`.
- Store theme preferences in downstream work-folder or project artifacts.

Interface contracts:

- `listThemes() -> ThemeDescriptor[]`
- `getCurrentTheme() -> ThemePreference`
- `setTheme(themeId: ThemeId) -> CommandResult<ThemePreference>`
- `resolveThemeTokens(themeId: ThemeId) -> ThemeTokenSet`
- `validateTheme(themeId: ThemeId) -> ValidationFinding[]`

Theme descriptor fields:

- `themeId`
- `displayName`
- `baseMode = light | dark | high-contrast | sepia | system`
- `isBuiltIn`
- `accessibilityNotes`

Invariants:

- At least one default/light, one dark or high-contrast-friendly, and one warm sepia theme are available.
- Theme preference is per-user local application configuration only.
- Theme validation failures are design/test blockers for built-in themes.

---

# 5. Data Design

## 5.1 Primitive Types

- `VolumeId`: stable slug string.
- `PageId`: four-digit zero-padded string matching PDF page position, e.g. `0001`.
- `PdfPageNumber`: positive integer starting at 1.
- `PrintedPageLabel`: optional user-editable string.
- `LocalPath`: normalized local file-system path.
- `Utf8Text`: valid UTF-8 text.
- `SchemaVersion`: positive integer.
- `ThemeId`: stable identifier for a built-in or configured UI theme.

## 5.2 volume.json

Required top-level fields:

- `schemaVersion`
- `volumeId`
- `title`
- `subtitle`
- `sortTitle`
- `group`
- `bibliographic`
- `sourcePdf`
- `indexing`
- `collection`
- `cover`
- `pageMap`
- `notes`

Bibliographic fields:

- `publisher`
- `publicationYear`
- `originalPublicationYear`
- `edition`
- `language`
- `isbn`
- `productCode`
- `creators`
- `series`
- `seriesNumber`
- `description`
- `subjects`
- `rights`

Page map entry:

- `pageId`
- `pdfPage`
- `pageTextPath`
- `printedPageLabel`
- `includeByDefault`

Validation rules:

- `volumeId` matches all companion artifacts.
- `sourcePdf.pageCount` equals page map count.
- `pageMap` contains one entry per PDF page.
- `pageTextPath` points to `pages/NNNN.txt`.

## 5.3 review-state.json

Fields:

- `schemaVersion`
- `volumeId`
- `pages`

Page entry:

- `pdfPage`
- `status`
- `selectedSource`
- `reviewedAt`
- `reviewedBy`
- `notes`
- `dirtyState`

Allowed statuses:

- `raw`
- `editing`
- `edited`
- `accepted`
- `skip`
- `quarantined`

## 5.4 application configuration

Application configuration includes:

- `sourceRoot`
- `outputRoot`
- `artifactRetentionPolicy`
- `ocrPolicy`
- `toolPaths`
- `themePreference`

Theme preference fields:

- `themeId`
- `followSystemAppearance`
- `lastUpdatedAt`

Configuration is per-user local application state and is not part of downstream work-folder/project output.

## 5.5 page-quality.json

Fields:

- `schemaVersion`
- `volumeId`
- `pages`

Page quality entry:

- `pdfPage`
- `selectedSource`
- `embedded`
- `ocr`
- `flags`
- `errors`

Candidate diagnostic fields:

- `available`
- `charCount`
- `qualityScore`
- `toolInfo`
- `modePolicy`

Useful flags:

- `empty`
- `very-short`
- `high-symbol-ratio`
- `high-numeric-ratio`
- `ocr-used`
- `embedded-text-suspicious`
- `possible-reading-order-issue`
- `extraction-error`
- `page-count-mismatch`
- `unicode-suspect`
- `embedded-ocr-delta-suspicious`

## 5.6 Text Artifacts

Reviewed text:

- Path: `pages/NNNN.txt`
- Format: plain UTF-8 text
- No front matter
- One file per PDF page, including empty pages

Raw candidates:

- Path: `raw/embedded/NNNN.txt`
- Path: `raw/ocr/NNNN.txt`
- Diagnostic only
- Local restricted artifact classification

## 5.7 Serialization and Persistence

- JSON uses UTF-8.
- JSON writes are atomic.
- Page text writes are atomic.
- Schema versions are explicit.
- Unknown fields are preserved where possible during metadata edits.

---

# 6. Failure Semantics

Error categories:

- `InputError`: missing/unreadable/not-PDF/password-protected source.
- `ToolError`: missing tool, unsupported version, nonzero process exit, timeout.
- `ExtractionError`: embedded/OCR extraction failed for page.
- `ValidationError`: invalid schema, missing files, bad status, page mismatch.
- `PolicyError`: unsafe path, source PDF write attempt, unsafe report.
- `ConflictError`: external modification or stale write token.
- `RecoverableWorkflowError`: user-correctable state.
- `FatalWorkflowError`: operation cannot continue safely.

Retry logic:

- File writes: no blind retry after conflict; re-read and prompt/resolve.
- External tools: one configured attempt per policy; batch continues page-wise unless policy says stop.
- Validation: deterministic; no retry except after repair or user action.

Circuit-breaker behavior:

- If tool capability detection fails, dependent extraction/OCR actions are disabled or marked unavailable.
- If repeated page-level OCR failures occur, batch OCR continues only if policy allows continue-on-error.

Degradation posture:

- Fail page-wise where possible.
- Preserve diagnostics.
- Leave release eligibility closed.

Recovery strategy:

- Restore from backup.
- Restore reviewed text from raw candidate.
- Re-run extraction/OCR for page or volume.
- Mark page skip/quarantined.

Escalation:

- Unsafe artifact operation is blocked.
- Schema migration need is reported as blocker until design/implementation supports it.

---

# 7. Non-Functional Derivation

- NFR-001 Local-only operation: HLA-CONFIG accepts operating-system filesystem paths only; no application-owned network service, remote API, or remote database is required.
- NFR-002 Source material protection: HLA-SAFE classifies restricted artifacts and validates ignore/safe report posture.
- NFR-003 Source PDF non-modification: HLA-INV/HLA-PDF open PDFs read-only; HLA-SAFE blocks source write operations.
- NFR-004 Reproducibility: HLA-DEP records versions; HLA-DIAG records tool/mode/page diagnostics.
- NFR-005 Auditability: HLA-APP audit sink, timestamped local application logfile, and JSON artifacts record review/diagnostic state.
- NFR-006 Sequential usability: HLA-UI/HLA-PDF synchronize PDF page and reviewed text; HLA-REVIEW supports next filtered page.
- NFR-007 Data integrity: HLA-WORK atomic writes, backups, external modification detection, and validation protect artifacts.
- NFR-008 Corpus agnosticism: HLA-CONFIG and HLA-META avoid hard-coded collection paths or Classic Traveller-specific fields.
- NFR-009 Native UX: Qt 6.10.3 or newer C++/QML is the design baseline to validate for native desktop workflow; Qt/C++ Widgets remains the fallback presentation path.
- NFR-010 Observability: progress events, diagnostics, readiness summaries, and capability reports expose long-running work.
- NFR-011 Schema evolvability: schema versions and validation rules are explicit.
- NFR-012 Error report safety: HLA-DIAG/HLA-SAFE omit substantive page text in reports.
- NFR-013 Platform support: Qt 6.10.3 or newer baseline aligns with Linux/macOS/Windows target families; packaging validation remains required.
- NFR-014 Package targets: HLA-PKG keeps AppImage/deb/rpm/dmg/msi targets visible for Packaging phase.
- NFR-015 Accessibility/basic usability: QML controls must expose labels, focus order, keyboard operation, contrast, validation states, and hover tooltips for icon-only or ambiguous controls.
- NFR-016 Theme accessibility: HLA-THEME validates built-in themes for contrast, visible focus, tooltip readability, and diagnostics/status legibility.

---

# 8. Testing Alignment

Planned test identifiers:

- `UT-CONFIG-001`: output root accepts operating-system filesystem path and rejects unsupported URI/API-style remote location.
- `UT-CONFIG-002`: user-mounted/cloud-synced path is treated as filesystem path while warning documentation covers consistency/locking limitations.
- `UT-WORK-001`: work-folder initialization creates one page file per PDF page.
- `UT-WORK-002`: atomic write prevents partial JSON/page text update.
- `UT-META-001`: page map validates PDF page count and page IDs.
- `UT-TEXT-001`: reviewed page text saves plain UTF-8 without metadata headers.
- `UT-REVIEW-001`: review status transitions obey allowed transition matrix.
- `UT-VALID-001`: validation catches missing page files and invalid statuses.
- `UT-SAFE-001`: safe report policy rejects substantive page text.
- `IT-PDF-001`: selected PDF inventory produces identity/page-count/readability record.
- `IT-EXT-001`: embedded extraction stores raw candidate without changing reviewed text.
- `IT-OCR-001`: OCR stores raw candidate and diagnostics without changing release eligibility.
- `IT-UI-001`: navigation changes PDF page and reviewed text page in sync.
- `IT-DEP-001`: missing OCR tool disables OCR action and reports safe diagnostic.
- `NFR-PLAT-001`: smoke launch on supported platform matrix.
- `NFR-A11Y-001`: keyboard operation and visible focus for core review workflow.
- `NFR-A11Y-002`: icon-only and ambiguous controls expose hover tooltips and accessible names.
- `NFR-THEME-001`: built-in light/default, dark or high-contrast-friendly, and warm sepia themes preserve contrast, visible focus, tooltip readability, and diagnostics/status legibility.
- `PBT-OCR-001`: OCR-BND-001 cannot self-authorize accepted status.
- `PBT-OCR-002`: embedded/OCR delta greater than 5 percent creates suspicious flag.

Test planning phase must refine these identifiers into executable test cases.

---

# 9. Packaging and Orchestration Impact

Runtime dependencies introduced by design baseline:

- Qt 6.10.3 or newer runtime modules for C++ application and QML/Qt Quick UI.
- PDF inspection/rendering/extraction capability through adapter-backed Qt PDF and bundled Poppler-family tool/library capability where packaging allows.
- Bundled Tesseract OCR capability through adapter-backed process/tool integration where packaging allows.

Qt platform reference note:

- Qt 6 supported-platform documentation and official Qt release sources were reviewed during DD drafting to validate that the Qt baseline is plausible for the required desktop OS families. The current minimum baseline is Qt 6.10.3 or newer. Exact module set and package behavior remain DD approval items.
- Reference: `https://doc.qt.io/qt-6/supported-platforms.html`
- Reference: `https://download.qt.io/official_releases/qt/6.10/`

Configuration changes:

- Application configuration file for source/output roots, retention policy, OCR policy, and tool paths where needed.

Build process impact:

- C++ application build with QML resources.
- Separate unit-testable domain/application libraries.
- Adapter integration tests isolated from UI.
- GitHub Actions should follow the proven project pattern from `/Users/chuck/CodeMonki-Projects/skrat`: explicit Qt install/cache for macOS and Windows, explicit Linux dependency/bootstrap steps, deploy tooling, artifact upload, and release publishing on tags. For this project the Qt install must target Qt 6.10.3 or newer rather than relying on runner-default Qt packages.

Clean build implications:

- Build must exclude source PDFs, work folders, raw OCR/embedded text, extracted text, caches, and package outputs.

Packaging configuration impact:

- Linux AppImage/deb/rpm packaging must include or otherwise package Qt runtime, Poppler-family capabilities, and Tesseract where licensing and packaging allow.
- macOS dmg packaging must include the app bundle, Qt runtime, Poppler-family capabilities, Tesseract, and tool capability metadata where licensing and packaging allow.
- Windows msi packaging must include Qt runtime, Poppler-family capabilities, Tesseract, and tool capability metadata where licensing and packaging allow.

Pipeline adjustments required:

- Validate documentation/RTM consistency.
- Build without copyrighted corpus material.
- Run safe tests using synthetic or approved fixtures.
- Preferred deployment/packaging builds should run through GitHub Actions where possible.
- GitHub-hosted runners may not provide the current Qt baseline through system packages, so CI orchestration must support installing/caching Qt 6.10.3 or newer through an approved Qt installer/action, prebuilt cache, or self-hosted runner. Prior `skrat` CI experience demonstrates feasibility but also confirms that workflow workarounds are likely.

---

# 10. Traceability Summary

Mapping references:

- Requirements: `docs/software-requirements-specification.md` v0.1 approved
- HLA: `docs/high-level-architecture.md` v0.1 approved
- RTM: `docs/requirements-traceability-matrix.md` v0.1

Planned implementation units:

- `app/ui-qml`
- `app/review-session-facade`
- `core/application`
- `core/domain`
- `core/inventory`
- `core/workfolder`
- `core/metadata`
- `core/text`
- `core/review`
- `core/diagnostics`
- `core/validation`
- `core/config`
- `core/theme`
- `adapters/pdf`
- `adapters/embedded-text`
- `adapters/ocr`
- `adapters/dependency`
- `packaging`

No orphaned DD scope is intentionally introduced. RTM DD mappings are initialized at section/component level and must be refined during DD stabilization.

---

# 11. Risk Assessment

<table style="font-size: 0.9em;">
  <thead>
    <tr>
      <th style="white-space: nowrap;">Risk ID</th>
      <th>Risk</th>
      <th style="white-space: nowrap;">Class</th>
      <th>Mitigation</th>
      <th style="white-space: nowrap;">Blocks Implementation?</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="white-space: nowrap;">DD-RISK-001</td>
      <td>Qt/QML packaging, accessibility behavior, or GitHub Actions availability may not satisfy every target platform cleanly.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Reuse proven GitHub Actions Qt install/deploy patterns where applicable, run framework/package/accessibility spike, and record decision before DD approval.</td>
      <td style="white-space: nowrap;">Yes</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">DD-RISK-002</td>
      <td>Bundling Poppler/Tesseract may differ significantly by OS and license/package mechanics.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Define bundled-first strategy with per-platform exceptions before DD approval.</td>
      <td style="white-space: nowrap;">Yes</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">DD-RISK-003</td>
      <td>OCR quality may remain poor for table-heavy or multi-column pages.</td>
      <td style="white-space: nowrap;">High</td>
      <td>Keep OCR review-gated, preserve raw candidates, and require accepted review state.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">DD-RISK-004</td>
      <td>Schema changes may be needed after real corpus use.</td>
      <td style="white-space: nowrap;">Moderate</td>
      <td>Version schemas and preserve unknown fields where possible.</td>
      <td style="white-space: nowrap;">No</td>
    </tr>
  </tbody>
</table>

---

# 12. Design Decisions

Resolved DD decisions:

- Qt minimum baseline is Qt 6.10.3 or newer.
- Initial Qt module set is Core, Gui, Qml, Quick, Quick Controls, Widgets fallback support, Pdf, Svg, and Concurrent if needed for worker orchestration. Exact CMake component names may be normalized during implementation without changing the design intent.
- GitHub Actions is the preferred deployment/packaging build path. CI shall install/cache Qt 6.10.3 or newer explicitly rather than relying on runner-provided system packages. The prior `skrat` workflow pattern is accepted as feasibility evidence.
- PDF adapter responsibility split: Qt PDF handles native display/rendering where viable; Poppler-family tooling handles PDF inspection and embedded text extraction; rendering may fall back to Poppler where Qt PDF is insufficient.
- Poppler/Tesseract packaging strategy is bundled-first for distributable packages where licensing and platform package mechanics allow. Runtime detection remains diagnostic/fallback behavior, not the normal packaged-user requirement.
- Comparative OCR mode is included in the first implementation slice.
- Audit events persist as UTF-8 JSON Lines in a timestamped local application logfile. Entries include timestamp, event type, volume ID, page ID where applicable, result, safe message, and artifact IDs. Log rotation defaults to size-based rotation at 10 MiB with retention of the most recent 10 files unless changed by later packaging/operations requirements.
- First implementation slice treats validation repair as report/plan-first. Automatic repair is limited to safe initialization-time creation of required missing empty artifacts. Post-validation repair execution requires explicit user confirmation and shall not rewrite reviewed text content except creating a missing empty page file.

No DD open question is considered a blocker after project-owner acceptance of these decisions.

---

# 13. Phase Gate Declaration

Confirm readiness to proceed to Implementation:

- All components decomposed? Yes
- Interface contracts complete? Yes
- NFR derivation explicit? Yes
- Deterministic-probabilistic refinement complete? Yes
- Traceability mapping complete? Yes
- Human approval granted? Yes

Detailed Design phase is approved. Test Planning may begin as the next lifecycle phase.

---

# Approval

Approved By: Chuck
Role: Project Owner
Date: 2026-05-01
Version Incremented: No

---

End of Detailed Design
