# UI shell (Qt Widgets)

| Field | Value |
|-------|--------|
| Status | Initial layout slice (implementation phase) |
| Date | 2026-05-03 (Widgets shell 2026-05-04) |
| Target binary | `pte_shell` |
| Sources | `src/ui/main.cpp`, `src/ui/shell_main_window.cpp`, `src/ui/review_session_facade.cpp` |
| CMake option | `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON` (default **OFF** so core library and tests build without Qt) |

---

## Visual guidance: mockup vs wireframe

- **`docs/pdf-text-extractor-mockup-review.md`** states the raster mockup is **not** an approved final UI spec; it captures **workflow intent** (one PDF at a time, side-by-side page vs text, metadata, status).
- **`docs/images/pdf-text-extractor-wireframe-v2.svg`** provides **adequate information architecture** for the first shell: title bar, menu row, toolbar strip, **three-column** main area (tree / PDF preview / page text), and **footer** status strip. The first `pte_shell` build uses a **horizontal splitter** with page list, preview placeholder, and page text; exact proportions are **not** binding.
- **Detailed design** (`docs/detailed-design.md` §4.1 HLA-UI) remains authoritative for behavior (facades, read models). The shell calls **`ReviewSessionFacade`** from C++ only (no direct tool calls from the view layer).

---

## Building

Requires **Qt 6.5 or newer** installed and discoverable by CMake. Point CMake at the **platform kit** that contains `lib/cmake/Qt6/Qt6Config.cmake` (not the parent `Qt` directory alone).

On **this machine**, Qt lives under `/Users/chuck/Qt`; the macOS 6.9.3 kit prefix is:

```text
/Users/chuck/Qt/6.9.3/macos
```

Example configure and build:

```bash
cmake -S . -B build \
  -DPDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON \
  -DCMAKE_PREFIX_PATH=/Users/chuck/Qt/6.9.3/macos
cmake --build build --target pte_shell
```

Design baseline in DD references **Qt 6.10.3+**; **6.9.x** is acceptable for early shell iteration until the baseline is upgraded.

---

## Launch

After a successful build, the orchestrator can start the UI:

```bash
make shell-run
```

One-step build and launch (sets `SHELL_BUILD_DIR` to `build-qt-shell` by default):

```bash
make shell-go QT_PREFIX=/path/to/Qt/6.x.x/macos
```

**Artifacts** (Ninja or Unix Makefiles, default layout):

| Platform | Path under `SHELL_BUILD_DIR` |
|----------|------------------------------|
| macOS app bundle | `src/ui/pte_shell.app` (multi-config generators may use `src/ui/Debug/pte_shell.app` or `…/Release/…`) |
| Linux | `src/ui/pte_shell` |
| Windows | `src/ui/pte_shell.exe` |

**macOS manual launch:** `open build-qt-shell/src/ui/pte_shell.app` (adjust if you changed `SHELL_BUILD_DIR`).

---

## Scope of this slice

- **In scope:** Application window, menus, toolbar strip, horizontal splitter (page list / **thin PDF preview** / page text), status bar, wired `ReviewSessionFacade` (open PDF, page selection, load/save reviewed page text). Preview rasterizes the current page via **Poppler `pdftoppm`** (`PopplerPageRenderer`); if `pdftoppm` is missing, the middle column shows a short unavailable message.
- **Out of scope:** Full in-window PDF viewer (search-in-preview, continuous scroll, embedded link navigation), file system tree beyond page IDs, persistence beyond page text save, metadata editing UI — follow-up slices.

---

## Traceability

- Requirements: FR-028 (native application framing; thin Poppler preview supports PDF review in-shell), NFR-009 (native UX baseline).
- Wireframe reference: `docs/images/pdf-text-extractor-wireframe-v2.svg`.
- ADR: `docs/adrs/0004-presentation-qt-qml-baseline.md` (Widgets amendment).
