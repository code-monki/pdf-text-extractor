# Static PDF Browser and Reusable Indexer Concept

Status: Ideation Draft  
Date: 2026-04-30  
Lifecycle Phase: Ideation  
Primary Corpus: Classic Traveller canon PDFs in `pdfs/`  
Governance Reference: `ai-toolkit/02-governance/01-ideation-guardrail.md`

---

## 1. Concept Summary

This project will explore a reusable static PDF browser and search-indexing system.

The system will allow a collection of PDFs to be distributed with a browser-only shell that provides catalog browsing, full-text search, theming, and collection-specific masthead/verbiage while preserving the original PDFs as the authoritative reading and printing artifacts.

The most valuable system capability is not the browser shell by itself. The core value is the ability to create an accurate, reusable, page-aware search index from noisy PDF source material.

---

## 2. Problem Statement

The Classic Traveller canon PDF corpus is large, historically valuable, and likely noisy for text extraction. The PDFs include scanned pages, OCR text layers, images, maps, forms, tables, counters, and layout-heavy content.

Native browser PDF readers are better suited than reconstructed HTML for reading, zooming, and printing. However, native PDF search across a distributed folder is poor or unavailable when running from local media. A browser-only discovery layer is needed.

The system should make the corpus searchable without requiring:

- a database
- a local server
- network access
- canonical HTML reconstruction
- modification of source PDFs

---

## 3. Product Decomposition

Conceptually, the product consists of:

1. PDF Corpus
2. PDF Scanner / Text Extraction Pipeline
3. Editable Per-Page Text Surrogates
4. Indexer
5. Browser Shell Page
6. Distribution Package

The editable per-page text layer is explicit because OCR and embedded PDF text are expected to be noisy. Manual correction is part of the expected workflow.

---

## 4. Source PDF Principle

The source PDFs remain the authoritative reading and printing artifacts.

The browser shell shall not attempt to become a PDF renderer, layout engine, or canonical republication format. It should provide discovery and navigation support only:

- browse volumes
- search indexed text
- display search result metadata
- display snippets
- open source PDFs

Reading, rendering, zooming, and printing are delegated to the browser or native PDF reader.

---

## 5. Text Surrogate Principle

The reviewed text layer exists only to support search, snippets, and page references.

It is not intended to reproduce visual layout, table structure, images, or page design. A page text surrogate is acceptable when it is good enough for finding the page by likely search terms.

In scope:

- headings
- body text
- important names
- rules terms
- table labels and meaningful table text
- map labels when useful for discovery

Out of scope:

- table layout fidelity
- image reproduction
- counters as visual artifacts
- diagrams as diagrams
- canonical HTML reconstruction

---

## 6. PDF Text Extraction Use Cases

The extraction pipeline should account for multiple PDF text conditions rather than assuming a simple text-versus-image split.

Primary use cases:

1. Born-digital or normal text PDFs
  - PDF contains real extractable text.
  - Text extraction tools may be sufficient.
2. Scanned image PDFs
  - Pages are essentially images.
  - OCR is required to produce useful text.

Additional cases that materially affect design:

1. OCR-layer PDFs
  - Page image includes a hidden OCR text layer.
  - Text may be extractable but noisy, incomplete, or misordered.
2. Mixed PDFs
  - A single volume may contain both extractable text pages and image-only pages.
  - Extraction decisions should therefore be page-level where practical.
3. Badly encoded text PDFs
  - PDF text exists, but extraction may produce garbage due to font encoding, missing Unicode maps, ligatures, spacing problems, or reading-order defects.
  - OCR fallback may be useful even when embedded text technically exists.
4. Layout-hostile PDFs
  - Tables, forms, maps, counters, multi-column pages, and other layout-heavy content may extract text in poor order.
  - These pages may require review and cleanup rather than full layout reconstruction.

Conceptual extraction flow:

1. Attempt embedded text extraction.
2. Score extracted text quality.
3. Run OCR when extraction is empty, low-quality, suspicious, or explicitly requested.
4. Select the best machine-generated candidate for `pages/NNNN.txt`.
5. Preserve raw extraction candidates for diagnostics when useful.
6. Record extraction method, quality signals, and review status per page.

