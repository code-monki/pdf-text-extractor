# PDF Text Extractor Mockup Review

Status: Ideation Note
Date: 2026-04-30
Lifecycle Phase: Ideation
Reviewed Image: `docs/images/pdf-text-extractor-mockup.png`

Updated Concept Wireframe: `docs/images/pdf-text-extractor-wireframe-v2.svg`

---

## 1. What The Mockup Communicates

The mockup appears to describe a local desktop-style PDF text extraction and review workspace:

- top application title: PDF Text Extractor
- left panel: corpus/file tree
- lower-left panel: selected file metadata
- center panel: likely PDF page preview
- top-center controls: page navigation with first/previous/current/next/last controls
- right panel: selected file/page text area, probably extracted or editable page text
- top-left controls: likely open/import folder and close/remove action
- bottom strip: likely status/progress/log area
- current stakeholder preference is that extraction/review is centered on one selected PDF at a time

This visual direction is consistent with the current extractor-output concept because it centers the workflow on one selected PDF/page and a paired text artifact.

Current stakeholder expectation is that the product is a native local application rather than a browser-based application. The rationale is to keep complexity and user friction low for local file access, PDF review, generated work folders, metadata editing, page navigation, and artifact management. This does not select a native framework or final UI architecture.

---

## 2. Extractor-Scope Signals

The mockup reinforces several extractor-side concepts that should be carried forward into Requirements exploration:

- page-level navigation is central, not volume-only processing
- file-level metadata needs first-class visibility
- the operator needs to compare a source PDF page against extracted/reviewed text
- the displayed text page should follow the selected PDF page automatically
- review work should be resumable by file and page
- status/progress visibility is expected
- the UI likely needs to expose extraction diagnostics or review state near the page/text workflow
- the metadata pane should provide an edit action rather than making the pane itself directly editable

---

## 3. Boundary Risks

This image should not yet be treated as an approved UI design or architecture.

Risks to avoid:

- selecting a native UI framework before requirements and architecture are stabilized
- coupling extraction engine behavior to a specific three-pane layout
- making the browser/indexer responsible for reviewer UI behavior
- hiding diagnostics in a UI-only workflow instead of preserving machine-readable artifacts
- letting file labels like `File 1` substitute for stable `volumeId`, source path, checksum, and page identity
- changing pages without protecting unsaved text edits

---

## 4. Candidate Interaction Behavior

The mockup discussion suggests a useful interaction rule:

- The selected PDF document and selected PDF page should drive the displayed text page.
- When the operator changes the PDF page, the text editor should automatically load the corresponding `pages/NNNN.txt` text surrogate.
- If no PDF document or page is selected, the text editor should be empty.

This reduces operator burden and helps prevent page/text mismatches during review.

Metadata editing note:

- The metadata pane should include an edit button that opens a metadata-edit dialog.
- The dialog should provide explicit save and cancel actions.
- The dialog may show a base metadata set by default and allow optional bibliographic fields to be added or revealed.
- The dialog may allow the user to specify a cover page if the volume has one, supporting future bookshelf or visual browse views without assuming page 1 is always the cover.
- This is captured now as a usability reminder, not as a finalized UI design or framework decision.

Important edge cases to carry forward:

- If the current text page has unsaved edits, page navigation should either save safely, prompt, or block navigation until the edit state is resolved.
- Current stakeholder preference leans toward automatically saving current editor state when navigating to a new page.
- If the matching `pages/NNNN.txt` file does not exist, the UI should show a clear missing-page-text state rather than silently displaying stale text.
- If a page is `skip`, `empty`, or `quarantined`, the text editor behavior should be explicit and status-aware.
- The UI must not show text from the previously selected page after PDF navigation changes.

---

## 5. Questions For Requirements

- Is this mockup for the extractor itself, a separate reviewer tool, or both?
- Should the center panel render the original PDF page, a rasterized page image, or both depending on available tooling?
- Is the right panel expected to edit `pages/NNNN.txt`, show raw candidates, or switch between reviewed/embedded/OCR text?
- Should page navigation auto-save current text, and what visible save/restore behavior is needed to make auto-save safe?
- Where should `page-quality.json` flags and `review-state.json` status appear?
- Should the bottom strip show process logs, validation warnings, extraction progress, or all of these?
- Does the file tree represent source PDFs, generated work folders, or both?
- Should metadata include checksums and page counts before extraction begins?
- Which metadata fields are editable in the dialog, and which are displayed as read-only diagnostics?
- Should cover page selection be part of the base metadata dialog or an optional visual-library field?

---

## 6. Carry-Forward Guidance

Use the mockup as a conceptual guide for human review ergonomics, especially the side-by-side page/text workflow.

Do not use it yet as:

- a finalized UI specification
- proof that a specific native UI framework is required
- an architecture decision
- a replacement for the output-folder contract

The strongest requirement candidate implied by the mockup is not the exact layout. It is the need for an operator to inspect a source page, inspect or edit the corresponding text surrogate, see page/file identity, and move through pages predictably.

---

# 8. Updated Concept Wireframe

`docs/images/pdf-text-extractor-wireframe-v2.svg` carries the original workflow forward with requirements-driven changes:

- conventional toolbar or command surface
- icon-first toolbar buttons
- tooltip/accessibility expectation for icon-only controls
- synchronized PDF page and reviewed text page
- metadata edit button leading to save/cancel dialog behavior
- reviewed/embedded/OCR/compare tabs
- review status controls
- diagnostics/status strip
- theme status indication
- local tool/capability status in the status bar

This SVG remains a conceptual workflow wireframe, not final visual design.
