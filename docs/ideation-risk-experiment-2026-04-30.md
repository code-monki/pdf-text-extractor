# Ideation Risk Experiment Summary

Status: Ideation Experiment Summary
Date: 2026-04-30
Lifecycle Phase: Ideation
Plan Reference: `docs/ideation-risk-exploration-plan.md`

---

## 1. Experiment

Single difficult PDF extraction/OCR trial.

Source PDF:

- filename: `CT B01 Book 01 Characters & Combat 1981.pdf`
- local source category: ignored `pdfs/` folder
- SHA-256: `613cf5262944027dc2426cf50aa66292645ec205c4f98ac14daa11551faae950`
- size: 8,299,116 bytes
- page count: 51
- encrypted: no
- tagged: no
- PDF version: 1.4
- creator/producer signal: Adobe Acrobat Paper Capture workflow
- stakeholder representativeness note: the PDF's table-heavy and occasional two-column layout is typical of the expected near-term corpus

No extracted source text is reproduced in this summary.

---

## 2. Tools Observed

Poppler:

- `pdfinfo` / `pdftotext` version: 26.02.0

Tesseract:

- version: 5.5.2

Candidate tool status:

- Poppler was available locally and successfully read the PDF.
- `pdftotext` completed page-level embedded extraction without stderr output.
- `pdftoppm` rendered page images without stderr output.
- Tesseract completed OCR without stderr output.

These tools remain exploration candidates, not approved architecture dependencies.

---

## 3. Local Outputs

Local-only ignored work folder:

```text
work/ct-b01-characters-combat-1981/
```

Generated local artifacts:

- `volume.json`
- `page-quality.json`
- `review-state.json`
- `pages/0001.txt` through `pages/0051.txt`
- `raw/embedded/0001.txt` through `raw/embedded/0051.txt`
- `raw/ocr/0001.txt` through `raw/ocr/0051.txt`
- rendered page images under `raw/page-images/`
- diagnostics under `diagnostics/`

All generated work-folder artifacts are ignored by git through the root `.gitignore`.

---

## 4. Embedded Extraction Summary

Embedded extraction output:

- pages processed: 51
- total characters: 154,728
- minimum page characters: 1
- maximum page characters: 5,083
- empty pages: 0
- very short pages: 2
- high-symbol pages: 1
- high-numeric pages: 0
- unicode/control-suspect pages after ignoring expected page breaks: 0

Observation:

- The PDF contains extractable embedded text for every page.
- Because the PDF appears to come from a Paper Capture workflow, embedded text may still be OCR-derived and should not be trusted without review.

---

## 5. OCR Summary

OCR output:

- pages processed: 51
- total characters: 139,451
- minimum page characters: 50
- maximum page characters: 4,582
- empty pages: 0
- very short pages: 2
- high-symbol pages: 1
- high-numeric pages: 0
- unicode/control-suspect pages: 0

Embedded versus OCR comparison:

- pages where OCR produced more characters: 5
- pages where embedded extraction produced more characters: 46
- pages with character-count delta greater than 500: 12
- stakeholder-proposed suspicious delta threshold after the experiment: greater than 5%
- pages with any simple flag from the trial: 4 before delta-based suspicion
- pages marked `embedded-text-suspicious` in generated `page-quality.json`: 13

Observation:

- OCR completed successfully and produced comparable page-level candidates.
- Large character-count deltas indicate that embedded and OCR candidates may differ materially on some pages.
- Character counts alone cannot establish which candidate is better; human review remains necessary.

---

## 6. Candidate Work-Folder Validation

Validation checks:

- `volumeId` matched across `volume.json`, `page-quality.json`, and `review-state.json`: yes
- page count: 51
- missing `pages/NNNN.txt` files: 0
- missing quality entries: 0
- missing review entries: 0
- invalid review statuses: 0
- quality entries: 51
- review entries: 51

Generated quality flag counts:

- `embedded-text-suspicious`: 13
- `very-short`: 2
- `high-symbol-ratio`: 1

---

## 7. Printed Page Label Handling

No reliable printed document page labels were captured during this trial.

Experiment implication:

- printed page label capture should be treated as best-effort
- user overwrite/manual entry remains necessary
- PDF page index should remain the stable internal page identity

---

## 8. Lessons For Requirements

Strong candidate Requirements inputs:

