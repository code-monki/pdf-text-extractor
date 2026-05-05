# ADR-0003: OCR probabilistic boundary (OCR-BND-001)

Date: 2026-05-03  
Status: **Accepted**  
Related: `docs/test-plan.md` (OCR containment), FR-013–FR-015, FR-018, FR-024, NFR-010, NFR-012

## Context

OCR output is **probabilistic** and must not silently authorize downstream release or “accepted” review status.

## Decision

1. OCR adapters produce **raw candidates** and diagnostics only.  
2. OCR routing may **schedule** candidate generation but does not change review state or reviewed text.  
3. Release/indexing eligibility remains gated on **reviewed page text** and **review-state**, not on OCR output alone.

## Consequences

**Positive:** Failures and low-quality OCR remain visible; deterministic workflows stay authoritative.

**Negative:** Extra steps for operators to review and accept pages; diagnostics must stay accurate.
