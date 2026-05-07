# ADR-0006: PDF enrichment as derived output (outline + links)

Date: 2026-05-07  
Status: **Accepted**  
Related: ADR-0002, ADR-0005, `docs/packaging-plan.md`, FR-028, NFR-003, NFR-014

## Context

Testers and end users requested:

- meaningful outline hierarchies (H1/H2-like) aligned with each volume's table of contents,
- clickable in-document navigation from outline/ToC entries,
- links to other volumes and external URLs.

Sidecar-only metadata cannot satisfy this in generic PDF readers. Achieving these behaviors in standard PDF viewers requires writing outline nodes, destinations, and link annotations into a PDF.

## Decision

Enrichment runs as an **optional derived-output pipeline**:

1. Source PDF remains unchanged.
2. The system writes a new enriched PDF artifact (for example under an output root or `dist/`-style handoff folder).
3. Enrichment inputs come from:
   - extracted/curated ToC-outline models,
   - destination mapping,
   - manual overrides for rectangle placement and heading hierarchy.
4. Manual fallback must be first-class because many legacy PDFs do not expose semantic H1/H2 tags.

This ADR does **not** approve immediate source-PDF mutation.

## Consequences

**Positive:** Enables portable navigation in any PDF viewer; supports tester workflows without manual Nitro-by-Nitro editing.

**Negative:** Adds a PDF write toolchain, coordinate-mapping complexity, and new validation burden (links, destinations, path portability).

**Risk controls:**

- Keep source PDFs immutable (aligns with NFR-003 intent).
- Require explicit operator action for enrichment generation.
- Persist an editable sidecar model for outline and link overrides.