- The extractor should preserve embedded and OCR raw candidates separately.
- The extractor should track per-page embedded and OCR character counts.
- The extractor should flag embedded/OCR character-count deltas greater than 5% for review.
- The extractor should initialize review state for every PDF page.
- The extractor should validate that every PDF page has page text, quality, and review-state entries.
- The extractor should retain source file hash, file size, page count, and readable/encrypted status.
- Printed page label handling should be best-effort and user-overridable.
- For Paper Capture PDFs, OCR should run on all pages by default.
- Human review is still required even when embedded text exists.

Open questions:

- Should Tesseract page segmentation mode be configurable, adaptive, or fixed by workflow profile?
- What additional fields, if any, should the `volume.json.pageMap` include for printed document labels?
- What level of OCR confidence data is needed beyond character counts and flags?

Discussion notes:

- Configurable Tesseract page segmentation mode could help with unusual layouts but may add user-facing friction and increase the testing matrix.
- Adaptive Tesseract page segmentation mode could reduce operator burden but requires reliable page classification or fallback behavior.
- Fixed Tesseract page segmentation mode is simpler but may perform poorly across mixed layouts.
- OCR confidence data may be useful, but its practical value is unclear until compared with review outcomes.
- Tesseract page segmentation modes and likely applicability are expanded in `docs/tesseract-psm-ideation-notes.md`.
- Follow-up pressure testing supports keeping printed page label metadata in `volume.json.pageMap` for current corpus-scale volumes.

---

## 9. Hidden Multi-Mode OCR Follow-Up

Follow-up script:

- `scripts/experiments/compare-tesseract-psm.js`
- updated to support bounded parallelism through `--jobs`

Sample:

- pages tested: 12
- modes tested: `3`, `4`, `6`, `11`
- OCR candidates generated: 48
- errors: 0 after fixing rendered-image filename padding
- total elapsed time across candidates: 68,622 ms
- minimum single candidate time: 610 ms
- maximum single candidate time: 1,986 ms
- local candidate storage for sample: 256 KB
- rerun with `--jobs 8`: 9,312 ms wall-clock
- rerun with `--jobs 14`: 6,820 ms wall-clock

Mode selection by crude exploratory score:

- mode `3`: 3 pages
- mode `6`: 3 pages
- mode `11`: 6 pages

Observed metric spread:

- pages with character-count spread greater than 250 across modes: 1
- pages with word-count spread greater than 25 across modes: 1
- pages with simple-score spread greater than 250 across modes: 3

Experiment implication:

- Hidden multi-mode OCR is technically viable on a sampled set.
- The modes do produce measurable differences.
- Sparse text mode `11` looked promising by crude metrics on several sampled pages.
- The current simple score is not enough to select final OCR candidates safely.
- Candidate selection logic needs review-grounded evidence, not just character counts or simple ratios.

Requirements/design carry-forward:

- The engine may run multiple PSM modes behind the scenes without exposing mode choice to the normal user workflow.
- Diagnostics should record which modes ran, candidate metrics, selected candidate, and selection rationale.
- Multi-mode OCR may be best limited to suspicious/layout-hostile pages unless broader experiments justify running it everywhere.

Full-document parallel run:

- pages tested: 51
- modes tested: `3`, `4`, `6`, `11`
- OCR candidates generated: 204
- jobs: 14
- wall-clock elapsed time: 26,279 ms
- total candidate elapsed time: 352,575 ms
- errors: 0
- local candidate storage: 888 KB

Full-document crude score selection:

- mode `3`: 18 pages
- mode `6`: 9 pages
- mode `11`: 24 pages

Full-document spread metrics:

- pages with character-count spread greater than 250 across modes: 1
- pages with word-count spread greater than 25 across modes: 2
- pages with simple-score spread greater than 250 across modes: 4

Hardware implication:

- On the Mac Mini M4 Pro environment, full-document hidden multi-mode OCR across four modes was fast enough to remain viable as an extraction strategy candidate.
- Parallelism changes the feasibility picture materially; serial OCR timings should not be used for capacity planning on this hardware.

---

## 10. Expanded Multi-Mode OCR Follow-Up

Expanded run:

- pages tested: 51
- modes tested: `1`, `3`, `4`, `6`, `11`, `12`
- OCR candidates generated: 306
- jobs: 14
- wall-clock elapsed time: 38,991 ms
- total candidate elapsed time: 532,777 ms
- errors: 0
- local candidate storage after TSV generation: 9.0 MB

Expanded crude score selection:

- mode `1`: 18 pages
- mode `6`: 9 pages
- mode `11`: 21 pages
- mode `12`: 3 pages

Mode observations:

