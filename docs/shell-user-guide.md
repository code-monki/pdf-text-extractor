# PDF Text Extractor — Shell user guide

| Audience | Operators using the Qt Widgets reviewer (`pte_shell`) |
|----------|------------------------------------------------------|
| Related docs | [UI shell (developer)](ui-shell.md) — build, CMake, theming, architecture |
| Help menu (planned) | **Documentation** should open this file (bundled or repo copy); **About** should show version and credits (see [§ Planned Help integration](#planned-help-integration)). |

---

## What this application does

The shell lets you open **one PDF at a time**, associate it with a **local work folder**, and **review or edit per-page text** stored under `pages/NNNN.txt`. Embedded text candidates come from Poppler tools where configured; OCR and deeper pipelines follow project rules in the SRS and detailed design. This guide describes **only the window**: menus, toolbar, shortcuts, and columns.

---

## Main areas

| Area | Purpose |
|------|---------|
| **Menu bar** | Full text for every command (File, Edit, View → Theme). |
| **Main toolbar** | Same commands as key File/Edit actions, shown as **icons** with **tooltips** (hover to see the full description and shortcuts). |
| **Page list** (left) | One row per page ID; click to change the current page. |
| **Preview** (center) | Thumbnail of the current PDF page via Poppler `pdftoppm` when available. |
| **Page text** (right) | Editable text for the current page; **Save** writes `pages/NNNN.txt` per the facade rules. |
| **Status bar** | Session messages from the review facade. |
| **Review line on toolbar** | Short summary of review metadata sync for the current page (NFR-006). |

---

## Toolbar icons (icon-only; text in tooltips)

Icons use the **current Qt style**’s standard pixmaps so they match the OS theme (e.g. Fusion on Linux, macOS style hints).

| Approximate control | Standard pixmap role | Meaning |
|---------------------|---------------------|---------|
| Open | Open / dialog open | Choose a source PDF and open or resume its session. |
| Save page text | Save | Persist the editor contents for the current page. |
| Volume metadata | File dialog “information list” | Edit `volume.json` (title, notes, bibliographic fields, page labels, cover). |
| Re-extract embedded | Browser reload | Re-run Poppler `pdftotext` for **all** pages into raw embedded candidates; does **not** delete reviewed `pages/*.txt`. |
| Readiness summary | Information | Dialog with safe readiness counts from local metadata (no substantive page text). |
| First page | Media skip backward | Go to page 1. |
| Previous page | Arrow left | Previous page. |
| Next page | Arrow right | Next page. |
| Last page | Media skip forward | Go to last page. |

---

## Keyboard shortcuts

Shortcuts apply **globally** where Qt assigns them to the action (application shortcut context).

| Action | Shortcut (typical) |
|--------|---------------------|
| Open PDF | Platform **Open** (e.g. Ctrl+O / ⌘O) |
| Quit | Platform **Quit** |
| Volume metadata | **Ctrl+M** (⌘M on macOS) |
| First page | **Ctrl+Home** |
| Last page | **Ctrl+End** |

Other toolbar actions rely on the menu or toolbar unless additional shortcuts are added later.

---

## Menus

### File

- **Open PDF…** — Same as toolbar open.
- **Save Page Text** — Enabled when a page is selected.
- **Volume metadata…** — Requires an active work folder.
- **Re-extract embedded candidates…** — Requires a work folder; see toolbar table.
- **Readiness summary…** — Requires a work folder.
- **Quit** — Exit the application.

### Edit

Duplicates **Volume metadata**, **Re-extract embedded candidates**, and **Readiness summary** for convenience.

### View → Theme

Choose a built-in theme (light, dark, sepia). Preference is stored per user on this machine (`QSettings`), not in the work folder.

---

## Workflow summary

1. **Open** a PDF; the application initializes or resumes the work folder and session state.
2. Select **pages** from the list or use **first / previous / next / last** controls.
3. **Edit** text in the page editor; **Save** when you want reviewed text written to disk.
4. Use **Volume metadata** for bibliographic and printed label fields.
5. Use **Readiness summary** for a high-level status picture before release-oriented steps.

For **building** the shell, Qt version, and **preview dependencies**, see [ui-shell.md](ui-shell.md).

---

## Planned Help integration

When **Help → Documentation** is implemented, the application should open this document:

- **Development:** `docs/shell-user-guide.md` in the repository (or a generated HTML copy).
- **Packaged build:** ship the same content beside the binary or inside the app bundle `Resources/`, and open with `QDesktopServices::openUrl` or a small HTML viewer.

When **Help → About** is implemented, show at minimum:

- Application name **PDF Text Extractor**
- Version string from build metadata (CMake / `QApplication::applicationVersion()`)
- Short credit line (project / license reference as required by your packaging)
- Optional link to project documentation root

Traceability: FR-028 (native shell), NFR-009 (native UX); Help actions may be tracked under future SRS items when added.
