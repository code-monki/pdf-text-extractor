# UI shell (Qt Widgets)

| Field | Value |
|-------|--------|
| Status | Initial layout slice (implementation phase) |
| Date | 2026-05-03 (Widgets shell 2026-05-04) |
| Target binary | `pte_shell` |
| Sources | `src/ui/main.cpp`, `src/ui/app_theme.cpp`, `src/ui/shell_main_window.cpp`, `src/ui/review_session_facade.cpp` |
| CMake option | `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON` (default **OFF** so core library and tests build without Qt) |
| Typography | `main.cpp` sets **`QFontDatabase::systemFont(QFontDatabase::GeneralFont)`** so controls use the OS UI font (e.g. SF on macOS), not Fusion’s generic default |

**Library integrators (no Qt):** to build the optional C++ demo binary **`pte_demo_app`** that links `pdf_text_extractor_core`, set **`PDF_TEXT_EXTRACTOR_BUILD_DEMO_APP=ON`** (default OFF). Walkthrough: **[integration-tutorial.md](integration-tutorial.md)**; documentation index: **[README.md](README.md)**.

---

## Visual guidance: mockup vs wireframe

- **`docs/pdf-text-extractor-mockup-review.md`** states the raster mockup is **not** an approved final UI spec; it captures **workflow intent** (one PDF at a time, side-by-side page vs text, metadata, status).
- **`docs/images/pdf-text-extractor-wireframe-v2.svg`** provides **adequate information architecture** for the first shell: title bar, menu row, toolbar strip, **three-column** main area (tree / PDF preview / page text), and **footer** status strip. The first `pte_shell` build uses a **horizontal splitter** with page list, preview placeholder, and page text; exact proportions are **not** binding.
- **Detailed design** (`docs/detailed-design.md` §4.1 HLA-UI) remains authoritative for behavior (facades, read models). The shell calls **`ReviewSessionFacade`** from C++ only (no direct tool calls from the view layer).

---

## Packaging

CPack archives that can include `pte_shell` (when built) are described in **`docs/packaging-plan.md`**. Default **`make package`** uses the same `BUILD_DIR` as headless builds unless you configure a Qt-enabled tree separately.

---

## Building

The default CMake tree (for example `cmake -S . -B build` or `make configure`) sets **`PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL` to OFF**. In that configuration **there is no `pte_shell` target**, so `cmake --build build --target pte_shell` fails with “No rule to make target”. Enable the option (and set **`CMAKE_PREFIX_PATH`** to your Qt 6 kit) to generate the shell target, or use **`make shell QT_PREFIX=…`**, which configures **`SHELL_BUILD_DIR`** (default `build-qt-shell`) with the shell enabled.