- modes `1` and `3` produced identical per-page metrics and confidence values on this PDF
- mode `1` added elapsed time without observable metric benefit for this normally oriented PDF
- mode `11` remained a frequent winner by crude score
- mode `12` had the highest aggregate mean confidence, but was rarely selected by crude score

Confidence observations:

- confidence-bearing candidate rows: 304 of 306
- mean of candidate mean confidence values: 92.3009
- low-confidence word count across candidates: 4,937
- best-confidence mode and best-simple-score mode disagreed on 32 of 51 pages

Experiment implication:

- OCR confidence is useful diagnostic evidence but should not be used as the sole automatic selection criterion.
- OSD-enabled modes may be useful for rotated/uncertain pages, but did not add value on this normally oriented PDF.
- A practical strategy may be to use non-OSD modes by default, reserve OSD modes for orientation uncertainty, and evaluate sparse-text modes for layout-hostile pages.

---

## 11. Ground-Truth Comparison

Ground-truth pages supplied locally:

- `0005`: table of contents
- `0015`: two-column lists/tables
- `0045`: equipment/body armor tables
- `0047`: weapons matrix plus short prose
- `0049`: combat procedure/range/terrain tables

Comparison script:

- `scripts/experiments/compare-ocr-to-ground-truth.js`

Safety:

- ground-truth files remain under ignored `work/`
- comparison reports contain aggregate metrics only
- no ground-truth or OCR source text is reproduced

Initial comparison modes:

- embedded
- direct Tesseract OCR
- Tesseract PSM `1`, `3`, `4`, `6`, `11`, `12`

Aggregate findings across five ground-truth pages:

- embedded text had the best mean CER and WER overall
- direct Tesseract / PSM `6` was best on some table-heavy pages
- PSM `11` reduced missing-word count but introduced enough ordering/extra-token noise that WER/CER stayed high
- best CER and best WER agreed on page winners except for page `0015`

Best by CER:

- `0005`: embedded
- `0015`: PSM `1` / equivalent PSM `3`
- `0045`: direct OCR / PSM `6`
- `0047`: embedded
- `0049`: embedded

Best by WER:

- `0005`: embedded
- `0015`: direct OCR / PSM `6`
- `0045`: direct OCR / PSM `6`
- `0047`: embedded
- `0049`: embedded

Interpretation:

- Embedded text cannot be dismissed just because the PDF is cursed or Paper Capture-derived.
- OCR candidates are still valuable as alternates and for comparison.
- Sparse-text modes may preserve discoverable tokens but can harm sequence-sensitive metrics.
- CER/WER are useful, but snippet usefulness and search-term preservation still need human judgment.
- Because the tested layout profile is representative of expected volumes, these findings are likely relevant to near-term corpus work rather than isolated to one unusual PDF.

---

## 12. OCRmyPDF Comparison

OCRmyPDF version:

- 17.3.0

Variants tested:

- `--force-ocr`, PSM `3`
- `--force-ocr --deskew --clean`, PSM `3`

Observed behavior:

- OCRmyPDF completed successfully.
- OCRmyPDF warned that pages already have text and force OCR rasterized text anyway.
- OCRmyPDF/Tesseract reported possible poor OCR on pages `31` and `44`.
- Force OCR output PDF was approximately 3.56 times larger than the input.
- Force OCR with deskew/clean output PDF was approximately 3.63 times larger than the input.
- Local OCRmyPDF artifacts consumed approximately 58 MB after both variants.

Ground-truth aggregate result:

- OCRmyPDF force OCR was worse than embedded text, direct Tesseract OCR, and the best PSM candidates on the five-page sample.
- OCRmyPDF force OCR with deskew/clean did not materially improve the sample.

Experiment implication:

- OCRmyPDF is useful as a pipeline/reference tool, but it does not currently look better than direct page-image Tesseract for this use case.
- OCRmyPDF may still be valuable for different PDFs, preprocessing experiments, or producing searchable PDFs, but the extractor's page-text candidate workflow should not assume OCRmyPDF is superior.

---

## 13. Transfer Validation Plan

A second PDF should be tested to validate whether the first experiment's findings transfer to another volume with similar formatting.

Focus:

- confirm embedded text remains competitive against OCR candidates
- confirm table-heavy and two-column pages show similar candidate-selection behavior
- confirm greater-than-5% embedded/OCR delta remains a useful suspicion flag
- confirm OCRmyPDF remains non-superior before deprioritizing it further
- identify any new page types not represented by the first PDF

