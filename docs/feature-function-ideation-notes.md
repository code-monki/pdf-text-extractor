# Feature and Function Ideation Notes

Status: Ideation Note
Date: 2026-04-30
Lifecycle Phase: Ideation

---

## 1. Purpose

This note captures stakeholder responses to candidate feature/function ideas for the PDF text extractor.

These notes are not approved requirements. They are current Ideation inputs to help avoid blind ideation and identify where later Requirements work needs sharper decisions.

---

## 2. Current Stakeholder Preferences

### Input And Inventory

- The application should focus on one selected PDF at a time.
- PDF selection should be driven by the file tree.
- Internal work-folder mechanics should not be exposed unnecessarily to the user.
- Duplicate PDF detection is useful, but the user should decide whether to keep or remove duplicates.
- File hash, file size, modified time, page count, and readable/unreadable status are useful metadata.
- Encrypted, damaged, password-protected, and malformed PDFs should be flagged.
- Detecting whether a PDF contains embedded text, images, or both is not valuable as a primary user-facing concern unless it affects extraction routing or diagnostics.

### Extraction Control

- Extraction is expected to be a one-PDF-at-a-time process.
- Re-running extraction for one page or one volume may be useful when a page is corrupted or the user wants to restart an editing pass.
- The preferred extraction flow is:
  - attempt embedded text extraction when a text layer is available
  - if embedded text is unacceptable to the user, provide a path to run OCR
  - if no text layer exists, proceed immediately to OCR
- Raw candidates should be preserved separately from reviewed text.

### Page-Level Review

- Synchronized PDF page and text page behavior is desired.
- First, previous, next, last, and direct page jump navigation are desired.
- Page status values are useful: `raw`, `editing`, `edited`, `accepted`, `skip`, and `quarantined`.
- Per-page reviewer notes are useful.
- Keyboard shortcuts for accept, skip, and next page are useful.

### Diagnostics

- Embedded character count versus OCR character count is useful.
- Quality flags are useful.
- Pages should be highlighted when they are empty, short, symbol-heavy, numeric-heavy, Unicode-suspect, extraction-error cases, or likely reading-order problem cases.
- Corpus or volume readiness summaries are useful, including accepted, skipped, raw, quarantined, and OCR-used counts.
- Quality reports should be exportable without full copyrighted text.

### Text Handling

- Unicode should be normalized where possible.
- Control characters should be stripped or flagged.
- Legitimate odd or international characters should be preserved.
- Tables should be flattened into searchable text where possible.
- When a table appears inside a paragraph, moving the table outside the paragraph may better support snippet generation.
- Manual replacement of machine output should be supported.
- Embedded, OCR, and reviewed text comparison is useful.
- Reviewed text should be restorable from raw candidates if needed.
- Every PDF page should have a corresponding `pages/NNNN.txt`.

### Output Contract

- `volume.json`, `page-quality.json`, and `review-state.json` should be generated or updated.
- Missing page files and page-count mismatches should be detected.
- Downstream release indexing should see only `accepted` pages by default.

### Workflow And Safety

- Dry-run mode is useful.
- Validate-only mode is useful.
- Reviewed text should not be overwritten accidentally.
- Backup before bulk regeneration is useful.
- Generated, reviewed, and raw diagnostic files should be clearly distinguished.
- Deleting or regenerating local artifacts should require an explicit warning.
- `.gitignore` coverage for source PDFs and derived text should remain verified.

### Downstream Integration

- A corpus-wide manifest is not currently considered necessary for this application.
- Per-volume metadata and downstream-consumable work folders are expected to be sufficient unless later evidence shows otherwise.
- `volumeId` and page IDs should be stable.
- Downstream tools should not need to know OCR internals.
- The `/Users/chuck/ct-cd` project is expected to look for the inputs to its indexer rather than a separate special-purpose export summary unless later evidence shows value.

### Larger Direction

