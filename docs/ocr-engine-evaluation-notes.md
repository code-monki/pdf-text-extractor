# OCR Engine Evaluation Notes

Status: Ideation Note
Date: 2026-04-30
Lifecycle Phase: Ideation

---

## 1. Purpose

This note captures the current OCR engine evaluation position for the PDF text extractor.

It does not approve OCR engine dependencies. It defines a risk-driven evaluation order for alternate engines if Tesseract-based output is not good enough.

---

## 2. Current Position

The project owner is prepared to install additional OCR engines if needed.

This relaxes the practical experimentation constraint but does not remove the need to justify dependencies.

Evaluation should remain evidence-driven:

- first test what is already locally available
- compare outputs against local manual review samples
- install heavier engines only when the expected quality gain justifies setup, packaging, and reproducibility cost

---

## 3. Candidate Evaluation Order

### 3.1 OCRmyPDF

Status:

- installed locally
- Tesseract-based, not an independent OCR engine

Why evaluate:

- may improve preprocessing, deskewing, PDF handling, and OCR-layer workflow
- low install cost because it is already present
- directly relevant to scanned PDF pipelines

Risk:

- quality is still bounded by Tesseract
- may be better as preprocessing/orchestration than as the extractor's primary text source

### 3.2 PaddleOCR

Status:

- not currently installed
- open-source OCR/document parsing ecosystem

Why evaluate:

- plausible strongest open-source alternate OCR candidate
- may handle layout/document parsing better than direct Tesseract in some cases

Risks:

- heavier install and runtime footprint
- Python/model dependency complexity
- packaging/reproducibility burden
- output contract integration work

### 3.3 EasyOCR / RapidOCR

Status:

- not currently installed

Why evaluate:

- possible OCR comparison candidates if PaddleOCR or Tesseract underperform

Risks:

- may be more scene-text oriented or introduce additional ML dependency burden
- needs evidence before adoption

### 3.4 Kraken / Calamari

Status:

- not currently installed

Why evaluate:

- useful for some historical/manuscript/line-recognition workflows

Risks:

- less obviously suited to general scanned RPG book PDFs
- may require line segmentation workflows that add complexity

---

## 4. Evaluation Criteria

Any candidate OCR engine should be compared using:

- character/word error against local ground-truth samples
- preservation of important search terms
- snippet usefulness
- table/form/map label usefulness
- reading-order quality
- speed on representative pages
- local install burden
- offline operation
- model/package size
- reproducibility and version pinning
- ability to emit confidence or bounding metadata
- ease of integrating into the work-folder contract

---

## 5. Current Recommendation

Do not install additional OCR engines immediately.

Next experiments should be:

1. Expand or refine the local ground-truth sample if current evidence is insufficient.
2. Use that sample to determine whether direct Tesseract and multi-PSM selection are good enough.
3. If quality is insufficient, evaluate PaddleOCR first as the primary alternate OCR candidate.

This keeps dependency growth tied to observed extraction risk.

---

## 6. OCRmyPDF Experiment Result

OCRmyPDF 17.3.0 was tested against the first five-page local ground-truth sample.

Variants:

- force OCR
- force OCR with deskew and clean

Observed result:

- OCRmyPDF completed successfully.
- OCRmyPDF did not outperform direct Tesseract or embedded text on the five-page sample.
- Deskew/clean did not materially improve the result.
- OCRmyPDF output artifacts were larger because force OCR rasterized text.

Current implication:

- Keep OCRmyPDF available as a reference/preprocessing pipeline.
- Do not treat OCRmyPDF as the preferred page-text extraction path based on current evidence.
- If Tesseract quality remains insufficient after selection logic improves, evaluate PaddleOCR next.