---

## 7. Proposed Work Folder Model

The scanner should produce a per-volume working folder containing editable page text and machine-maintained metadata.

Example:

```text
work/
  ct-b01-characters-combat-1981/
    volume.json
    page-quality.json
    review-state.json
    pages/
      0001.txt
      0002.txt
      0003.txt
```

Expected responsibilities:

- `pages/*.txt`: human-editable page text used for indexing.
- `page-quality.json`: machine-maintained extraction quality signals by page.
- `review-state.json`: solo-editor progress state by page.
- `volume.json`: source filename, title, page count, collection metadata, and index inclusion settings.

Source PDFs should remain under the corpus folder and should not be modified during indexing unless explicitly authorized.

---

## 8. Solo Editor Workflow

The workflow assumes one editor who may review every page.

The system should optimize for low friction:

- one editable text file per page
- simple statuses
- resumable per-volume review
- quality flags as guidance, not bureaucracy
- packaging warnings rather than unnecessary process blockage

Suggested page statuses:

- `raw`: machine-generated and not checked
- `editing`: currently being reviewed
- `edited`: manually changed but not final
- `accepted`: good enough for indexing
- `skip`: intentionally excluded from indexing

The expected final state for a fully reviewed volume is that every page is either `accepted` or `skip`.

---

## 9. Search Result Metadata

Unlike the GJTAS browser, which searched article-level units, this corpus should search page-level units.

Each search hit should be able to display:

- volume title
- source PDF path
- page number
- snippet
- open-PDF action

Conceptual result shape:

```json
{
  "volumeId": "ct-b01-characters-combat-1981",
  "volumeTitle": "Book 1: Characters & Combat (1981)",
  "pdfPath": "pdfs/CT B01 Book 01 Characters & Combat 1981.pdf",
  "page": 17,
  "snippet": "...survival, commission, promotion..."
}
```

The system should not depend on PDF fragment navigation such as `file.pdf#page=17`. Search results should display the page number explicitly so the user can navigate manually in the native PDF reader.

---

## 10. Reuse From GJTAS PDF Browser

The `/Users/chuck/gjtas-cd` project provides reusable concepts and possible code assets:

- single-file browser shell
- Vue-based catalog browsing UI
- theming
- modal search interface
- boolean query parser
- stemmed inverted search index
- script-loaded `search-index.js` that works under `file://`
- zip packaging layout

Known mismatch:

- GJTAS indexed generated HTML articles.
- This project must index reviewed page-level text surrogates derived from PDFs.

The reusable system should retain the browser-only distribution pattern while replacing the content pipeline with PDF scanning, editable text review, and page-granular indexing.

---

## 11. Collection Configuration

The system should be collection-agnostic. The expected differences between Traveller collections should be handled through configuration rather than code changes where feasible.

Likely configurable items:

- collection name
- masthead image
- masthead text
- about/help text
- menu labels
- volume grouping labels
- theme defaults
- corpus folder path
- output package name

The current assumption is that masthead and associated verbiage are the main differences between versions, but the concept should allow modest collection metadata variation without over-generalizing.

---

## 12. Quality Reporting

The scanner/indexer should produce a quality report so the editor can understand corpus readiness.

Potential quality signals:

- empty or very short extracted pages
- high symbol ratio
- high numeric ratio
- OCR fallback usage
- extraction errors
- page count mismatches
- suspected duplicate files
- unreviewed pages
- skipped pages

Quality reports should support review and packaging decisions. They should not silently block work unless later Requirements define explicit gates.

---

## 13. Waiting Room

### WR-001: Bookshelf View

Explore an optional visual browse mode using cover images after core search, page-level snippets, reviewed text workflow, and packaging are stable.

Do not include this in MVP requirements unless the stakeholder confirms it materially improves browsing.

Open feasibility questions:

- Should covers be extracted from page 1 or manually supplied?
- Does the view improve discovery or only add visual appeal?
- Does it complicate package size, build time, or review workflow?
- How should it behave when a volume has no useful cover image?

