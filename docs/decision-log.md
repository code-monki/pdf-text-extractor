# Decision Log

Project Name: pdf-text-extractor
Version / Branch: Documentation baseline
Maintainer: Chuck, Codex
RTM Version Reference: `docs/requirements-traceability-matrix.md` v0.1 (Gate 7 exit 2026-05-04)

---

# Entry ID: DL-008

Date (YYYY-MM-DD): 2026-05-04  
Lifecycle Phase: Implementation (Gate 7 exit)  
Author: Chuck (owner directive); Codex (documentation execution)

Status: Approved

## 1. Decision Summary

Close **Gate 7 (Implementation)** and authorize **Gate 8 (Packaging)** as the next lifecycle focus. Finalize SRS scope split: **Tesseract** for FR-013 OCR candidates; Qt 6 Widgets + Poppler family; **NFR-013** / **NFR-015** Implementation-phase measurements vs full-matrix / WCAG audits deferred per SRS §17.

## 2. Evidence

- `docs/software-requirements-specification.md` §17  
- `docs/requirements-traceability-matrix.md` §9 (implementation traceability Yes)  
- `docs/phase-gate-record.md` (Gate 7 → 8 Closed)

---

# Entry ID: DL-001
Date (YYYY-MM-DD): 2026-05-01
Lifecycle Phase: High-Level Architecture
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

**Supersession note (2026-05-03):** The phase-gate wording in §2 was accurate when this entry was drafted. The High-Level Architecture is now **approved** (`docs/high-level-architecture.md`). Layering authority is recorded in **ADR-0001** (`docs/adrs/0001-layered-native-architecture.md`). Implementation is authorized under Gate 7 (`docs/phase-gate-record.md`, RTM).

---

## 1. Decision Summary

Propose a modular local native application architecture with layered dependency direction: presentation, application workflow, domain services, and external adapters.

---

## 2. Context

Requirement IDs involved:

- FR-001 through FR-033
- NFR-001 through NFR-016

Architectural Component IDs involved:

- HLA-UI
- HLA-APP
- HLA-INV
- HLA-WORK
- HLA-META
- HLA-PDF
- HLA-EXT
- HLA-OCR
- HLA-TEXT
- HLA-REVIEW
- HLA-DIAG
- HLA-VALID
- HLA-CONFIG
- HLA-DEP
- HLA-PKG
- HLA-SAFE

Non-functional drivers:

- Local-only operation
- Source material protection
- Reproducibility
- Auditability
- Sequential review usability
- Native UX
- Cross-platform support

Phase-gate context: Requirements were approved; HLA has since been approved. Use ADR-0001 for current layering authority.

---

## 3. Deterministic-Probabilistic Boundary Impact

This decision isolates OCR behind HLA-OCR and OCR-BND-001. OCR output remains probabilistic, non-authoritative, diagnostic until accepted through reviewed page text and review-state workflow.

Validation harness implications:

- OCR candidates must be validated through page-level diagnostics and human review.
- OCR candidates must not become release eligible without accepted review state.

Containment logic:

- OCR adapters cannot directly update release eligibility.
- Raw OCR candidates remain separate from reviewed page files.

Fallback behavior:

- Embedded text, manual text, skip, and quarantined states remain available.

Observability:

- Candidate availability, quality flags, errors, and tool capability data flow into diagnostics.

Reproducibility:

- Tool identity, version, and invocation profile must be reportable.

---

## 4. Alternatives Considered

- Single monolithic GUI with embedded extraction logic: rejected because extraction, validation, backup, overwrite protection, and OCR behavior must be testable without driving UI.
- CLI-first product: rejected because approved requirements center the user workflow on a native local application.
- Browser-hosted app: rejected because NFR-009 requires native local UX.
- Tool-first architecture around a selected OCR/PDF stack: rejected because dependency selection remains deferred.

---

## 5. Decision Rationale

The layered modular structure matches approved requirements while preserving architectural boundaries. It reduces coupling between native UI behavior and extraction logic, contains OCR risk, supports future tool substitution, and keeps downstream consumers dependent on explicit artifacts instead of implementation internals.

---

## 6. Impact Analysis

Requirements: All approved FR/NFR IDs receive an HLA component mapping in the RTM.

Architecture: `docs/high-level-architecture.md` defines the proposed structural model.

Detailed Design: Detailed design is approved (`docs/detailed-design.md`); refinements follow normal change control.

Implementation: Core units are authorized per RTM Gate 7; UI and packaging remain phased.

Testing: Test planning is approved (`docs/test-plan.md`); execution tracks RTM test case IDs.

