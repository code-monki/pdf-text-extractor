# PDF enrichment requirements draft (post-Gate 8 Phase A)

Status: Historical draft (superseded by SRS FR-034/035/036 on 2026-05-07)  
Date: 2026-05-07  
Owner intent: automate outline and link authoring into derived PDFs

---

## 1. Problem statement

Current sidecars support extraction quality and search pipelines, but do not provide portable navigation inside generic PDF readers. Testers asked for:

- outline/ToC hierarchy (H1/H2-like),
- clickable in-document navigation from outline/ToC,
- intra-document, inter-document, and URL links.

---

## 2. Candidate FRs

### C-FR-034 Outline enrichment artifact

The system shall produce or update an outline hierarchy for a derived PDF using ToC-aligned headings.

Acceptance draft:

- Outline tree supports at least level-1 and level-2 nodes.
- Each outline node resolves to a destination in the same PDF.
- Non-semantic PDFs are supported via manual hierarchy override.

### C-FR-035 Link annotation injection

The system shall inject link annotations in a derived PDF for:

- in-document destinations,
- cross-document targets (relative-path based),
- external URLs.

Acceptance draft:

- Links open expected target in standard desktop PDF viewers on primary host.
- Invalid targets are reported in diagnostics and skipped safely.

### C-FR-036 Manual fallback editor inputs

The system shall support manual override data for:

- heading hierarchy,
- destination binding,
- link rectangle coordinates.

Acceptance draft:

- Operator can provide explicit rectangle coordinates and target ids when auto-detection fails.
- Manual overrides are stored in local sidecar data and reapplied deterministically.

---

## 3. Candidate NFR impact

- **NFR-003 (source non-modification):** preserved by writing enriched PDFs as derived outputs.
- **NFR-004/NFR-005:** enrichment diagnostics record created/skipped links and unresolved targets.
- **NFR-012:** diagnostics must avoid substantive page text.
- **NFR-014:** packaging plan may include enriched PDFs and sidecar override schemas.

---

## 4. Data model draft

Potential new sidecars (names tentative):

- `outline-map.json` — heading nodes, levels, destination ids.
- `link-map.json` — link rectangles (PDF coordinates), target type (`intra`, `inter`, `url`), target payload.

These are local build artifacts unless approved for distribution.

---

## 5. Open decisions

1. PDF write backend/tool choice (qpdf/pikepdf/mutool/other).
2. Coordinate extraction strategy for auto-linking ToC lines.
3. Cross-document target addressing convention for packaged output.
4. Whether enriched PDFs are always derived copies or optionally overwrite in a guarded mode.

---

## 6. Next implementation slice (recommended)

1. Approve ADR-0006 (or revise + accept).
2. Build a single-volume prototype that writes:
   - one outline tree,
   - one intra-doc link,
   - one inter-doc link,
   - one URL link,
   all from sidecar-driven mappings.
