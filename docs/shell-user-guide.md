# PDF Text Extractor — Shell user guide

| Audience | Operators using the Qt Widgets reviewer (`pte_shell`) |
|----------|------------------------------------------------------|
| Related docs | [UI shell (developer)](ui-shell.md) — build, CMake, theming, architecture; [CLI reference](cli-reference.md) (`pte_bootstrap`, `pte_enrich`); [Documentation index](README.md); [Integration tutorial](integration-tutorial.md) (embedding `pdf_text_extractor_core`, optional `pte_demo_app`) |
| Help menu | **Help → Documentation** opens `docs/shell-user-guide.md` (repo dev tree or app bundle Resources). **Help → About** shows version **0.1.0** and credits. |

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
| **Preview** (center) | Scrollable PDF page view via **PDFDocumentView** (PDFium). Toolbar page navigation and the page list stay synchronized. Use **View → Preview fit width** / **Preview reset zoom** for zoom. |
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

Preview zoom uses **View → Preview fit width** and **View → Preview reset zoom** (no default shortcuts yet).

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

**Preview fit width** and **Preview reset zoom** adjust the PDFDocumentView preview column only.

### Help

- **Documentation…** — opens this user guide (`docs/shell-user-guide.md`) with the system default application for Markdown.
- **About…** — application name, packaged version string, and PDFDocumentView credit line.

## Workflow summary

1. **Open** a PDF; the application initializes or resumes the work folder and session state.
2. Select **pages** from the list or use **first / previous / next / last** controls.
3. **Edit** text in the page editor; **Save** when you want reviewed text written to disk.
4. Use **Volume metadata** for bibliographic and printed label fields.
5. Use **Readiness summary** for a high-level status picture before release-oriented steps.

For **building** the shell, Qt version, and **PDFDocumentView** dependency, see [ui-shell.md](ui-shell.md).

---

## Extraction vs preview

- **Preview** uses **PDFDocumentView** (PDFium) embedded in the shell — no Poppler `pdftoppm` required for display.
- **Embedded text candidates** and inventory still use **Poppler** (`pdftotext`, `pdfinfo`) on `PATH` when configured.
