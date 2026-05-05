# Ideation Risk Exploration Plan

Status: Ideation Draft
Date: 2026-04-30
Lifecycle Phase: Ideation

---

## 1. Purpose

This plan defines the first concrete risk-reduction experiment for the PDF text extractor.

It does not authorize implementation architecture. It identifies a focused experiment to reduce uncertainty about embedded text extraction, OCR quality, output artifacts, and diagnostics before Requirements stabilization.

---

## 2. Experiment 1: Single Difficult PDF Extraction Trial

Use one representative local PDF that is likely to be difficult for text extraction.

The selected PDF may be a "cursed" PDF: scanned, OCR-poor, missing text layer, badly encoded, layout-heavy, or otherwise likely to stress extraction quality.

The experiment does not require the GUI. Command-line scripts or direct tool execution are acceptable for this Ideation risk experiment because the goal is to validate extraction/OCR behavior and artifact shape, not user workflow.

---

## 3. Objectives

The experiment should answer:

- Can embedded text be extracted from the selected PDF?
- If embedded text exists, is it plausibly useful as searchable page text?
- If embedded text is absent or poor, can OCR produce useful page-level text?
- Can the candidate work-folder shape represent the result?
- What quality flags are immediately useful?
- What metadata is needed to explain the result without reproducing copyrighted text?
- What tool/version data is needed for reproducibility?

---

## 4. Candidate Tools

Candidate embedded text extraction tool:

- Poppler utilities, such as `pdftotext`, subject to local availability and later dependency evaluation.

Candidate OCR tool:

- Tesseract OCR, subject to local availability and later dependency evaluation.

Notes:

- These are candidate tools for exploration, not approved architecture or dependency decisions.
- If another open-source OCR engine appears materially better, it should be evaluated deliberately during Requirements, Architecture, or a later risk experiment before adoption.

---

## 5. Proposed Trial Flow

1. Select one local PDF outside git-tracked source.
2. Record minimal source metadata:
   - filename
   - path location category, not full copyrighted content
   - file size
   - checksum
   - page count
   - readable/unreadable status
3. Attempt embedded text extraction.
4. Record per-page embedded extraction metadata:
   - availability
   - character count
   - empty/very-short status
   - obvious symbol/control-character issues
   - extraction errors
5. Run OCR on the PDF or selected representative pages.
6. Record per-page OCR metadata:
   - OCR availability
   - character count
   - obvious quality flags
   - OCR errors
7. Create a local test work folder using the candidate contract:
   - `volume.json`
   - `page-quality.json`
   - `review-state.json`
   - `pages/NNNN.txt`
   - `raw/embedded/NNNN.txt` where available
   - `raw/ocr/NNNN.txt` where available
8. Inspect generated files locally without quoting substantive PDF text in docs or responses.
9. Summarize findings using counts, flags, filenames, hashes, page numbers, and minimal metadata only.

---

## 6. Safety Rules

- Do not commit the source PDF.
- Do not commit raw extraction candidates.
- Do not commit generated page text.
- Do not reproduce substantive source text in documentation or responses.
- Keep output under ignored local artifact paths such as `work/`.
- Use filenames, hashes, page counts, quality flags, and minimal metadata for reporting.
- Confirm `.gitignore` covers the generated paths before running the experiment.

---

## 7. Expected Outputs

Local-only experiment outputs:

- candidate work folder for the selected PDF
- raw embedded text candidates, if available
- raw OCR candidates, if available
- selected `pages/NNNN.txt` files for inspection
- page-quality metadata
- review-state metadata initialized to a non-final status

Commit-safe documentation outputs:

- experiment summary without source text
- tool availability and version notes
- page-count and extraction-count summary
- quality flag summary
- lessons for Requirements
- contract issues discovered

---

## 8. Printed Page Label Handling During Experiment

Printed document page labels should be treated as best-effort metadata.

During initial extraction/OCR:

- attempt to observe whether printed page labels can be detected or inferred
- record uncertainty rather than guessing silently
- allow for later user overwrite

Important distinction:

- PDF page index is the stable internal page identity.
- Printed document page label is indexing/reference metadata and may differ from the physical PDF page index.

Follow-up experiments settled the current concept position: printed page labels should live in `volume.json.pageMap` unless future metadata growth creates evidence for splitting page metadata into a separate file.

---

## 9. GUI Scope Implication

The reviewer GUI is in scope for this project, but it is not the first risk-reduction target.

The first Spiral quadrant should focus on extraction infrastructure and OCR behavior because the GUI depends on reliable extraction artifacts.

Later GUI work should consume the extraction outputs rather than hide extraction behavior in UI-only state.

---

## 10. Open Questions After Experiment

The experiment should help answer:

- Is Poppler sufficient for embedded extraction attempts?
- Is Tesseract acceptable for likely OCR-heavy PDFs?
- What quality flags are most useful immediately?
- Is the candidate work-folder contract missing required fields?
- What additional page-label fields, if any, are needed beyond the current `volume.json.pageMap` concept?
- What should the app do when embedded text exists but is poor?
- How much tool/version metadata is required for reproducibility?
- Does raw candidate retention create any workflow or storage problem?
