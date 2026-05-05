# ADR-0004: Native presentation baseline (Qt / QML)

Date: 2026-05-03  
Status: **Accepted**  
Related: `docs/detailed-design.md` (presentation), FR-028–FR-033, NFR-009, NFR-013–NFR-016

## Context

Requirements call for a **native local** review experience with a GUI-centered workflow. The DD evaluated Qt with QML/Quick for presentation.

## Decision

Baseline presentation stack: **Qt 6.x** with **QML / Qt Quick**, calling **C++ application facades** that encapsulate domain services.

If QML is not viable for a target, fallback is **Qt Widgets** over the **same** C++ facades—presentation swaps without changing domain contracts.

## Amendment (2026-05-04)

The `pte_shell` prototype ships as **Qt Widgets** (`ShellMainWindow` + `ReviewSessionFacade`) because it matches the project’s working native-dialog pattern and reduces presentation-layer risk. **QML / Qt Quick remains an acceptable future presentation** for this codebase as long as domain logic stays behind the same facade boundary (per the original decision).

## Consequences

**Positive:** Aligns with DD; keeps extraction logic toolkit-independent below the facade layer.

**Negative:** Qt dependency and licensing considerations for packaging; cross-platform GUI validation effort.
