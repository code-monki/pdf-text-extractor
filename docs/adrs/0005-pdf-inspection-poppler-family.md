# ADR-0005: PDF structural inspection via Poppler-family tools

Date: 2026-05-03  
Status: **Accepted**  
Related: HLA-PDF, HLA-INV, `docs/detailed-design.md` §4.6–4.7, FR-002, NFR-004

## Context

Page count, encryption hints, and readability signals are required for inventory and routing. Qt PDF and Poppler-family tooling are candidates; core logic must not hard-code one vendor library when a narrow **inspection** capability suffices.

## Decision

For **non-mutating structural inspection** (e.g. page count, encrypted flag from tool output), use **Poppler-family CLI tools** (e.g. `pdfinfo`) behind a dedicated adapter type, with configurable executable paths for reproducibility and testing.

Rendering and embedded-text extraction may use different adapters (Qt PDF, `pdftotext`, etc.) but MUST remain behind adapter interfaces per HLA/DD.

## Consequences

**Positive:** No PDF library link dependency for inspection-only paths; consistent with exploration notes; easy to version external tools.

**Negative:** Requires Poppler (or compatible) installs on developer machines; subprocess orchestration and PATH handling must stay secure (no shell interpolation; argv execution).
