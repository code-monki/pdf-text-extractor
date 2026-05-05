# Ideation Stabilization Summary

Status: Approved for Requirements Phase
Date: 2026-05-01
Lifecycle Phase: Ideation
Governance Reference:

- `ai-toolkit/02-governance/01-ideation-guardrail.md`
- `ai-toolkit/02-governance/12-phase-gate-checklist.md`

---

## 1. Purpose

This document summarizes the approved Ideation position for `pdf-text-extractor`.

Advancement to Requirements was explicitly approved by the project owner on 2026-05-01.

---

## 2. Gate Criteria Assessment

Ideation exit criteria from the phase gate:

- Problem statement formally defined: satisfied
- Stakeholders identified: satisfied
- Constraints documented: satisfied
- Risks identified: satisfied
- Success criteria defined: satisfied at conceptual level
- Alternative solution paths explored: satisfied
- No unresolved ambiguity in problem framing: satisfied

The remaining open questions are Requirements inputs, not blockers to understanding the problem space.

---

## 3. Stabilized Problem Frame

The project addresses unreliable local PDF text extraction for downstream search, review, and packaging workflows.

The core problem is not merely extracting text. The project must support inspectable page-level extraction, OCR fallback or OCR routing, quality diagnostics, human normalization, stable page identity, and local-only handling of copyrighted source and derived full-text artifacts.

The immediate validation corpus is Classic Traveller PDF material for `/Users/chuck/ct-cd`, but the extractor should remain reusable for other local PDF corpora.

---

## 4. Stabilized Scope Position

Likely in scope for Requirements:

- one selected PDF at a time
- source PDF identity, checksums, file size, modified time, readability, encryption status, and page count
- embedded text extraction attempts
- OCR candidate generation for image-only or weak embedded text pages
- Paper Capture PDFs running OCR on all pages by default as a candidate workflow
- raw embedded and OCR candidate retention as local-only artifacts
- reviewed page text in `pages/NNNN.txt`
- `volume.json`, `page-quality.json`, and `review-state.json`
- work-folder validation
- release/indexing rule that indexes only `accepted` pages by default
- native local application expectation
- GUI-centered page review workflow
- synchronized PDF page and text page navigation
- metadata review/editing workflow

Likely adjacent or conditional:

- reusable volume package format for broader library-system use
- corpus-level metadata and grouping
- package/export profiles
- alternate OCR engines if Tesseract evidence becomes insufficient
- richer page metadata if current `volume.json.pageMap` becomes inadequate

Likely out of scope unless Requirements explicitly expand it:

- static browser UI
- browser-based extractor application
- search index generation
- hosted service behavior
- database-backed document management
- source PDF editing
- rights-management enforcement
- full library management platform ownership

---

## 5. Stabilized Output Contract Concept

The current work-folder concept is:

```text
work/
  <volume-id>/
    volume.json
    page-quality.json
    review-state.json
    pages/
      0001.txt
    raw/
      embedded/
      ocr/
```

Current positions:

- `volume.json` is the primary volume metadata file.
- `volume.json` should include bibliographic metadata when known.
- Candidate bibliographic fields include title, subtitle, sort title, publisher, publication year, original publication year, edition, language, ISBN, product code, creators, series, series number, description, subjects, and rights.
- Optional cover metadata should allow a user-specified cover page and/or generated cover asset when useful.
- `volume.json.pageMap` maps page file ID to PDF page number and printed document page label.
- `pageMap` stays in `volume.json` unless future evidence shows larger volumes or richer metadata make that inadequate.
- `pages/NNNN.txt` contains reviewed UTF-8 search-surrogate page text with no front matter.
- `page-quality.json` records extraction diagnostics.
- `review-state.json` records human review workflow state.
- Raw candidates stay local-only unless explicitly approved.

---

## 6. Stabilized Experiment Findings

Representative PDF experiments showed:

- Paper Capture PDFs may contain usable embedded text.
- OCR remains useful as an alternate candidate, not as trusted final text.
- Manual normalization is likely required page by page.
- Printed page labels need manual correction or entry.
- Greater-than-5% embedded/OCR character delta is a useful candidate suspicious-text flag.
- Hidden multi-PSM Tesseract comparison is feasible on the available hardware.
- OCR confidence alone is not reliable enough for automatic candidate selection.
- PSM `1` and PSM `3` produced identical metrics on normally oriented tested PDFs.
- OCRmyPDF did not outperform direct Tesseract on the ground-truth sample.
- A 161-page pressure test supports keeping `pageMap` in `volume.json`.

No extracted source text was reproduced in documentation.

---

## 7. Stabilized UI Workflow Concepts

The reviewer UI is in scope for the project concept, and the current product expectation is a native local application rather than a browser-based application. The rationale is to reduce complexity and friction for the proposed local-file-centered workflow. No native UI architecture or framework is selected.

Current workflow hypotheses:

- User works one selected PDF at a time.
- PDF page navigation drives the displayed text page.
- If no document or page is selected, the text editor is empty.
- Auto-save on page navigation is a strong candidate, provided visible save status, restore-from-raw, and backup protections exist.
- Metadata pane should expose an edit action rather than being directly editable.
- Metadata edit action should open a dialog with save and cancel actions.
- Metadata dialog may expose a base field set by default and allow optional bibliographic fields to be added or revealed.
- Metadata dialog may allow the user to specify a cover page for future bookshelf or visual library views.

---

## 8. Requirements Inputs

Items to carry into Requirements:

- formalize work-folder schema
- define required versus optional volume metadata
- define user-editable versus read-only metadata
- define corpus-relative path behavior
- define review statuses and transitions
- define page-label entry behavior
- define extraction/OCR candidate retention policy
- define work-folder validation behavior
- define overwrite, backup, and restore protections
- define quality flags and thresholds
- decide whether multi-PSM OCR comparison is required, optional, or experimental
- decide whether OCRmyPDF remains reference-only
- define GUI workflow requirements without selecting architecture
- define native application expectations without selecting framework
- define optional cover page metadata behavior
- decide packaging/export ownership boundaries

---

## 9. Deferred Questions

Questions that do not block Ideation completion:

- exact native UI framework or implementation model
- exact component boundaries for HLA
- final OCR candidate scoring algorithm
- whether alternate OCR engines are needed
- exact package format for broader library-system distribution
- whether corpus-level metadata requires a separate manifest
- concrete split trigger for moving `pageMap` out of `volume.json`
- final field list for base versus optional metadata edit dialog

---

## 10. Approval

Ideation is approved and complete.

Approval record:

- Approved to proceed to Requirements: Yes
- Approval name: Chuck
- Approval role: Project owner
- Approval date: 2026-05-01
- Approval basis: concept document reviewed with no obvious holes; project owner explicitly approved moving to the next phase

Next step:

- Requirements work should use the stabilized Ideation documents as inputs, not as binding architecture.
