# UI shell (Qt Widgets)

| Field | Value |
|-------|--------|
| Status | Initial layout slice (implementation phase) |
| Date | 2026-05-03 (Widgets shell 2026-05-04) |
| Target binary | `pte_shell` |
| Sources | `src/ui/main.cpp`, `src/ui/app_theme.cpp`, `src/ui/shell_main_window.cpp`, `src/ui/review_session_facade.cpp` |
| CMake option | `PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON` (default **OFF** so core library and tests build without Qt) |
| Typography | `main.cpp` sets **`QFontDatabase::systemFont(QFontDatabase::GeneralFont)`** so controls use the OS UI font (e.g. SF on macOS), not Fusion’s generic default |

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

## Operator documentation (Help menu target)

End-user orientation for menus, **icon toolbar**, shortcuts, and workflows lives in **`docs/shell-user-guide.md`**. That file is the intended body for a future **Help → Documentation** action (`QDesktopServices` or bundled HTML). **Help → About** should surface version/credits per the same guide’s “Planned Help integration” section.

---

## Scope of this slice

- **In scope:** Application window, **File** / **Edit** menus and main toolbar (**Volume metadata…**, shortcut **Ctrl+M** / **⌘M** on macOS via Qt) opening `VolumeMetadataDialog` for `volume.json` (FR-006 / NFR-009), including the **Pages and cover** tab for printed page labels and cover page ID (FR-008 / FR-010); toolbar **review sync** summary line (`ReviewSessionFacade::reviewSyncSummary`, NFR-006); horizontal splitter (page list / **thin PDF preview** / page text); status bar; `ReviewSessionFacade` wiring. On open, when **`pdftotext`** is on `PATH`, the shell runs **embedded candidate extraction** for all pages (OCR off by default for speed). Reviewed files under `pages/` start empty; the editor **shows embedded candidate text** until you save, which persists reviewed page text. Preview rasterizes the current page via **Poppler `pdftoppm`** (`PopplerPageRenderer`); if `pdftoppm` is missing, the middle column shows a short unavailable message.
- **Out of scope:** Full in-window PDF viewer (search-in-preview, continuous scroll, embedded link navigation), file system tree beyond page IDs, full bibliographic field matrix — follow-up slices.

---

## Theming (FR-033, NFR-016, HLA-THEME)

Requirements (**`docs/software-requirements-specification.md`**): **FR-033** — user-selectable application themes, persisted **per user** locally (not work-folder artifacts). **NFR-016** — built-in themes keep readable contrast for chrome and core controls. DD: **`docs/detailed-design.md`** §4.17 (HLA-THEME). Decision context: **`docs/decision-log.md`**, **`docs/adrs/0004-presentation-qt-qml-baseline.md`**.

**Implemented in `pte_shell`:**

- **`app_theme.cpp`** — **Fusion** style, three built-in theme ids (`builtin.light`, `builtin.dark`, `builtin.sepia`), preference in **`QSettings`** (`pdf-text-extractor` / `ui` / `theme`). Restored in **`main.cpp`** before the main window is shown.
- **`View` → `Theme`** — exclusive actions; save + apply immediately.
- **Debug:** **`validateBuiltInThemesContrastHint()`** at startup; **`qWarning`** if WCAG-style checks fail.

Preview column: **Poppler** image is unchanged; only the widget chrome around it follows the palette.

---

## Traceability

- Requirements: FR-006 (volume metadata edit dialog), FR-008 / FR-010 (printed labels and cover in metadata UI), FR-028 (native application framing; thin Poppler preview supports PDF review in-shell), FR-033 (theme selection + persistence), NFR-006 (review metadata aligned with page selection), NFR-009 (native UX baseline), NFR-016 (theme contrast sanity checks).
- Operator documentation: **`docs/shell-user-guide.md`** (menus, icon toolbar, shortcuts, planned Help integration).
- Wireframe reference: `docs/images/pdf-text-extractor-wireframe-v2.svg`.
- ADR: `docs/adrs/0004-presentation-qt-qml-baseline.md` (Widgets amendment).