- The project should not be CLI-first because the project purpose is to facilitate easy correction.
- GUI-centered workflow does not imply UI-coupled internals; extraction logic is expected to become a separate component during HLA/design.
- The pushback against CLI-first is specifically against building a separate CLI-oriented application just to expose operations.
- The reviewer GUI is in scope for this project, but likely not the first Spiral risk-reduction target because it depends on extraction infrastructure.
- OCR should be used when the user requests it or when the PDF has no usable text layer.
- OCR should be included in early risk exploration because expected corpora are likely OCR-heavy.
- For Paper Capture PDFs, OCR should run on all pages by default.
- Poppler is a candidate embedded text extraction tool and Tesseract is a candidate OCR tool for early exploration, subject to later dependency evaluation.
- The project owner is willing to install alternate OCR engines if evidence shows they are needed.
- Raw candidates should be retained by default, with a manual cleanup option controlled by the user.
- The first tested PDF layout is representative of expected near-term volumes: table-heavy at times and occasionally two-column.

---

## 3. Clarifications And Explanations

### 3.1 Corpus Manifest

A corpus manifest is valuable only if the tool needs a stable list of known source PDFs and their identities separate from current extraction outputs.

Potential value:

- detects when a source PDF was renamed, moved, replaced, or duplicated
- records source file identity before extraction begins
- gives downstream tools a source inventory without reading extraction work folders
- supports validation that every intended source file has an extraction workspace

Counterpoint:

- If this tool is truly one-PDF-at-a-time and the downstream indexer only consumes completed work folders, a separate corpus manifest may be unnecessary early on.

Current Ideation position:

- Do not include a corpus-wide manifest in the current application concept.
- Revisit only if Requirements identify a concrete need that cannot be met by per-volume `volume.json` and downstream-consumable work folders.

### 3.2 Per-Page Extraction Method Override

The proposed override means a user could tell one page to use a different source than the default selected candidate.

Examples:

- use OCR for page 17 even though embedded text exists
- use embedded text for page 18 even though OCR was also generated
- mark page 19 as manual because the user replaced the machine output
- mark page 20 as empty or quarantined

Current Ideation position:

- This may not need a separate visible "override" feature. It may naturally fall out of compare/replace actions and `selectedSource` metadata.
- Keep friction low. Revisit manual override behavior during Requirements or Design once compare/replace workflow is clearer.

### 3.3 PDF Page Index Versus Printed Page Label

PDF page index means the physical page position in the PDF: page 1, page 2, page 3.

Printed page label means what the page itself says, which may be different:

- cover page may have no printed number
- front matter may use roman numerals
- book page "1" may be PDF page 7
- some scans may have missing or repeated printed numbers

Current Ideation position:

- PDF page index should be the stable identifier for `pages/NNNN.txt`.
- Printed page labels are needed for indexing/reference purposes and should be capturable or user-enterable.
- Printed page labels should be best-effort during initial extraction/OCR and user-overridable afterward.
- Printed page labels should not replace PDF page indexes as stable internal IDs.

### 3.4 Unsaved Edit State

Unsaved edit state means the text editor contains changes that have not yet been written to `pages/NNNN.txt`.

Possible navigation policies:

- Auto-save on page change.
- Prompt before page change.
- Block navigation until Save or Revert.

Current stakeholder preference leans toward auto-saving when the user navigates to a new page.

Risk:

- Auto-save is efficient, but a mistaken edit can be persisted silently unless undo, restore-from-raw, or backup behavior is strong.

Current Ideation position:

- Auto-save is a strong candidate, but it should be paired with visible save status, restore-from-raw, and backup/regeneration protections.

### 3.5 Jump To Next Flagged Or Unreviewed Page

This means navigation commands such as:

- next unreviewed page
- next page with quality flags
- next quarantined page
- next extraction error

Current Ideation position:

- The primary workflow likely still requires working through every page, especially for Classic Traveller and similar follow-on corpora where most PDFs are expected to lack usable text layers and need OCR.
- Jump/filter behavior may still be useful for returning to errors, quarantined pages, or review cleanup, but it is secondary to full sequential review.