Packaging: Deferred until Packaging phase and Gate 8 authorization (`docs/phase-gate-record.md`).

Orchestration: Makefile/CTest orchestration is active; CI packaging mappings remain pending.

RTM update required: Yes; incremental mapping updates continue during Gate 7.

---

## 7. Traceability References

Requirement IDs:

- FR-001 through FR-033
- NFR-001 through NFR-016

Architectural Component IDs:

- HLA components defined in `docs/high-level-architecture.md`, as amended through DD approval

Detailed Design references: `docs/detailed-design.md` v0.1 approved

Test Case IDs: Pending Test Planning phase

Packaging reference: Pending Packaging phase

Orchestration reference: Pending Orchestration phase

---

## 8. Risk Assessment

Residual risks:

- Native UI framework and packaging feasibility remain unresolved.
- External OCR/PDF dependency packaging remains unresolved.
- OCR quality remains a high-risk probabilistic subsystem.

Assumptions:

- File-system-backed local artifacts remain sufficient for current scope.
- One-PDF-at-a-time workflow remains stable.
- Downstream consumers can operate from explicit extraction artifacts.

Monitoring requirements:

- Revisit if future requirements assign broader library packaging or multi-user behavior.
- Revisit if Qt/QML or another native framework fails packaging/accessibility experiments.
- Revisit if OCR engine evidence invalidates current adapter assumptions.

Risk level: High until native framework/package feasibility and external tool strategy are resolved.

Mitigation plan:

- Conduct focused HLA risk spikes before approving HLA or before approving DD.

---

## 9. Versioning & Lineage

RTM version at time of decision: `docs/requirements-traceability-matrix.md` v0.1

System documentation version: `docs/high-level-architecture.md` v0.1 approved

Related prior Decision Log entries: None

---

## 10. Phase Impact Declaration

- Does this decision require rollback to prior phase? No
- Does this decision require revalidation of tests? No; tests are not defined yet
- Does this decision require packaging revalidation? No; packaging is not defined yet
- Does this decision alter NFR posture? No; it derives structure from approved NFRs

---

# Entry ID: DL-002
Date (YYYY-MM-DD): 2026-05-01
Lifecycle Phase: Detailed Design
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Propose Qt 6 C++ with QML/Qt Quick presentation as the native application design baseline to validate, with standard Qt/C++ Widgets retained as the fallback presentation path.

---

## 2. Context

Requirement IDs involved:

- FR-028
- NFR-009
- NFR-013
- NFR-014
- NFR-015

Architectural Component IDs involved:

- HLA-UI
- HLA-PKG

Non-functional drivers:

- Native local UX
- Cross-platform support
- Package artifact targets
- Accessibility and basic usability

Constraints:

- Backend/domain logic must remain independent of QML presentation.
- Final package execution is deferred to Packaging phase.
- Framework selection must be validated during Test Planning and the first implementation slice.

---

## 3. Deterministic-Probabilistic Boundary Impact

No direct probabilistic boundary impact. OCR-BND-001 remains unchanged.

---

## 4. Alternatives Considered

- Native platform-specific UI per OS: rejected for now because it increases maintenance and multiplies design/test surfaces.
- Browser-hosted application: rejected by NFR-009.
- CLI-first workflow: rejected by approved user workflow requirements.
- Qt Widgets-only UI: retained as fallback if QML/Quick fails feasibility, accessibility, packaging, or maintainability validation.

---

## 5. Decision Rationale

Qt 6.10.3 or newer C++ with QML/Qt Quick appears aligned with the user's Qt/C++ experience, keeps backend logic in C++, supports native desktop targets, and allows the presentation layer to remain separated from extraction and domain services. Standard Qt/C++ Widgets remains a practical fallback because it preserves the same backend/application/domain design while reducing QML-specific risk if validation fails.

---

## 6. Impact Analysis

Requirements: FR-028, NFR-009, NFR-013, NFR-014, NFR-015.

Architecture: HLA-UI and HLA-PKG.

Detailed Design: `docs/detailed-design.md` sections 3, 4.1, 4.15, 7, 9, 11, and 12.

RTM update required: Yes; DD mapping added.

---

## 7. Traceability References

Requirement IDs: FR-028, NFR-009, NFR-013, NFR-014, NFR-015

Architectural Component IDs: HLA-UI, HLA-PKG

Detailed Design references: DD 4.1, DD 4.15, DD 7, DD 9

Test Case IDs: Pending Test Planning phase

Packaging reference: Pending Packaging phase

Orchestration reference: Pending Orchestration phase

---

## 8. Risk Assessment

Residual risks:

- Linux AArch64 packaging and runtime behavior require validation.
- Accessibility behavior must be validated against native desktop conventions.
- Qt 6.10.3 or newer is the minimum baseline.
- QML/Qt Quick viability must still be validated during implementation and test planning; Qt/C++ Widgets is the fallback if validation fails.
- GitHub Actions may lag current Qt availability through system packages, so CI must install/cache Qt 6.10.3 or newer or use self-hosted runners.

Risk level: High until framework/package/accessibility spike is complete.

Mitigation plan:

- Validate Qt/QML smoke application and GitHub Actions Qt installation/cache strategy during Test Planning and the first implementation slice, or select Qt/C++ Widgets as fallback.

---

## 9. Versioning & Lineage

RTM version at time of decision: `docs/requirements-traceability-matrix.md` v0.1

System documentation version: `docs/detailed-design.md` v0.1 approved

Related prior Decision Log entries: DL-001

---

## 10. Phase Impact Declaration

- Does this decision require rollback to prior phase? No
- Does this decision require revalidation of tests? No; tests are not defined yet
- Does this decision require packaging revalidation? Yes, in Packaging phase
- Does this decision alter NFR posture? No

---

# Entry ID: DL-003
Date (YYYY-MM-DD): 2026-05-01
Lifecycle Phase: Detailed Design
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Propose Tesseract as the primary OCR adapter target, Poppler-family capabilities as the PDF adapter target, and OCRmyPDF as reserve/reference preprocessing only.

---

## 2. Context

Requirement IDs involved:

- FR-012
- FR-013
- FR-014
- FR-024
- FR-031
- NFR-004
- NFR-005
- NFR-010
- NFR-012

Architectural Component IDs involved:

- HLA-PDF
- HLA-EXT
- HLA-OCR
- HLA-DIAG
- HLA-DEP

Non-functional drivers:

- Reproducibility
- Auditability
- Observability
- Error report safety

---

## 3. Deterministic-Probabilistic Boundary Impact

This decision refines OCR-BND-001 without changing it. OCR remains probabilistic, review-gated, observable, and non-authoritative.

---

## 4. Alternatives Considered

- OCRmyPDF as primary OCR workflow: held in reserve because project experiments did not show it outperforming direct Tesseract for the representative samples.
- Multiple OCR engines in first slice: rejected for now because it increases dependency and packaging risk without enough evidence.
- Embedded-text-only workflow: rejected because representative PDFs often require OCR.

---

## 5. Decision Rationale

Tesseract and Poppler-family capabilities match prior experiment direction, are open-source-oriented, and can be isolated behind adapters. OCRmyPDF remains useful as a reserve/reference path without becoming a product dependency prematurely.

---

## 6. Impact Analysis

Requirements: FR-012, FR-013, FR-014, FR-024, FR-031, NFR-004, NFR-005, NFR-010, NFR-012.

Architecture: HLA-PDF, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-DEP.

Detailed Design: `docs/detailed-design.md` sections 4.6, 4.7, 4.8, 4.11, 4.14, 5.4, 6, 8, and 12.

RTM update required: Yes; DD mapping added.

---

## 7. Traceability References

Requirement IDs: FR-012, FR-013, FR-014, FR-024, FR-031, NFR-004, NFR-005, NFR-010, NFR-012

Architectural Component IDs: HLA-PDF, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-DEP

Detailed Design references: DD 4.6, DD 4.7, DD 4.8, DD 4.11, DD 4.14

Test Case IDs: Pending Test Planning phase

Packaging reference: Pending Packaging phase

Orchestration reference: Pending Orchestration phase

---

## 8. Risk Assessment

Residual risks:

- Poppler/Tesseract bundled-first packaging details and per-platform exceptions remain unresolved.
- OCR quality remains probabilistic and variable.
- Comparative OCR mode is included in the first implementation slice but may increase runtime and complexity.

Risk level: High until dependency packaging strategy is resolved.

Mitigation plan:

- Keep adapter boundaries, record tool versions/configuration, and resolve bundled-first per-platform packaging details during Test Planning, Implementation, and Packaging.

---

## 9. Versioning & Lineage

RTM version at time of decision: `docs/requirements-traceability-matrix.md` v0.1

System documentation version: `docs/detailed-design.md` v0.1 approved

Related prior Decision Log entries: DL-001

---

## 10. Phase Impact Declaration

- Does this decision require rollback to prior phase? No
- Does this decision require revalidation of tests? No; tests are not defined yet
- Does this decision require packaging revalidation? Yes, in Packaging phase
- Does this decision alter NFR posture? No

---