---

## 14. Preliminary Risks

- OCR and embedded text may be noisy, incomplete, or misleading.
- Some PDFs may be malformed or difficult for extraction tools.
- Table-heavy and map-heavy pages may produce poor text order.
- The manual review effort may be substantial.
- Search index size may grow large if snippets or page text are over-embedded.
- Browser PDF page deep links are unreliable and should not be required.
- Reusing GJTAS browser code may import article-centric assumptions.
- Over-generalizing for future corpora too early could slow the first usable package.

---

## 15. Additional Ideation Concerns

### 15.1 Page Numbering

Most source pages appear to have visible printed page numbers. The system may be able to capture those during parsing or review.

Important distinction:

- PDF page index supports locating the page in the source PDF.
- Printed page number may better match the volume's own internal references.

The initial concept should preserve PDF page index as the reliable locator. Printed page labels may be captured when feasible, but should not be required unless later analysis proves extraction/review is reliable enough.

### 15.2 Review Tooling

The existing `skrat` file viewer may provide a lower-friction path for reviewing extracted page text alongside PDFs, images, or related files.

Repository reference:

```text
https://github.com/code-monki/skrat
```

This should be treated as a candidate review aid, not as a selected dependency during Ideation. If used later, it should be evaluated during Requirements or Design for fit, setup burden, and whether it keeps the browser/indexer workflow reusable.

The text extraction and correction workflow may justify a standalone extractor/reviewer application, but that tool should be treated as a separate project from the browser/indexer deliverable.

Conceptual standalone tool:

- display the source PDF page in one pane
- display the extracted text for that page in an editable pane
- allow page-by-page navigation
- show extraction method and quality flags
- allow page status changes such as `raw`, `editing`, `accepted`, `skip`, and `quarantined`
- write reviewed UTF-8 page text back to the work folder
- preserve raw extraction candidates separately for diagnostics

Qt may be a practical implementation option for this kind of desktop review tool because side-by-side PDF display and editable text panes are straightforward in that environment.

Scope boundary: the standalone review tool has a definite scope and can be useful outside this project. It should not be absorbed into the browser project unless later explicitly authorized.

For this project, the browser/indexer should define and consume the file contracts that such a tool would read and write, such as work-folder layout, page text files, review status, quality metadata, and raw candidate locations.

### 15.3 Post-Index Validation

Post-index checks are expected to be important acceptance criteria and test-plan material.

Potential validation areas:

- generated JavaScript resources parse successfully
- expected browser globals or registries are present
- catalog entries resolve to actual PDFs
- page-text resources resolve to catalog page IDs
- search index page IDs resolve to catalog entries
- release builds exclude unreviewed pages
- skipped pages are intentionally excluded
- package layout contains required runtime resources
- no forbidden source corpus or derived full-text artifacts are accidentally committed

Automation should be explored where it reduces review burden or catches regressions early.

### 15.4 Copyright, Repository Privacy, and CI

The PDFs and derived text are copyrighted material. The repository is expected to remain private if GitHub CI is used.

Private repository use does not remove the need to keep copyright handling explicit:

- `pdfs/` remains ignored by git for now.
- generated extraction caches remain local build artifacts unless explicitly approved otherwise.
- CI workflows should avoid publishing source PDFs or extracted full-text artifacts.
- Any assumptions about third-party platform use of private repository contents should be treated as external platform risk and verified against current platform terms before relying on them.

Current preference: avoid GitHub entirely if practical.

The product is essentially a static distribution consisting of HTML, JavaScript, PDFs, generated indexes, and supporting assets. A hosted CI pipeline may not provide enough value to justify pushing copyrighted source or derived text artifacts to GitHub, even in a private repository.

The editor expects to have access to a self-hosted Gitea instance in a home lab environment. Self-hosted Gitea or local build automation should therefore be considered lower-risk options for orchestration, packaging, and validation.

GitHub CI should remain a non-preferred option unless later requirements identify a concrete benefit that outweighs the copyright and platform-risk concerns.

### 15.5 Candidate Non-Functional Requirements

