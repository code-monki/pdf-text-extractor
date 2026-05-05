# PDF Text Extractor Output Contract Concept

Status: Ideation Draft
Date: 2026-04-30
Lifecycle Phase: Ideation
Source Context: Extracted from prior static PDF browser/indexer discussion
Governance Reference: `ai-toolkit/02-governance/01-ideation-guardrail.md`

---

## 1. Purpose

This document captures the provisional output contract concept for the separated PDF text extractor project.

It is not an approved Requirements artifact, architecture decision, or implementation design. It preserves context for later Requirements work and should be validated before stabilization.

The core idea is that the extractor produces and maintains a local, inspectable work-folder contract. Downstream systems such as the browser/indexer consume that contract without depending on extractor internals, OCR tool details, or corpus-specific assumptions.

---

## 2. Work Folder Model

Use one work folder per book or volume:

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
    raw/
      embedded/
        0001.txt
      ocr/
        0001.txt
```

Conceptual responsibilities:

- `volume.json`: stable book-level metadata and indexing intent.
- `pages/NNNN.txt`: editable reviewed page text, one UTF-8 text file per PDF page.
- `page-quality.json`: machine-maintained extraction diagnostics by page.
- `review-state.json`: human/editor workflow status by page.
- `raw/embedded/NNNN.txt`: optional raw embedded text candidate.
- `raw/ocr/NNNN.txt`: optional raw OCR candidate.
- `raw/`: diagnostic-only extraction material that stays local unless explicitly approved.

---

## 3. `volume.json` Concept

`volume.json` is book-level only. It should not contain per-page extracted text.

Example:

```json
{
  "schemaVersion": 1,
  "volumeId": "ct-b01-characters-combat-1981",
  "title": "Book 1: Characters & Combat",
  "subtitle": "1981 edition",
  "sortTitle": "Book 01 Characters Combat",
  "group": "Books",
  "publisher": "Game Designers' Workshop",
  "publicationYear": 1981,
  "originalPublicationYear": null,
  "edition": "1981 edition",
  "language": "en",
  "isbn": null,
  "productCode": null,
  "creators": [],
  "series": null,
  "seriesNumber": null,
  "description": "",
  "subjects": [],
  "rights": {
    "copyrightNotice": "",
    "usage": "copyrighted-source-local-only"
  },
  "cover": {
    "pdfPage": null,
    "pageFile": null,
    "assetPath": null
  },
  "sourcePdf": {
    "path": "pdfs/CT B01 Book 01 Characters & Combat 1981.pdf",
    "filename": "CT B01 Book 01 Characters & Combat 1981.pdf",
    "pageCount": 48
  },
  "pageMap": {
    "0001": {
      "pdfPage": 1,
      "pageFile": "pages/0001.txt",
      "printedPageLabel": null,
      "printedPageSort": null
    }
  },
  "indexing": {
    "include": true,
    "defaultPageStatusRequired": "accepted",
    "skipReason": null
  },
  "collection": {
    "collectionId": "classic-traveller",
    "edition": "Classic Traveller",
    "order": 10
  },
  "notes": {
    "copyrightStatus": "copyrighted-source-local-only",
    "reviewNotes": ""
  }
}
```

Current metadata ownership and path positions:

- All `volume.json` fields are initially extractor-owned because the extractor creates or initializes the file.
- User modification is expected and allowed for metadata fields after initialization.
- Publisher, publication year, original publication year, edition, language, optional ISBN, product code, creators, series, series number, description, subjects, and rights are useful volume-level bibliographic metadata.
- Bibliographic fields should be initialized when known and left nullable, empty, or omitted when unavailable.
- A future metadata edit dialog may present a base field set by default while allowing users to add or reveal optional bibliographic fields as needed.
- Optional cover metadata should allow the user to specify the source cover page when one exists, removing ambiguity for future bookshelf or visual library views.
- Collection grouping, when available, is corpus-level metadata rather than volume-level metadata.
- `sourcePdf.path` should be recorded relative to the corpus root.
- Downstream indexers should adapt corpus-relative paths as needed for their own packaging or runtime layout.

Open validation question:

- Whether future larger volumes or expanded page metadata justify splitting `pageMap` into a separate page metadata file. Current evidence supports keeping it in `volume.json`; this remains open only as a future adequacy check.

Current page-label concept:

- PDF page index remains the stable internal identity.
- Page text filename remains the zero-padded PDF page index, for example `pages/0007.txt`.
- Printed document page labels are indexing/reference metadata only and may differ from PDF page index.
- Printed document page labels are expected to be best-effort from extraction/OCR and manually correctable.
- `printedPageLabel` stores the user-facing label as printed, such as roman numerals, blank front matter, or numbered pages.
- `printedPageSort` is an optional normalized sortable value for downstream navigation/index ordering when the printed label is not directly numeric.
- The label map should associate `pageFile`, `pdfPage`, and `printedPageLabel` so downstream tools do not infer one from another.
- Current working position: keep `pageMap` in `volume.json` because the metadata is volume-scoped and expected to remain small enough for normal volumes.
- Split trigger to monitor: larger volumes or future page metadata growth may justify moving the map to a separate file, but that should be evidence-driven rather than assumed now.

Pressure-test evidence:

- A 161-page representative PDF produced a pretty-printed `volume.json` page-map sample of approximately 24 KB.
- The compact JSON equivalent was approximately 20 KB.
- The sample validated with 161 page-map entries and no structural errors.
- This supports keeping `pageMap` in `volume.json` for current corpus-scale volumes.

---

## 4. `pages/NNNN.txt` Concept

Each page file is plain UTF-8 text with no front matter:

```text
Page heading or useful page title if present

