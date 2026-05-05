# ADR-0001: Layered native application architecture

Date: 2026-05-03  
Status: **Accepted**  
Supersedes: Informal decision captured in `docs/decision-log.md` (DL-001)  
Related: `docs/high-level-architecture.md`, `docs/detailed-design.md`, FR-001–FR-033, NFR-001–NFR-016

## Context

The system must provide local-only PDF extraction, review, validation, and diagnostics with replaceable tool adapters and testable domain logic.

## Decision

Adopt a **layered modular native application** with strict dependency direction:

1. Presentation (native UI) consumes application facades only.  
2. Application workflow coordinates domain services and adapters.  
3. Domain services own business rules and artifact contracts.  
4. External adapters (PDF view, embedded text, OCR, filesystem tools) sit at the edge and are substitutable.

OCR and other probabilistic processing remain isolated behind explicit boundaries (see ADR-0003).

## Consequences

**Positive:** Domain and application logic can be tested without UI or external tools; tools can be swapped per adapter contracts.

**Negative:** More interfaces and glue code than a monolith; requires discipline to avoid leaking UI or toolkit types into domain code.

## Compliance

Implementation MUST respect component boundaries defined in the approved HLA and DD unless this ADR is superseded by a later Accepted ADR.