# Entry ID: DL-004
Date (YYYY-MM-DD): 2026-05-01
Lifecycle Phase: Detailed Design
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Add application theming as a traceable UI capability with a dedicated HLA-THEME component and DD theme service.

---

## 2. Context

Requirement IDs involved:

- FR-033
- NFR-009
- NFR-015
- NFR-016

Architectural Component IDs involved:

- HLA-UI
- HLA-THEME

Non-functional drivers:

- Native local UX
- Accessibility and basic usability
- Theme accessibility

Constraints:

- Theme support applies to application chrome and controls, not source PDF rendering fidelity.
- Theme preference must remain per-user local application configuration, not project/work-folder metadata or downstream extraction output.

---

## 3. Deterministic-Probabilistic Boundary Impact

No deterministic-probabilistic boundary impact. OCR-BND-001 remains unchanged.

---

## 4. Alternatives Considered

- Treat theme selection as untracked UI polish: rejected because it affects accessibility, persistence, UI architecture, and test coverage.
- Store theme preference in work-folder/project metadata: rejected because themes are per-user application-local preferences, not extraction artifacts.
- Allow arbitrary custom palettes in the first design pass: deferred because built-in validated themes reduce accessibility risk.

---

## 5. Decision Rationale

Theming is expected in modern native applications and directly affects usability. Adding a dedicated HLA-THEME component preserves traceability, avoids overloading HLA-UI, and keeps per-user theme state separate from extraction artifacts and project/work-folder metadata.

---

## 6. Impact Analysis

Requirements: FR-033, NFR-016.

Architecture: HLA-UI and HLA-THEME.

Detailed Design: `docs/detailed-design.md` sections 4.1, 4.17, 5.4, 7, and 8.

RTM update required: Yes; FR-033 and NFR-016 mappings added.

---

## 7. Traceability References

Requirement IDs: FR-033, NFR-016

Architectural Component IDs: HLA-UI, HLA-THEME

Detailed Design references: DD 4.1, DD 4.17, DD 5.4, DD 7

Test Case IDs: NFR-THEME-001 planned

Packaging reference: Pending Packaging phase

Orchestration reference: Pending Orchestration phase

---

## 8. Risk Assessment

Residual risks:

- Theme palettes could weaken contrast, focus visibility, or diagnostics readability.
- Theme behavior may differ between QML/Quick and Qt Widgets fallback.

Risk level: Moderate.

Mitigation plan:

- Validate built-in themes for contrast, visible focus, tooltip readability, and diagnostics/status legibility during Test Planning and UI implementation.

---

## 9. Versioning & Lineage

RTM version at time of decision: `docs/requirements-traceability-matrix.md` v0.1

System documentation version: `docs/detailed-design.md` v0.1 approved

Related prior Decision Log entries: DL-001, DL-002

---

## 10. Phase Impact Declaration

- Does this decision require rollback to prior phase? Yes; SRS and HLA were amended in place before DD approval
- Does this decision require revalidation of tests? No; tests are not defined yet
- Does this decision require packaging revalidation? Yes, in Packaging phase
- Does this decision alter NFR posture? Yes; NFR-016 was added

---

# Entry ID: DL-005
Date (YYYY-MM-DD): 2026-05-02
Lifecycle Phase: Implementation
Author: Codex

Status:
- Approved for current implementation slice

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Use a top-level `Makefile` as the repository orchestration entry point while delegating actual build and test execution to CMake and CTest.

---

## 2. Context

Requirement IDs involved:

- FR-017
- FR-018
- FR-025
- FR-031
- NFR-011
- NFR-012
- NFR-014

Architectural Component IDs involved:

- HLA-WORK
- HLA-VALID
- HLA-PKG
- HLA-SAFE

Non-functional drivers:

- Reproducible local execution
- Auditability
- Package target visibility
- Source material protection

Constraints:

- Packaging phase has not defined release artifacts, dependency bundling, versioning, or package validation.
- The orchestrator must not package source PDFs or derived full-text artifacts.

---

## 3. Deterministic-Probabilistic Boundary Impact

No direct OCR boundary impact. The orchestrator preserves test execution as the validation entry point and does not bypass review-gated OCR containment rules.

---

## 4. Alternatives Considered

- Use only raw CMake commands: rejected because the orchestration guardrail requires a primary documented entry point.
- Add package generation immediately: rejected because Packaging phase has not defined approved package artifacts or validation.
- Add dependency installation automation now: deferred because external PDF/OCR and Qt packaging strategy remains unresolved.

---

## 5. Decision Rationale

A top-level `Makefile` provides a stable local entry point for `tool-check`, `configure`, `build`, `test`, `check`, `clean`, `distclean`, and `clean-build` while keeping CMake as the build system of record. The `package` target fails clearly until formal Packaging work authorizes concrete artifacts.