The following items should be carried forward as SRS inputs.

Accessibility:

- Browser UI should target WCAG 2.2 AA conformance.
- Selected WCAG 2.2 AAA practices may be adopted where practical, but AAA conformance should not be a release gate.

Browser support:

- Google Chrome 147
- Microsoft Edge 147
- Firefox 149
- Safari 26.4

Performance:

- Performance budgets are not determined during Ideation because baseline metrics are not yet available.
- Requirements should defer specific numeric budgets until measurement data exists.

Build reproducibility:

- Builds should be deterministic.
- Given the same reviewed text, source corpus, configuration, and build inputs, generated catalog/index/package artifacts should be reproducible except for explicitly versioned or timestamped metadata.

Versioning and upgrade model:

- The packaged deliverable is expected to be built by a top-level `Makefile` acting as the project orchestrator.
- Browser shell, collection package, and index format components are expected to remain synchronized within each built package.
- Individual runtime pieces are not expected to be distributed separately.
- Upgrade guidance is expected to be remove the older installation and/or overlay it with the new installation.

Disaster recovery:

- Reviewed text and project assets are expected to be protected by a git repository and nightly backups on the development system.

Manifest:

- A manifest should exist to define or override collection metadata, titles, subtitles, ordering, grouping, skip flags, package labels, and related collection configuration.
- The Classic Traveller corpus should include the grouping structure shown in the existing `pdfs/index.html`.

Search behavior:

- Search query behavior should match the GJTAS PDF browser unless later analysis identifies a reason to diverge.

Character handling:

- The base character set should be UTF-8.
- International and odd characters should be treated as normal text, with validation/sanitization focused on malformed or unsafe character sequences rather than stripping legitimate characters.

Validation UX:

- Validation reports should be readable and actionable for the solo editor, not only machine-readable.

No silent degradation:

- Runtime failures should be visible to the user where they affect behavior.
- If page-text resources fail to load, search results should indicate that snippets are unavailable rather than silently omitting them.
- If an index, catalog, page-text resource, or PDF reference is missing or invalid, the browser/build validation should report it explicitly.
- Degraded behavior may be acceptable in some cases, but hidden degradation is not acceptable.

Documentation:

- Packaged user help should explain search syntax, page-number behavior, opening PDFs, theme controls, and known limitations.
- Builder/operator documentation should explain scanning PDFs, reviewing pages, building indexes, validating outputs, and packaging a collection.
- Collection notes should explain corpus provenance, edition label, known extraction limitations, and review status.
- Documentation should be treated as release-gating support material during later lifecycle phases, not optional polish.

---

## 16. Ideation Clarifications

The following questions were reviewed during Ideation and have provisional answers. These are concept-level clarifications, not formal requirements.

### 16.1 Page Review Status for Packaging

The minimum acceptable page review status for packaging is expected to be `accepted` for indexable pages.

Rationale: the editor expects to review every page before release. Pages that should not be indexed may be marked `skip`.

### 16.2 Unreviewed Pages in Builds

Release and packaging builds should not index unreviewed pages.

Prototype or development builds may optionally index raw pages when explicitly requested, but this should be disabled by default.

If raw pages are indexed during development, they must first pass validation and sanitization intended to catch character sequences that could break generated JavaScript resources, search parsing, browser rendering, or packaging.

Raw development results must be clearly labeled as unreviewed.

Rationale: search quality depends on reviewed text, so release outputs should not include raw OCR text. Development indexing of raw pages can reveal bad OCR, missing pages, bad titles, poor tokenization, and other extraction defects earlier in the workflow. However, unreviewed OCR can contain unexpected character sequences, malformed Unicode, control characters, broken ligatures, invalid encodings, or delimiter-like text that could trigger failures far away from the extraction step. Those failures can be expensive to diagnose if raw text is admitted into generated browser resources without validation.

Conceptual mitigation:

- normalize Unicode before generating artifacts
- remove or escape control characters
- serialize generated JavaScript data using structured serialization rather than string concatenation
- validate generated resources before browser testing
- quarantine pages that fail text validation
- report suspect character patterns in the quality report
- require explicit opt-in for raw development indexing