Requires **Qt 6.5 or newer** and a checkout of **[PDFDocumentView](https://github.com/code-monki/PDFDocumentView)** for the preview column (PDFium backend). CMake option **`PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT`** must point at that tree when **`PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON`**. If unset, configure tries **`../PDFDocumentView`** relative to this repository (sibling checkout).

Point CMake at the **platform Qt kit** that contains `lib/cmake/Qt6/Qt6Config.cmake` (not the parent `Qt` directory alone).

On **this machine**, Qt lives under `/Users/chuck/Qt`; the macOS 6.9.3 kit prefix is:

```text
/Users/chuck/Qt/6.9.3/macos
```

Example configure and build:

```bash
cmake -S . -B build-qt-shell \
  -DPDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON \
  -DPDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT=/path/to/PDFDocumentView \
  -DCMAKE_PREFIX_PATH=/Users/chuck/Qt/6.9.3/macos
cmake --build build-qt-shell --target pte_shell
```

Or with sibling checkout `../PDFDocumentView`:

```bash
cmake -S . -B build \
  -DPDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON \
  -DCMAKE_PREFIX_PATH=/Users/chuck/Qt/6.9.3/macos
cmake --build build --target pte_shell
```

Design baseline in DD references **Qt 6.10.3+**; **6.9.x** is acceptable for early shell iteration until the baseline is upgraded.

**Upgrading to Qt 6.11+ (macOS, parallel installs):** Qt keeps each minor version in its own directory (for example `~/Qt/6.11.2/macos` next to `6.9.3`). Use one of:

1. **Qt Maintenance Tool** — If you installed Qt with the official installer, run **`~/Qt/MaintenanceTool.app`** (or **Applications → Qt Maintenance Tool**), sign in, choose **Add or remove components**, open **Qt 6.11.x** → **Desktop** → **macOS**, select the kit, apply. [Qt docs: Get and install Qt](https://doc.qt.io/qt-6/get-and-install-qt.html).
2. **Qt Online Installer** — Download from [qt.io Download](https://www.qt.io/download), open-source account required; during setup pick **Custom** and add **Qt 6.11.x** for macOS.
3. **Command line (`aqt`)** — `pip install aqtinstall`, then `aqt list-qt mac desktop --arch 6.11` to see the architecture id, then `aqt install-qt mac desktop 6.11.<patch> <arch> -O ~/Qt` (see [aqtinstall](https://aqtinstall.readthedocs.io/)).

After install, point CMake at the new kit: **`-DCMAKE_PREFIX_PATH=$HOME/Qt/6.11.x/macos`** (adjust patch folder to match what was installed).

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

## Operator documentation (Help menu)

End-user orientation for menus, **icon toolbar**, shortcuts, and workflows lives in **`docs/shell-user-guide.md`**. **`pte_shell`** implements **Help → Documentation** (opens that guide via `QDesktopServices`) and **Help → About** (version + credits).

---

## Scope of this slice

- **In scope:** Application window, **File** / **Edit** / **View** / **Help** menus and main toolbar; **PDFDocumentView** (`PdfDocumentViewWidget`) preview column (PDFium) with **in-preview find** and **link-map host overlays**; page list / preview / text synchronization via `ReviewSessionFacade`; volume metadata dialog; readiness summary; **Help → Check extraction tools** (FR-031 via `DependencyCapabilityService`); themes; Help → Documentation / About.
- **Preview:** Embedded **PDFDocumentView** — scrollable page view with fit-width / reset-zoom (**View** menu), PDF text search (**Edit** / **View → Find in preview**), and optional **link-map overlays** from work-folder sidecars (FR-035/036 preview). Poppler **`pdftoppm` is not used for shell preview** (Poppler remains required for extraction/inventory when those features are enabled).
- **Out of scope (follow-up):** Clickable in-preview link navigation (overlays are visual only), file system tree beyond page IDs.

---

## Theming (FR-033, NFR-016, HLA-THEME)

Requirements (**`docs/software-requirements-specification.md`**): **FR-033** — user-selectable application themes, persisted **per user** locally (not work-folder artifacts). **NFR-016** — built-in themes keep readable contrast for chrome and core controls. DD: **`docs/detailed-design.md`** §4.17 (HLA-THEME). Decision context: **`docs/decision-log.md`**, **`docs/adrs/0004-presentation-qt-qml-baseline.md`**.

**Implemented in `pte_shell`:**

- **`app_theme.cpp`** — **Fusion** style, three built-in theme ids (`builtin.light`, `builtin.dark`, `builtin.sepia`), preference in **`QSettings`** (`pdf-text-extractor` / `ui` / `theme`). Restored in **`main.cpp`** before the main window is shown.
- **`View` → `Theme`** — exclusive actions; save + apply immediately.
- **Debug:** **`validateBuiltInThemesContrastHint()`** at startup; **`qWarning`** if WCAG-style checks fail.

Preview column: **PDFDocumentView** renders page content; application chrome (themes) wraps the widget via Qt palettes.

---

## Traceability

- Requirements: FR-006, FR-008 / FR-010, FR-028 (native shell + PDFDocumentView preview), FR-031 (extraction tool report), FR-035/FR-036 (link-map preview overlays), FR-033, NFR-006, NFR-009, NFR-016.
- Operator documentation: **`docs/shell-user-guide.md`** (Help → Documentation target; implemented).
- Wireframe reference: `docs/images/pdf-text-extractor-wireframe-v2.svg`.
- ADR: `docs/adrs/0004-presentation-qt-qml-baseline.md` (Widgets amendment).