Reviewed searchable page text goes here.

Tables may be flattened into readable search-oriented text.
Map labels, names, rules terms, and important captions may be included when useful for discovery.
```

Conceptual rules:

- Filename is zero-padded PDF page index: `0001.txt`, `0002.txt`, etc.
- One file exists per PDF page, even if empty.
- Text is a search surrogate, not layout reconstruction.
- Metadata stays in JSON files, not page text headers.
- Text is normalized as UTF-8.
- Readable search text is preferred over visual fidelity.
- Empty or unusable pages are allowed but represented in review state.
- Copyrighted page content is not reproduced in documentation, commits, or agent responses unless explicitly approved.

---

## 5. `page-quality.json` Concept

Example:

```json
{
  "schemaVersion": 1,
  "volumeId": "ct-b01-characters-combat-1981",
  "pages": {
    "0001": {
      "pdfPage": 1,
      "selectedSource": "embedded",
      "embedded": {
        "available": true,
        "charCount": 1240,
        "qualityScore": 0.82
      },
      "ocr": {
        "available": true,
        "charCount": 1198,
        "qualityScore": 0.76
      },
      "flags": [
        "short-page",
        "possible-reading-order-issue"
      ],
      "errors": []
    }
  }
}
```

Candidate `selectedSource` values:

- `embedded`
- `ocr`
- `manual`
- `empty`
- `quarantined`

Candidate quality flags:

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

Open validation questions:

- Whether quality scores should be comparable across embedded extraction and OCR.
- Whether diagnostics should include tool/version information for reproducibility.
- Whether errors should use structured codes rather than free-form messages.
- Whether embedded/OCR character-count deltas greater than 5% should become the standard suspicious-delta flag threshold.

---

## 6. `review-state.json` Concept

Example:

```json
{
  "schemaVersion": 1,
  "volumeId": "ct-b01-characters-combat-1981",
  "pages": {
    "0001": {
      "pdfPage": 1,
      "status": "raw",
      "reviewedAt": null,
      "reviewedBy": null,
      "notes": ""
    }
  }
}
```

Candidate statuses:

- `raw`: machine-generated and not checked.
- `editing`: currently being reviewed.
- `edited`: manually changed but not final.
- `accepted`: good enough for indexing.
- `skip`: intentionally excluded from indexing.
- `quarantined`: failed validation or needs special handling.

Conceptual release/indexing rule:

- Index only `accepted` pages by default.
- Exclude `skip` pages intentionally.
- Do not include `raw`, `editing`, `edited`, or `quarantined` pages in release builds.

Open validation questions:

- Whether the extractor owns review-state transitions or only initializes/validates the file.
- Whether `reviewedBy` should be optional, local-only, or configurable.
- Whether `quarantined` should be controlled only by validation tooling or also by manual review.
- Whether page-label editing should live in the same reviewer screen as page text editing or in a separate metadata panel.

---

## 7. Boundary Statement

The PDF text extractor should produce and maintain this work-folder contract.

The browser/indexer should consume:

- `volume.json`
- `pages/*.txt`
- `page-quality.json`
- `review-state.json`

The browser/indexer should not depend on:

- OCR engine selection
- embedded text extraction tooling
- raw extraction candidate layout beyond the approved contract
- extractor-internal scoring implementation
- corpus-specific assumptions

This boundary remains provisional until Requirements and Architecture phases stabilize it.

---

## 8. Packaging manifest (sidecar ingestion)

When the project defines **packaging** (installable artifacts per platform), the package should include a **machine-readable manifest** so external applications can ingest the extractor’s **sidecar data** without reverse-engineering the install layout. At minimum, the manifest should identify:

- the **work-folder contract** version or schema bundle reference (`volume.json`, `page-quality.json`, `review-state.json`, `pages/*.txt`, optional `raw/` layout);
- **paths** to those artifacts relative to the package root or to a declared content root;
- optional **checksums** or file sizes for integrity verification (not substantive text).

Exact manifest format (JSON-LD, SPDX-like JSON, or a small project-specific schema) is **deferred** until the Packaging phase; this note records the requirement so Gate 8 planning does not omit downstream consumers.

---

## 9. Risks To Carry Forward

- The contract may encode Classic Traveller-specific metadata too early.
- Review state may belong to a separate reviewer tool rather than the extractor core.
- Raw extraction candidates may expose copyrighted full text and require strict ignore/default-local handling.
- Per-page diagnostics may need stronger schema discipline before downstream automation can rely on them.
- Page IDs, PDF page indexes, and printed page labels must not be conflated.
- OCR and embedded extraction quality scores may be misleading if not calibrated against real corpus samples.