### 16.3 Snippet Generation

Snippets should likely be generated at query time rather than embedded directly in the search index.

Rationale: embedding snippets in the index could bloat `search-index.js`. Query-time snippets keep the inverted index smaller, provided the browser has access to enough page text or compact page-level text resources.

Local data loading constraint: the editor has not successfully gotten `fetch()` to work without a web server. The browser package should therefore assume local data resources must be script-loadable rather than fetched as arbitrary `.txt` or `.json` files.

Related constraint: the browser File API should not be assumed to solve local data loading. In the editor's experience, modern browser File API behavior still runs into practical sandboxing constraints for this distribution model.

Clarification: user-mediated file selection can provide local file access, but that is a manual upload-style workflow. It does not satisfy the goal of a self-contained browser package that automatically loads its catalog, index, and page-text resources from the distribution folder.

Working assumption: reviewed page text should be packaged as generated JavaScript resources, possibly one resource per volume or chunk, and loaded on demand for snippet generation.

Example concept:

```text
catalog.js
search-index.js
page-text/
  ct-b01.js
  ct-b02.js
```

Each page-text resource could attach reviewed text to a known browser global or equivalent local-safe data registry. This preserves the GJTAS lesson that browser-only local distribution works best when runtime data is loaded as script resources.

### 16.4 Search Result Presentation

Search results should be displayed in a modal dialog, following the interaction pattern used by the GJTAS PDF browser.

Rationale: search is a focused discovery workflow and should not displace the main catalog browsing context.

### 16.5 Volume Grouping Metadata

Each volume has a unique title/subtitle, which is expected to be sufficient for identity.

Optional grouping metadata may still be useful for navigation, such as Books, Supplements, Adventures, Double Adventures, Alien Modules, and Games.

Rationale: title/subtitle may be enough to identify a volume, but the catalog experience may benefit from explicit groupings already implied by the source corpus. Grouping should remain configurable and should not be hard-coded into reusable indexing logic.

### 16.6 OCR Fallback Output

The editable `pages/*.txt` files should receive the best machine-generated starting text before review begins.

Raw extraction outputs may be preserved separately for diagnostics, such as:

```text
raw/
  embedded/
    0001.txt
  ocr/
    0001.txt
pages/
  0001.txt
```

Rationale: the editor should only need to edit one page text file, but preserving raw embedded-text and OCR outputs can help diagnose extraction problems. Raw extraction outputs should be treated as local build artifacts and excluded from git/distribution unless explicitly approved.

### 16.7 Package Naming and Versioning

Expected package naming format:

```text
[Traveller edition]-n.n.n
[Traveller edition]-n.n.n-rc-n
```

Example:

```text
classic-traveller-1.0.0
classic-traveller-1.0.0-rc-1
```

The exact edition label remains to be confirmed later.

---

## 17. Remaining Open Questions

- How should query-time snippets access reviewed page text without making the browser package or initial load too large?
- Should reviewed page text be loaded as one resource, per-volume resources, or compact page text chunks?
- What exact edition label should be used for the first package name?
- What quality report format best supports the solo page review workflow?
- What grouping metadata, if any, materially improves Classic Traveller catalog navigation?
- Can printed page numbers be captured reliably enough to display alongside PDF page indices?
- Should `skrat` become part of the review workflow, remain an external helper, or be ignored for this project?
- What file contracts must this project define so an external extractor/reviewer tool can interoperate cleanly?
- What responsibilities belong to the browser/indexer project versus the separate extractor/reviewer project?
- What metrics should be gathered before setting performance budgets?
- What exact checks are needed to verify WCAG 2.2 AA conformance for the browser shell?

---

## 18. Ideation Exit Notes

This document does not select a final architecture or define formal requirements.

Before advancing to Requirements, the project should confirm:

- project scope and stakeholder intent
- MVP success criteria
- source corpus boundaries
- review workflow expectations
- quality reporting expectations
- whether GJTAS browser reuse is a constraint or an implementation preference

Explicit human approval is required before entering the Requirements phase.