The second test should reuse the same safety rules: no committed source PDFs, no committed derived text, and no substantive extracted text in documentation or responses.

---

## 14. Transfer Validation Result: Book 3

Second test source:

- file: `pdfs/CT B03 Book 03 Worlds and Adventures 1981.pdf`
- SHA-256: `734cca312a53c24c9f0aaf8f0add3cdef845b0ee3ad7ce17bff1d49b03e7dd7e`
- size: 8,265,941 bytes
- pages: 52
- PDF profile: Adobe Acrobat 7.08 / Paper Capture Plug-in, PDF 1.4
- encrypted: no
- generated work folder: `work/ct-b03-worlds-and-adventures-1981/`

Embedded extraction aggregate:

- pages: 52
- total characters: 150,264
- minimum page characters: 1
- maximum page characters: 5,674
- empty pages: 0
- very short pages: 3
- high-symbol pages: 1
- unexpected control-character pages: 0

Direct Tesseract OCR baseline:

- mode: PSM `6`
- pages: 52
- total characters: 132,876
- minimum page characters: 50
- maximum page characters: 4,918
- empty pages: 0
- very short pages: 1
- high-symbol pages: 1
- errors: 0

Embedded/OCR comparison:

- OCR produced more characters on 6 pages
- embedded text produced more characters on 46 pages
- absolute character delta greater than 500 on 15 pages
- greater-than-5% embedded/OCR delta on 24 pages

Work-folder validation:

- `volume.json`, `page-quality.json`, and `review-state.json` parse successfully
- `volumeId` values match
- 52 `pages/NNNN.txt` files exist
- 52 quality entries exist
- 52 review-state entries exist
- all review statuses are currently `raw`
- validation errors: 0

Expanded hidden PSM comparison:

- modes tested: `1`, `3`, `4`, `6`, `11`, `12`
- candidates generated: 312
- wall-clock time at 14 jobs: 37,901 ms
- errors: 0
- retained OCR candidate artifact size: approximately 8.5 MB
- total local Book 3 work folder size after page images and OCR candidates: approximately 116 MB

Mode selected by crude simple score:

- PSM `1`: 19 pages
- PSM `4`: 1 page
- PSM `6`: 12 pages
- PSM `11`: 12 pages
- PSM `12`: 8 pages

Mode selected by highest mean confidence:

- PSM `1`: 13 pages
- PSM `4`: 1 page
- PSM `6`: 7 pages
- PSM `11`: 28 pages
- PSM `12`: 3 pages

Additional observations:

- PSM `1` and PSM `3` again produced identical per-page metrics on this normally oriented PDF.
- Best-confidence mode and best-simple-score mode disagreed on 36 of 52 pages.
- Mean candidate confidence was 92.1086 across candidates with confidence rows.
- Low-confidence word count across candidates was 5,271.

Transfer interpretation:

- Book 3 confirms that the Book 1 findings are not isolated to a single volume.
- Paper Capture PDFs may contain usable embedded text even when OCR is still needed as a comparison candidate.
- The greater-than-5% embedded/OCR delta threshold is sensitive enough to flag a meaningful review set.
- Hidden multi-mode OCR remains viable on the available hardware for this corpus scale.
- Confidence alone is not reliable enough to choose OCR candidates automatically.
- PSM `1` appears unnecessary when PSM `3` is already used for normally oriented pages, unless future rotated/skewed samples prove otherwise.

---

## 15. Page-Map Pressure Test

Purpose:

- Test whether keeping page label metadata inside `volume.json` becomes awkward for a larger representative volume.

Test source:

- file: `pdfs/CT The Traveller  Book.pdf`
- SHA-256: `568dbf25707b3e006ec467176e5286fc2ee5565c033a909ea2d1935ae997c602`
- size: 48,438,496 bytes
- pages: 161
- PDF profile: Adobe Acrobat 7.08 / Paper Capture Plug-in, PDF 1.4
- encrypted: no

Generated artifact:

- metadata-only sample: `work/ct-the-traveller-book-1982/volume-page-map-pressure.json`
- no extracted page text was generated for this test

Result:

- pretty-printed JSON size: 23,488 bytes
- compact JSON size: 16,421 bytes
- average pretty-printed bytes per page: approximately 146
- average compact bytes per page: approximately 102
- structural validation errors: 0

Interpretation:

- A 161-page volume does not make `volume.json.pageMap` materially large.
- Current evidence supports keeping `pageMap` inside `volume.json`.
- Splitting page metadata into a separate file should remain a monitored future trigger, not a current requirement.
