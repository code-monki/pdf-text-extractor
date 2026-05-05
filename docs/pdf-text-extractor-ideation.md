# PDF Text Extractor Project Primer

Project Name: pdf-text-extractor
Initiation Date (YYYY-MM-DD): 2026-04-30
Author(s): Chuck, Codex
Lifecycle Phase: Ideation - Approved for Requirements Transition
Version: 0.2
RTM Scaffold Created? No

---

# 1. Problem Statement

Local PDF corpora can contain valuable text that is difficult to extract reliably. Pages may be born-digital, scanned, OCR-layered, mixed, malformed, badly encoded, or hostile to extraction because of layout features such as tables, maps, forms, counters, and multi-column text.

The immediate originating need is to support downstream search/index construction for Classic Traveller static PDF browsing work in `/Users/chuck/ct-cd`. That downstream work cannot be trusted until page-level text extraction quality, coverage, diagnostics, and traceability are understood.

The cost of inaction is that downstream search and browser behavior may be built on incomplete, corrupted, duplicated, out-of-order, or untraceable text. That would hide extraction failures, create unreliable search results, increase manual review burden, and raise the risk of accidentally committing copyrighted source or derived full-text artifacts.

This project exists to reduce the extraction risk before downstream indexing, browser presentation, and packaging decisions depend on it.

---

# 2. Desired Outcomes

Conceptual success means the project can support local, reproducible, inspectable PDF text extraction for varied PDF collections.

Observable success conditions:

- source PDFs remain unmodified
- local PDF inputs can be inventoried without committing restricted content
- extraction can produce page-level text artifacts for every PDF page
- extraction gaps, OCR use, low-quality pages, and suspicious text are visible
- output destinations are configurable by the user
- downstream tools can consume explicit artifacts rather than extractor internals
- copyrighted PDFs and derived full-text outputs are treated as local-only artifacts unless explicitly approved

Conceptual acceptance criteria for Ideation:

- the extraction problem is clearly bounded
- stakeholders and integration boundaries are visible
- copyright and artifact-retention constraints are explicit
- extraction risks are surfaced
- alternative solution directions are identified
- open questions are carried forward without premature architectural decisions

---

# 3. Stakeholders

Primary stakeholders:

- The local operator/editor who runs extraction, reviews page text, and controls output destinations.
- The downstream `/Users/chuck/ct-cd` static PDF browser/search-indexing work that needs reliable page text.

Secondary stakeholders:

- Future users of the extractor against other local PDF corpora.
- Future browser/indexer or QA tools that may consume extraction outputs.

Decision makers:

- Chuck, as project owner and phase-gate approval authority.

Operational owners:

- Local development/operator environment owner.

Compliance stakeholders:

- Copyright and source-material handling concerns are active constraints. PDFs and derived full-text artifacts may contain copyrighted material and must be protected from accidental publication or commits.

---

# 4. Context and Environment

This repository was split from a broader static PDF browser/indexing effort. The original concept combined PDF browsing, indexing, packaging, and extraction. This repository narrows the focus to the extraction subsystem.

Existing context artifacts:

- `docs/static-pdf-browser-indexer-concept.md`
- `docs/extractor-output-contract-concept.md`
- `docs/pdf-text-extractor-mockup-review.md`
- `docs/images/pdf-text-extractor-mockup.png`
- `docs/feature-function-ideation-notes.md`
- `docs/ideation-risk-exploration-plan.md`
- `docs/tesseract-psm-ideation-notes.md`
- `docs/ocr-engine-evaluation-notes.md`
- `docs/reusable-volume-packaging-concept.md`
- `docs/ideation-stabilization-summary.md`

Important upstream/downstream context:

- `/Users/chuck/ct-cd` is expected to consume clean extraction outputs for search-index construction.
- A broader electronic-document library system is expected to need reusable, consistent volume formatting and package specifications beyond the narrower `/Users/chuck/ct-cd` need.
- Source PDFs and extracted text may be copyrighted and should remain local build inputs/artifacts.
- The browser/indexer should not depend on OCR tooling, embedded extraction details, or extractor-internal scoring logic.
- The first tested PDF is considered representative of the expected near-term corpus layout profile: table-heavy at times and occasionally two-column.

Operational environment:

- Local development on the user's machine.
- No hosted service, database, or network service is assumed.
- Git is used for source/governance artifacts, not source PDFs or derived full-text outputs.

---

# 5. Constraints

Technical constraints:

- Source PDFs must not be modified by extraction.
- Output paths must remain user-configurable.
- The extractor should be corpus-agnostic enough to support different PDF collections by changing content/configuration.
- Downstream consumers need explicit artifacts and diagnostics, not hidden extractor state.
- Extraction should operate locally.

Governance constraints:

