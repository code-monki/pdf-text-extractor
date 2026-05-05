# Reusable Volume Packaging Concept

Status: Ideation Note
Date: 2026-05-01
Lifecycle Phase: Ideation
Source Context: Broader electronic-document library system discussion
Governance Reference: `ai-toolkit/02-governance/01-ideation-guardrail.md`

---

## 1. Purpose

This note captures early concepts for packaging individual electronic-document volumes in a consistent, reusable manner.

The immediate extractor project exists to produce reliable page text and metadata for downstream systems. The broader library-system goal may need a common package format that can be reused across corpora, rulesets, indexers, browser tools, QA tools, and local archival workflows.

This is not a Requirements artifact, architecture decision, or packaging plan. It is a place to preserve the concept so it can be reviewed later without forcing the extractor project to become a full document-library platform prematurely.

---

## 2. Boundary

The extractor should produce package-ready ingredients:

- source PDF identity and metadata
- reviewed page text
- page quality diagnostics
- review state
- page-label mapping
- local-only raw extraction candidates

A reusable library package may consume those ingredients, but it may also include concerns outside the extractor:

- corpus membership
- collection grouping
- package manifests
- packaging paths
- downstream index configuration
- library catalog metadata
- cover/thumb assets
- provenance and versioning
- release/export policies

The extractor should not silently assume ownership of the entire library package model unless later Requirements explicitly assign that responsibility.

---

## 3. Candidate Package Shape

A reusable volume package could use a predictable directory layout:

```text
volumes/
  <volume-id>/
    volume.json
    page-quality.json
    review-state.json
    pages/
      0001.txt
      0002.txt
    assets/
      cover.png
      thumbnails/
        0001.png
    source/
      source-pdf-reference.json
    raw/
      embedded/
      ocr/
```

Current packaging instincts:

- `volume.json` is the primary package metadata file for one volume.
- `pageMap` remains in `volume.json` unless future evidence shows that it is too large or semantically too broad.
- Source PDFs may be referenced by identity/path/hash without being copied into a distributable package.
- Raw extraction candidates remain local-only by default.
- Reviewed page text is package content only when the operator explicitly approves the package/export target.
- Corpus or collection grouping should be represented above the volume when available.

---

## 4. Candidate Volume Metadata

Volume-level metadata should likely include:

- stable `volumeId`
- title
- subtitle
- sort title
- group/category within a corpus, if applicable
- publisher
- publication year
- original publication year, when different
- edition or edition label
- language
- optional ISBN
- product code
- creators or contributors
- series
- series number or volume number
- description
- subjects or tags
- rights or copyright notice
- optional cover page reference
- source PDF identity
- page count
- page map
- indexing intent
- copyright/local-use notes
- review notes

Publisher, publication year, original publication year, edition, language, ISBN, product code, creators, series, series number, description, subjects, and rights are volume-level bibliographic metadata. They should be initialized by extraction/import when known and remain user-editable. Most of these fields should be optional because many volumes may not expose them.

Metadata editing concept:

- A base set of common fields can be shown by default.
- Less common bibliographic fields can be added, revealed, or left empty through the metadata edit dialog.
- This preserves complete metadata coverage without forcing every volume to carry the same amount of manually entered detail.

Cover concept:

- Future bookshelf or visual library views may need a cover image.
- The user should be able to specify the cover page when a useful cover exists.
- Cover metadata should reference the source PDF page and/or generated cover asset.
- This avoids ambiguous automatic assumptions such as treating page 1 as the cover for every volume.

---

## 5. Candidate Corpus Metadata

Corpus-level metadata may include:

- corpus ID
- corpus title
- collection/grouping definitions
- default publisher or rights notes
- root paths for source PDFs and generated work folders
- packaging/export profiles
- downstream index targets

This suggests collection grouping should not be forced into each volume as the only authority. A volume may carry enough metadata for standalone use, but corpus-level grouping should be available when managing a library.

---

## 6. Open Questions

- Should a reusable library package include source PDFs, or only references to local source PDFs?
- Should reviewed page text be packaged by default, or only exported into explicit release packages?
- What is the minimum package manifest needed for another tool to trust a volume package?
- Should package versioning track extractor version, source PDF hash, review state, or all of these?
- Should corpus-level grouping live in a separate corpus manifest, a package manifest, or both?
- Should the package format support multiple text layers, such as reviewed text, embedded candidate, OCR candidate, and normalized export text?
- What package profile is needed for local development versus distribution?

---

## 7. Carry-Forward Position

The current extractor output contract should stay simple and volume-centered.

The broader library-system package concept should be developed as a reusable downstream contract only after Requirements clarify whether this project owns packaging directly or merely emits package-ready artifacts.
