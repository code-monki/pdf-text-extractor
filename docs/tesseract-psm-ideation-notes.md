# Tesseract Page Segmentation Mode Ideation Notes

Status: Ideation Note
Date: 2026-04-30
Lifecycle Phase: Ideation
Source: local `tesseract --help-psm`

---

## 1. Purpose

This note captures Tesseract page segmentation mode options and early applicability thinking for the PDF text extractor.

It does not select a final OCR strategy. It provides shared vocabulary for Requirements and later Design discussion.

---

## 2. What Page Segmentation Mode Controls

Tesseract page segmentation mode controls the layout assumption Tesseract uses before recognizing text.

This matters because the expected PDF corpus may contain:

- ordinary prose pages
- tables embedded in or near prose
- forms
- maps or labels
- covers and title pages
- sparse text pages
- multi-column or layout-heavy pages
- scanned pages with uncertain orientation

The wrong segmentation mode can produce bad reading order, dropped text, or noisy output even when OCR technically succeeds.

---

## 3. Available Modes

| Mode | Name | Local Tesseract description | Likely applicability |
| --- | --- | --- | --- |
| `0` | `osd_only` | Orientation and script detection only. | Useful as a preprocessing probe, not for extracting text. |
| `1` | `auto_osd` | Automatic page segmentation with orientation and script detection. | Useful when page orientation may vary or scans may be rotated. Slower and potentially noisier than mode `3`. |
| `2` | `auto_only` | Automatic page segmentation, no OSD or OCR; not implemented. | Not useful for this project. |
| `3` | `auto` | Fully automatic page segmentation, no OSD. Default. | Strong default candidate for ordinary pages when orientation is already correct. |
| `4` | `single_column` | Assume a single column of text of variable sizes. | Candidate for standard book pages that are mostly one-column prose. |
| `5` | `single_block_vert_text` | Assume one uniform block of vertically aligned text. | Probably not useful for these English-language RPG PDFs unless rotated/vertical text appears. |
| `6` | `single_block` | Assume a single uniform block of text. | Candidate for cropped text regions or simple prose pages. May be too rigid for tables/forms. |
| `7` | `single_line` | Treat image as a single text line. | Useful only for extracted/cropped line regions, not full pages. |
| `8` | `single_word` | Treat image as a single word. | Useful only for cropped labels/words, not full pages. |
| `9` | `circle_word` | Treat image as one word in a circle. | Not expected to be useful. |
| `10` | `single_char` | Treat image as a single character. | Not useful for page OCR. |
| `11` | `sparse_text` | Find as much text as possible in no particular order. | Candidate for maps, counters, forms, labels, or layout-heavy pages where reading order matters less than discovery terms. |
| `12` | `sparse_text_osd` | Sparse text with orientation and script detection. | Candidate for sparse/label-heavy pages with uncertain orientation. |
| `13` | `raw_line` | Treat image as single text line, bypassing Tesseract-specific hacks. | Useful only for cropped line regions, not full pages. |

---

## 4. Candidate Strategy Options

### Option A: Fixed Default Mode

Use one mode for all full-page OCR, probably `3` or `6`.

Advantages:

- simplest to implement and test
- easiest to reproduce
- fewest user-facing decisions

Risks:

- may perform poorly on maps, tables, forms, title pages, or sparse pages
- may hide that a different mode would have produced better OCR

### Option B: User-Configurable Mode

Allow the user to select the mode for a page or volume.

Advantages:

- gives control for difficult pages
- useful when an operator recognizes page type better than heuristics

Risks:

- adds friction
- increases documentation burden
- increases test matrix
- may expose too much OCR machinery to the user

### Option C: Adaptive Mode

Select OCR mode based on page classification or extraction diagnostics.

Possible examples:

- ordinary prose page: `3` or `4`
- simple block page: `6`
- sparse map/label/form page: `11`
- uncertain orientation: `1` or `12`

Advantages:

- reduces user burden
- may improve OCR across mixed documents

Risks:

- page classification can be wrong
- adaptive behavior may be harder to reproduce and explain
- failures may be less transparent unless diagnostics record selected mode and reason

