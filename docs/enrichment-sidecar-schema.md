# Enrichment sidecar schema (prototype)

Status: Gate 8 prototype schema for FR-034/035/036  
Date: 2026-05-07  
Scope: Sidecar inputs for derived PDF enrichment (`pte_enrich`, `PdfEnrichmentService`)

---

## 1. `outline-map.json`

Purpose: define H1/H2-like hierarchy and destination bindings for a derived PDF outline tree.

```json
{
  "schemaVersion": 1,
  "nodes": [
    {
      "id": "ch1",
      "title": "Chapter 1",
      "level": 1,
      "destination": { "pageIndex": 3, "yPt": 712.0 },
      "manual": true
    },
    {
      "id": "ch1-sec1",
      "title": "Character Creation",
      "level": 2,
      "destination": { "pageIndex": 4, "yPt": 640.0 },
      "manual": false
    }
  ]
}
```

Required fields per node:

- `title` (string)
- `level` (number; intended 1+)
- `destination` (object; destination payload is pipeline-specific)

Optional fields:

- `id` (string)
- `manual` (bool)

---

## 2. `link-map.json`

Purpose: define link rectangles and targets for in-document, inter-document, or URL navigation.

```json
{
  "schemaVersion": 1,
  "links": [
    {
      "pageIndex": 0,
      "rect": [72.0, 700.0, 312.0, 720.0],
      "target": { "type": "intra", "destinationId": "ch1" },
      "manual": true
    },
    {
      "pageIndex": 0,
      "rect": [72.0, 620.0, 312.0, 640.0],
      "target": { "type": "inter", "volumeId": "book-2", "destinationId": "intro" }
    },
    {
      "pageIndex": 0,
      "rect": [72.0, 590.0, 312.0, 610.0],
      "target": { "type": "url", "url": "https://example.org" }
    }
  ]
}
```

Required per link:

- `pageIndex` (number; zero-based PDF page for annotation placement)
- `rect` (array of four numbers, PDF coordinate space)
- `target` (object, must include `type`)

Target type rules:

- `intra`: requires `destination` object or `destinationId` that matches an outline node `id`.
- `inter`: requires `destinationId` and either `path` or `volumeId`.
- `url`: requires `url` beginning with `http://` or `https://`.

Optional:

- `manual` (bool)

---

## 3. Report output

Prototype writes a safe report JSON (`enrichment-report.json`) with:

- source/derived paths
- outline node count
- link count
- manual override count
- warnings array
- prototype status message

No substantive PDF page text is included.

---

## 4. Schema version policy (prototype)

Current accepted value: `schemaVersion = 1` for both sidecar files.  
Unsupported versions are rejected safely with non-destructive failure.

