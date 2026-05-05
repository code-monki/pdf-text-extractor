# ADR-0002: Work-folder artifact and downstream contract

Date: 2026-05-03  
Status: **Accepted**  
Related: `docs/detailed-design.md` (work-folder sections), FR-004–FR-011, FR-024–FR-027

## Context

Downstream consumers (`ct-cd` and similar) must rely on **explicit files and schemas**, not hidden application state or OCR internals.

## Decision

The **work folder** is the authoritative extraction surface:

- Deterministic paths and schema-versioned JSON (`volume.json`, `review-state.json`, `page-quality.json`, etc.).  
- Reviewed page text in `pages/NNNN.txt` is the human-approved source for release/indexing eligibility.  
- Raw embedded and OCR candidates remain separate from reviewed text until the review workflow promotes content.

Atomic writes (temp + rename) apply where the DD specifies them.

## Consequences

**Positive:** Traceable, inspectable artifacts; clear boundary for packaging and downstream indexers.

**Negative:** Schema evolution requires explicit migration or versioning discipline.