### 3.6 Work Folder Validation

Work-folder validation means checking the generated output contract for internal consistency without re-extracting text.

Potential checks:

- `volume.json` exists and has valid JSON
- `page-quality.json` exists and has valid JSON
- `review-state.json` exists and has valid JSON
- `volumeId` matches across JSON files
- `sourcePdf.pageCount` matches expected page entries
- every PDF page has `pages/NNNN.txt`
- every page file has review-state and quality entries
- no unexpected missing raw candidate references
- review statuses are from the allowed set
- release/export mode excludes non-`accepted` pages
- page IDs are zero-padded and stable

Current Ideation position:

- This is high-value and should likely become a Requirements candidate.

### 3.7 Machine-Readable Diagnostics

Machine-readable diagnostics means structured JSON output that another tool can parse, not just human-readable logs.

Potential value:

- downstream indexer can reject unsafe or incomplete inputs automatically
- validation can fail on missing pages or non-accepted pages without scraping text logs
- reports can be generated consistently
- future tests can compare expected issue counts and flags

Counterpoint:

- If `page-quality.json`, `review-state.json`, and validation output already cover this, a separate diagnostics file may be redundant.

Current Ideation position:

- Do not create a separate diagnostics channel unless needed. Prefer using `page-quality.json`, `review-state.json`, and a structured validation report.
- Revisit if downstream integration or test planning reveals a need for a stronger machine-readable diagnostics artifact.

### 3.8 Review State Ownership

Review state ownership asks which part of the system is allowed to create and change `review-state.json`.

Possible models:

- extractor owns it entirely: creates statuses and updates them as the user reviews pages
- reviewer UI owns it: extraction initializes it, UI updates it
- external editor owns it manually: extractor only validates it

Current stakeholder direction suggests the app should own review state because easy correction is central to the project.

Open question:

- Even if the app owns review state, should the extraction engine itself mutate review status, or should only the review workflow mutate it?

Current Ideation position:

- The application likely owns review state.
- HLA/design should separate extraction logic from UI/review workflow responsibilities.

### 3.9 Embedded/OCR Delta Threshold

Character-count deltas between embedded text and OCR candidates are useful as review flags.

Current Ideation position:

- Mark embedded/OCR character-count deltas greater than 5% as suspicious.
- Treat the threshold as a candidate Requirements input subject to validation against more PDFs.

### 3.10 Tesseract Page Segmentation Mode

Tesseract page segmentation mode may materially affect OCR quality.

Options to discuss:

- configurable mode: gives control but can add friction and testing burden
- adaptive mode: reduces user burden but requires reliable page classification or fallback behavior
- fixed workflow mode: simplest but may perform poorly across mixed layouts
- hidden multi-candidate mode: runs multiple modes behind the scenes and compares outcomes without asking the user to choose

Current Ideation position:

- Keep this open for Requirements/Design discussion.

### 3.11 OCR Confidence Data

OCR confidence data may help assess quality, but its practical value is unclear without comparing it to review outcomes.

Current Ideation position:

- Character counts and flags are known useful signals.
- Revisit confidence data once OCR outputs are compared with human review results.
- Expanded OCR comparison showed confidence is useful but not sufficient as a sole selector because it can disagree with other quality metrics.

### 3.11.1 Ground-Truth Comparison

Five local ground-truth pages were supplied for representative page types.

Current Ideation findings:

- Embedded text won several pages against ground truth.
- Direct Tesseract / PSM `6` won some table-heavy pages.
- Sparse-text modes may preserve tokens while hurting sequence-sensitive metrics.
- OCRmyPDF did not outperform direct Tesseract or embedded text on the first sample.

Current implication:

- Candidate selection should compare embedded and OCR candidates rather than assuming OCR is always better.
- Ground-truth comparison should remain part of OCR evaluation.