---

## 6. Impact Analysis

Requirements: FR-017, FR-018, FR-025, FR-031, NFR-011, NFR-012, NFR-014.

Architecture: HLA-WORK, HLA-VALID, HLA-PKG, HLA-SAFE.

Detailed Design: Current backend-domain implementation and validation harness remain unchanged.

RTM update required: Yes, when orchestration mappings are formally consolidated.

---

## 7. Traceability References

Requirement IDs: FR-017, FR-018, FR-025, FR-031, NFR-011, NFR-012, NFR-014

Architectural Component IDs: HLA-WORK, HLA-VALID, HLA-PKG, HLA-SAFE

Detailed Design references: `docs/detailed-design.md` validation, work-folder, package boundary, and safe-reporting sections

Test Case IDs: TC-FR-017, TC-FR-018, TC-FR-025, TC-FR-031, TC-NFR-011, TC-NFR-012, TC-NFR-014

Packaging reference: Pending Packaging phase

Orchestration reference: `Makefile`

---

## 8. Risk Assessment

Residual risks:

- Package artifact generation remains undefined.
- Dependency installation automation remains deferred until toolchain/package decisions are stable.
- Cross-platform behavior needs validation outside the current macOS environment.

Risk level: Moderate.

Mitigation plan:

- Keep package target explicit but failing until Packaging phase approval.
- Continue using CMake/CTest as the executable validation spine.
- Validate clean build through `make clean-build` during implementation.

---

## 9. Versioning & Lineage

RTM version at time of decision: `docs/requirements-traceability-matrix.md` v0.1

System documentation version: `docs/detailed-design.md` v0.1 approved

Related prior Decision Log entries: DL-001, DL-002, DL-003

---

## 10. Phase Impact Declaration

- Does this decision require rollback to prior phase? No
- Does this decision require revalidation of tests? Yes; orchestrated test execution must pass
- Does this decision require packaging revalidation? No; packaging is not defined yet
- Does this decision alter NFR posture? No

---

# Entry ID: DL-006
Date (YYYY-MM-DD): 2026-05-03
Lifecycle Phase: Implementation (governance alignment)
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Establish **Accepted ADRs** under `docs/adrs/` for layering, work-folder contracts, OCR boundaries, Qt presentation baseline, and Poppler-family PDF inspection; record **phase gate** status in `docs/phase-gate-record.md`; refresh **AGENTS.md** lifecycle wording; implement **`pdfinfo` argv inspection** (`src/core/pdf_inspection_poppler.*`) per ADR-0005.

---

## 2. Context

Requirement IDs involved:

- FR-002, FR-031, NFR-004, NFR-010 (inspection and dependency reporting posture)

Architectural references:

- ADR-0001 through ADR-0005
- `docs/phase-gate-record.md`
- `ai-toolkit/02-governance/12-phase-gate-checklist.md`

---

## 3. Impact Analysis

- ADRs become the structural authority pointer required by `AGENTS.md` section 6.
- Packaging and orchestration remain explicitly deferred until Gate 8.
- PDF structural inspection moves from placeholder caller-supplied evidence toward adapter-backed **pdfinfo** execution on POSIX; Windows remains a documented stub until a native inspection path is authorized.

---

## 4. Traceability References

RTM: `docs/requirements-traceability-matrix.md` — FR-002 implementation mapping updated for `pdf_inspection_poppler.*`.

Tests: `tests/core_tests.cpp` — pdfinfo stdout parsing and inspection edge cases.

---

# Entry ID: DL-007
Date (YYYY-MM-DD): 2026-05-03
Lifecycle Phase: Implementation
Author: Codex

Status:
- Approved

If superseded, reference replacement Entry ID.

---

## 1. Decision Summary

Publish **`docs/cli-reference.md`** as the authoritative operator artifact listing all **`pte_bootstrap`** switches, exit codes, outputs, and examples; cross-link from the RTM and phase-gate record.

---

## 2. Context

Requirement IDs involved:

- FR-024, FR-031, NFR-014 (orchestration, dependency overrides, safe reporting)

---

## 3. Impact Analysis

- Agents and operators have a single documentation target for CLI behavior alongside `src/cli/bootstrap_main.cpp`.
- Packaging phase can cite `docs/cli-reference.md` for installed-binary help parity.

---

## 4. Traceability References

- `docs/cli-reference.md`
- `docs/requirements-traceability-matrix.md` (CLI reference line)
- `docs/phase-gate-record.md` (headless CLI subsection)

---

End of Decision Log