### Option D: Multi-Candidate OCR

Run more than one mode behind the scenes and compare candidates before selecting or presenting a result.

Advantages:

- may improve OCR quality without forcing the user to choose a mode
- gives reviewer alternatives
- useful for layout-hostile pages
- can be applied only to suspicious pages to control cost

Risks:

- slower
- increases local artifact size
- complicates selection/scoring
- may create too much review noise
- poor automatic scoring could select a worse candidate with false confidence

Possible comparison signals:

- character count
- word count
- symbol ratio
- dictionary-like token ratio
- OCR confidence, if available and useful
- line count and very long/short line patterns
- disagreement between candidates
- manual reviewer preference over time

Candidate retention options:

- retain all OCR mode candidates in `raw/ocr/<mode>/NNNN.txt`
- retain only the selected OCR candidate plus summary metrics for rejected candidates
- retain multiple candidates only for pages marked suspicious

Open issue:

- Multi-candidate OCR should not silently hide uncertainty. Diagnostics should record which modes ran, which candidate was selected, and why.

---

## 5. Current Ideation Position

The extractor should not expose all Tesseract modes as a primary workflow control.

Likely direction to evaluate:

- use a simple default mode or small candidate set for full-page OCR
- record the mode used in diagnostics
- consider hidden multi-mode OCR for suspicious or layout-hostile pages
- allow an advanced/manual re-OCR option later if a page is poor
- consider sparse-text mode for specific page classes only if experiments prove value

Initial follow-up evidence:

- A 12-page sample using modes `3`, `4`, `6`, and `11` completed 48 OCR candidates without errors after image-path handling was corrected.
- With bounded parallelism, the same 48-candidate sample completed in 9,312 ms at 8 jobs and 6,820 ms at 14 jobs.
- A full 51-page run across the same four modes completed 204 OCR candidates in 26,279 ms at 14 jobs with no errors.
- A full 51-page run across modes `1`, `3`, `4`, `6`, `11`, and `12` completed 306 OCR candidates in 38,991 ms at 14 jobs with no errors.
- On the tested PDF, modes `1` and `3` produced identical per-page metrics and confidence values, suggesting OSD added no value for normally oriented pages.
- Best-confidence mode and best-simple-score mode disagreed on 32 of 51 pages, so confidence should be treated as one signal rather than an automatic selector.
- Mode outputs differed enough to make hidden comparison worth further exploration.
- A crude metric score selected different modes for different pages, including mode `11` on several pages.
- The crude score is not sufficient final selection logic.

Second-volume transfer evidence:

- A second Paper Capture volume with 52 pages completed 312 OCR candidates across modes `1`, `3`, `4`, `6`, `11`, and `12` in 37,901 ms at 14 jobs with no errors.
- The crude simple score selected multiple modes: PSM `1` on 19 pages, PSM `4` on 1 page, PSM `6` on 12 pages, PSM `11` on 12 pages, and PSM `12` on 8 pages.
- Highest mean confidence selected a different distribution and disagreed with the crude simple score on 36 of 52 pages.
- Modes `1` and `3` again produced identical per-page metrics and confidence values on the normally oriented PDF.
- This strengthens the case for hidden multi-candidate comparison while weakening the case for running both PSM `1` and PSM `3` on normally oriented pages.

This should be validated with additional OCR experiments before Requirements stabilization if OCR quality remains a primary project risk.

---

## 6. Requirements Questions

- Which default mode should be used for full-page OCR trials: `3`, `4`, or `6`?
- Should Paper Capture PDFs use a different default from image-only scans?
- Should sparse pages be re-run with `11` or `12`?
- Should the UI expose OCR mode directly, hide it under an advanced option, or avoid exposing it entirely?
- Should `page-quality.json` record Tesseract mode and OCR command metadata?
- Should multiple OCR candidates be retained for pages where modes disagree significantly?
- Should the engine run multiple PSM modes behind the scenes and compare outcomes?
- Which comparison signals are reliable enough to select an OCR candidate automatically?