### 3.12 Tables Inside Paragraphs

Tables embedded inside paragraph flow may reduce snippet quality by interrupting prose context.

Current Ideation position:

- When a table appears inside a paragraph, consider normalizing the reviewed text so the table is outside the paragraph.
- This is not layout reconstruction. It is search/snippet-oriented text shaping.
- The goal is to keep prose snippets readable while still preserving table terms for discovery.

Open question:

- Should the app provide any table-handling affordance, or should this remain a manual reviewer convention?

### 3.13 Tesseract Page Segmentation Modes

Tesseract page segmentation mode needs more detailed Requirements/Design discussion.

Reference:

- `docs/tesseract-psm-ideation-notes.md`

Current Ideation position:

- Do not expose all Tesseract modes as a primary workflow control.
- Record the selected mode in diagnostics.
- Evaluate a simple default, hidden multi-mode comparison for suspicious pages, and possible advanced/manual re-OCR behavior.
- Prefer non-OSD modes by default for normally oriented pages unless orientation uncertainty is detected.

---

## 4. Assumptions To Challenge

### GUI-Centered Workflow

The project purpose is easy correction, so a GUI-centered workflow is sensible.

Challenge:

- A GUI-centered product does not require extraction logic and UI logic to be tightly coupled. HLA/design should apply componentization principles so extraction, validation, dry-run, backup, and overwrite protection can be tested without driving the GUI.

### One-PDF-At-A-Time Operation

One-PDF-at-a-time operation matches focused review.

Challenge:

- Some non-interactive functions may still need limited multi-file awareness, especially duplicate detection and readiness summaries. A corpus-wide manifest is not currently justified.

### Embedded Text Detection Is Immaterial

It may be immaterial as a user-facing display concept.

Challenge:

- It is still material as a routing and diagnostics concept because it determines whether the tool attempts embedded extraction first, proceeds to OCR, or explains why OCR was needed.

### Auto-Save On Navigation

Auto-save reduces friction and prevents losing edits.

Challenge:

- It can also preserve accidental edits. This needs visible save state, undo/revert, raw-candidate restore, and backups before bulk regeneration.

### Raw Candidate Retention

Retaining raw candidates improves recoverability and diagnostics.

Challenge:

- Raw candidates are derived full-text artifacts and therefore copyright-sensitive. Retention should remain local-only, ignored by git, and removable via explicit cleanup.

---

## 5. Candidate Requirements Inputs

The following items appear strong enough to carry forward into Requirements discussion:

- The app operates on one selected PDF at a time for extraction/review.
- Duplicate detection is advisory; the user decides what to keep or remove.
- File identity metadata includes hash, size, modified time, page count, and readability status.
- The app flags encrypted, damaged, password-protected, and malformed PDFs.
- Embedded text extraction is attempted when available; OCR is used when no text layer exists or when the user requests OCR after judging embedded text unacceptable.
- Raw embedded and OCR candidates are retained separately from reviewed text by default.
- The PDF page and displayed text page stay synchronized.
- The app supports page statuses, per-page notes, keyboard shortcuts, and page-level diagnostics.
- The app supports capture or user entry of printed document page labels for indexing/reference purposes.
- Printed document page labels are best-effort extraction/OCR metadata and user-overridable.
- Embedded/OCR character-count deltas greater than 5% should be flagged as suspicious.
- OCR should run on all pages by default for Paper Capture PDFs.
- Reviewed text may move tables outside paragraph flow where that improves snippet usefulness.
- OCR diagnostics should record the Tesseract page segmentation mode used.
- The app validates that each PDF page has a `pages/NNNN.txt` file.
- The app validates the work-folder contract.
- The app protects reviewed text from accidental overwrite.
- The app supports dry-run and validate-only behavior.
- Release/downstream indexing uses only `accepted` pages by default.
- Raw candidates remain local-only and are removable by explicit user cleanup.
- A corpus-wide manifest is not part of the current application concept.