- Ideation remained active until explicit approval to advance; that approval was granted on 2026-05-01.
- Future lifecycle phase advances still require explicit approval.
- Artifacts created in Ideation are non-binding unless later stabilized.
- Accepted ADRs are authoritative once they exist; none exist yet.
- Requirements, architecture, design, implementation, test planning, packaging, and documentation phases must not be skipped.

Copyright/source-material constraints:

- Source PDFs, OCR outputs, extracted page text, normalized text, raw candidates, and generated full-text indexes must stay local unless explicitly approved.
- Documentation and responses must not reproduce substantive PDF content.
- `.gitignore` must prevent accidental commits of source and derived text artifacts.

Resource constraints:

- The expected review workflow currently assumes a solo editor.
- Manual correction may be significant, so low-friction page review matters.
- Current stakeholder preference favors a GUI-centered correction workflow rather than a CLI-first product.
- Current stakeholder expectation is a native local application rather than a browser-based application.
- Native application rationale is tied to proposed functionality: reducing friction around local file access, PDF review, generated work folders, metadata editing, page navigation, and artifact management.
- Current stakeholder preference favors focused one-PDF-at-a-time extraction/review from the selected PDF in the file tree.
- A corpus-wide manifest is not currently considered necessary for this application.

---

# 6. Risks and Unknowns

High risks:

- Embedded text may be absent, corrupted, duplicated, out of order, or misleading.
- OCR quality may vary by scan quality, fonts, page images, layout, and image resolution.
- Derived full-text artifacts may accidentally expose copyrighted material if repository controls are incomplete.
- Downstream systems may receive incomplete or untraceable text if extraction failures are hidden.

Moderate risks:

- A work-folder contract may encode Classic Traveller-specific metadata too early.
- Review state may belong in the extractor, a separate reviewer tool, or only an output contract.
- Raw extraction candidates may be useful diagnostically but risky to retain or expose.
- Quality scores may create false confidence if not calibrated against real corpus samples.
- Page IDs, PDF page indexes, and printed page labels may be conflated.
- Printed document page labels are needed for downstream indexing/reference purposes but should not replace PDF page indexes as stable internal IDs.
- Visual inspection of representative OCR outputs supports the working assumption that most pages will require manual normalization before acceptance.
- Printed document page labels will likely need manual entry/correction rather than relying on OCR.
- External tool dependencies may reduce reproducibility if versions and diagnostics are not captured.
- Native UI expectations may prematurely constrain the extractor engine if engine boundaries are not preserved.
- One-PDF-at-a-time interaction may still need limited multi-file awareness for duplicate detection and readiness summaries.

Low risks:

- No ADRs exist yet, so architecture constraints are currently limited to governance and project instructions.
- The mockup may be misread as final design unless clearly labeled as non-binding input.

Unknowns:

- Which extraction tools will perform acceptably on representative PDFs.
- Whether the first experiment's findings transfer cleanly to a second representative PDF.
- How much page text review is required before downstream indexing is useful.
- Which initialized metadata fields should be user-editable versus read-only diagnostics.
- How the native reviewer UI should be scoped without coupling extraction logic to UI behavior.
- How HLA/design should componentize extraction logic, validation, review workflow, persistence, and UI responsibilities.
- Whether Poppler and Tesseract are sufficient candidate tools for the expected difficult PDFs.
- Whether alternate OCR engines are justified by quality gains despite install and reproducibility costs.
- Whether Tesseract page segmentation mode should be configurable, adaptive, or fixed by workflow profile.
- Whether the engine should run multiple Tesseract page segmentation modes behind the scenes and compare outcomes.
- What OCR confidence data is useful beyond character counts and flags.

---

# 7. Preliminary Non-Functional Considerations

Reproducibility:

- Extraction should be repeatable given the same source PDFs, configuration, tool versions, and review state.
- Diagnostics should capture enough context to explain extraction outcomes.

Auditability:

- Page-level extraction method, quality signals, errors, and review status should be inspectable.
- Failures should be explicit rather than silently ignored.
- Embedded/OCR character-count deltas greater than 5% are a candidate suspicious-text flag.

Maintainability:

- Extraction logic should avoid browser/indexer assumptions.
- Corpus-specific metadata should be isolated from reusable extraction behavior.

Security and copyright handling:

- Local-only restricted artifacts must be ignored by git by default.
- Full-text derivatives should be treated as build artifacts unless explicitly approved.

Performance:

- Performance budgets are not yet known.
- Early exploration should measure enough to avoid impractical workflows on real corpora.

Reliability:

- Empty pages, unreadable pages, OCR failures, low-confidence OCR, and page-count mismatches must be visible.

Usability:

- Review workflows should minimize page/text mismatches and reduce operator burden.
- The mockup suggests that selected PDF page and displayed text page should remain synchronized.
- Auto-save on page navigation is a strong workflow candidate, but needs visible save state and restore/backup protections.
- Text normalization should support useful snippets; tables embedded inside paragraph flow may need to be moved outside paragraphs in reviewed text.
- The reviewer workflow should make printed page-label entry low-friction while the operator is already normalizing each page.
- A straightforward page metadata model should associate the stable page text file, the PDF page number, and the printed document page label.

---

# 8. Deterministic-Probabilistic Awareness

OCR is an anticipated probabilistic or quality-variable subsystem. Its behavior may be affected by scan quality, image preprocessing, font shapes, layout, tables, maps, and noise.

Risk areas:

- OCR may produce plausible but wrong text.
- OCR confidence may not correlate cleanly with downstream search usefulness.
- Embedded text may appear deterministic while still being wrong or badly ordered.
- OCR fallback can improve coverage but introduces toolchain, reproducibility, and validation concerns.
- Raw embedded and OCR candidates improve recoverability but remain copyright-sensitive derived artifacts.
- Paper Capture PDFs should run OCR on all pages by default as a candidate workflow rule.

Containment questions:

- Should OCR results be kept separate from selected reviewed page text?
- What quality signals are enough to route pages for review?
- Should low-quality OCR output be quarantined by default?
- How should OCR tool names and versions be recorded?

No probabilistic architecture is authorized by this Ideation artifact.

---

# 9. Alternative Solution Directions

## Option A: CLI-first extraction engine plus work-folder contract

Concept:

- Build a local command-line extractor that inventories PDFs, extracts embedded text, optionally routes OCR, emits page text, diagnostics, and review-state files.
- Human review happens in normal editors or a later separate tool.

Advantages:

- Keeps core extraction reusable and testable.
- Avoids premature GUI design.
- Aligns with reproducible local automation.
- Provides downstream artifacts early.

Risks:

- Manual review may be awkward without a dedicated page/PDF comparison UI.
- Operator may have to coordinate PDF pages and text files manually.
- It may delay learning about reviewer ergonomics.

Open questions:

- How much review pain is acceptable for the first operational slice?
- Can existing file viewers support enough side-by-side review?

## Option B: Extractor engine with integrated reviewer UI

Concept:

- Build extraction and human review around a native local UI similar to the mockup: file tree, metadata, PDF/page preview, synchronized text editor, diagnostics/status area.
- Current stakeholder preference leans toward this direction because easy correction is the point of the project.

Advantages:

- Directly supports page-by-page review.
- Reduces risk of page/text mismatch.
- Can expose diagnostics and review state where the operator needs them.

Risks:

- May over-commit to native UI architecture before extraction risks are understood.
- UI complexity may slow extraction engine stabilization.
- Framework choice could dominate the project too early.

Open questions:

- Is the UI required for early risk reduction, or is it a later reviewer layer?
- Should the UI edit `pages/NNNN.txt`, inspect raw candidates, or both?

## Option C: Library/core plus separate CLI and reviewer surfaces

Concept:

- Define a reusable extraction core and local artifact contract, then expose it through a CLI and possibly a GUI/reviewer interface.
- This option can still support a GUI-centered product if the core is an internal boundary rather than a user-facing CLI-first workflow.

Advantages:

- Preserves separation between engine, review workflow, and downstream browser/indexer.
- Allows early CLI experiments without blocking future UI.
- Reduces coupling between extraction internals and human workflow.

Risks:

- May be too abstract too early if not grounded in real PDF experiments.
- Requires discipline to avoid over-generalizing before corpus needs are proven.
- Could create more moving parts than the first slice needs.

Open questions:

- What is the smallest useful core boundary?
- Which behavior belongs in core versus CLI versus reviewer UI?
- How can componentization support testability without producing a separate CLI-first application?

No option is selected during Ideation.

---

# 10. Preliminary Scope Boundaries

Likely in scope:

- local PDF inventory
- source PDF identity, checksums, and page counts
- embedded text extraction exploration
- OCR routing/fallback exploration
- page-level extraction outputs
- text normalization for page copy
- machine-readable diagnostics
- user-directed output paths
- local-only artifact handling
- output contract exploration for downstream consumers

Likely adjacent or conditional scope:

- review-state initialization and validation
- native reviewer UI workflow, with current stakeholder preference favoring inclusion in this project
- raw candidate retention policies
- printed document page label capture or user entry
- page preview/rasterization support for review
- toolchain/version diagnostics

Likely out of scope for this extractor:

- browser UI
- static search index generation
- hosted search service
- database-backed document management
- PDF editing
- source PDF preservation beyond non-modification
- rights-management enforcement
- canonical HTML reconstruction
- collection-specific browser theming or masthead behavior

Explicit downstream boundary:

- The browser/indexer may consume `volume.json`, `pages/*.txt`, `page-quality.json`, and `review-state.json`.
- The browser/indexer should not depend on OCR engine selection, embedded extraction tooling, raw candidate internals, scoring implementation, or corpus-specific extractor assumptions.

---

# 11. Candidate Work-Folder Contract

The following is a provisional concept, not a Requirements baseline:

```text
work/
  <volume-id>/
    volume.json
    page-quality.json
    review-state.json
    pages/
      0001.txt
      0002.txt
    raw/
      embedded/
        0001.txt
      ocr/
        0001.txt
```

Candidate meanings:

- `volume.json`: book-level metadata and indexing intent.
- `pages/NNNN.txt`: editable reviewed UTF-8 page text, one file per PDF page.
- `page-quality.json`: machine-maintained extraction diagnostics.
- `review-state.json`: human/editor workflow status by page.
- `raw/`: diagnostic raw candidates, local-only unless explicitly approved.

Candidate release/indexing rule:

- downstream release builds index only `accepted` pages by default
- `skip` pages are intentionally excluded
- `raw`, `editing`, `edited`, and `quarantined` pages are not release-indexed

Validation needed:

- user-editable versus read-only metadata fields
- schema strictness
- review-state ownership
- raw candidate retention policy
- relationship between PDF page index and printed page labels
- additional fields needed in `volume.json.pageMap` for printed document labels

---

# 12. Mockup-Derived Workflow Hypotheses

The mockup in `docs/images/pdf-text-extractor-mockup.png` is a conversation anchor, not a final UI decision.

Useful workflow hypotheses:

- operator sees a corpus/file list
- application work is centered on one selected PDF at a time
- selected file metadata is visible
- selected file metadata should have an edit action that opens a metadata dialog with save and cancel actions
- printed document page label can be captured or entered for indexing/reference purposes
- source PDF page and extracted/reviewed text are visible together
- page navigation is explicit
- selected PDF page drives the displayed text page
- if no PDF document/page is selected, the text editor is empty
- diagnostics, validation warnings, or process status are visible

Important edge cases:

- unsaved text edits must be handled before navigation changes pages
- auto-save on page navigation is a candidate policy, provided restore/backup protections exist
- missing `pages/NNNN.txt` must not display stale text
- `skip`, `empty`, and `quarantined` pages need explicit behavior

No native UI architecture or framework is selected by this hypothesis.

---

# 13. Candidate Exploration Experiments

These experiments are candidates for risk reduction before Requirements stabilization:

- inspect a small representative set of local PDFs without committing source or derived text
- classify pages by born-digital, scanned, OCR-layer, mixed, badly encoded, and layout-hostile cases
- run embedded text extraction against easy and difficult pages
- test whether extracted text order and character quality can be detected with simple diagnostics
- prototype OCR routing on image-only or suspicious pages
- run OCR in early experiments because the expected corpora are likely OCR-heavy
- compare embedded and OCR candidates using counts, flags, and manual spot checks without quoting source content
- evaluate the proposed greater-than-5% embedded/OCR delta threshold across additional samples
- validate that the candidate work-folder contract can represent real pages, empty pages, failures, and quarantined pages
- verify `.gitignore` blocks source PDFs and derived text artifacts
- validate auto-save, restore-from-raw, and do-not-overwrite-reviewed-text behavior with a harmless sample
- run the first concrete risk experiment defined in `docs/ideation-risk-exploration-plan.md`

Experiment outputs should use filenames, hashes, page counts, flags, and minimal metadata. They should not reproduce substantive PDF contents in documentation or responses.

---

# 14. Lifecycle Readiness Assessment

Current status:

- Problem is clearly articulated for Ideation purposes.
- Desired outcomes are defined at a conceptual level.
- Stakeholders are identified.
- Constraints are documented.
- Risks are surfaced and classified.
- Alternative approaches are explored.
- Representative extraction/OCR risks have been explored.
- Output contract concepts have been captured.
- UI workflow hypotheses have been captured without selecting design or architecture.
- Broader reusable packaging concepts have been captured as adjacent/downstream context.
- Architectural commitments have not been made.
- RTM scaffold has not been created.
- Requirements phase has been authorized by the project owner.

Requirements-phase inputs to carry forward:

- formalize the work-folder output contract
- define required versus optional metadata fields
- define user-editable versus read-only metadata
- define review workflow behavior and status transitions
- define extraction/OCR routing and candidate-retention behavior
- define quality flags, thresholds, and validation behavior
- define native UI workflow requirements without selecting a UI architecture
- define packaging/export ownership boundaries
- decide whether an RTM scaffold is created at Requirements entry

Ideation phase-gate review is complete. Requirements phase is authorized.

---

# 15. Gate Declaration

Ideation Stabilized for Gate Review: Yes

Approved to Proceed to Requirements Phase: Yes

Approval name: Chuck
Approval role: Project owner
Approval date: 2026-05-01
Signature: Conversation approval

Version incremented for Ideation stabilization.
