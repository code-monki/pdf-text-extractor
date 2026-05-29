# Documentation index

**Project version** (build / CPack): **0.1.0** — see root `CMakeLists.txt` (`project(pdf_text_extractor VERSION …)`). User-facing release notes: [`CHANGELOG.md`](../CHANGELOG.md) in the repository root.

**Lifecycle:** Gates 1–9 closed (2026-05-29); deferred **Packaging Phase B** and orchestration — [`phase-gate-record.md`](phase-gate-record.md).

---

## Operator-facing

| Document | Audience |
| -------- | -------- |
| [`cli-reference.md`](cli-reference.md) | `pte_bootstrap` switches; `pte_enrich` prototype |
| [`shell-user-guide.md`](shell-user-guide.md) | Qt reviewer `pte_shell` (menus, toolbar, shortcuts) |
| [`packaging-plan.md`](packaging-plan.md) | Phase A archives (`make package`), lint, what ships |

---

## Developer / integrator

| Document | Audience |
| -------- | -------- |
| [`ui-shell.md`](ui-shell.md) | Build `pte_shell` (`PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL`), Qt, theming |
| [`integration-tutorial.md`](integration-tutorial.md) | Link `pdf_text_extractor_core` in-process; optional **`pte_demo_app`** (`PDF_TEXT_EXTRACTOR_BUILD_DEMO_APP`, default OFF) |
| [`enrichment-sidecar-schema.md`](enrichment-sidecar-schema.md) | Outline/link sidecars for `pte_enrich` |

---

## Requirements and traceability

| Document | Role |
| -------- | ---- |
| [`software-requirements-specification.md`](software-requirements-specification.md) | SRS (authority); §17 Implementation vs Packaging scope |
| [`requirements-traceability-matrix.md`](requirements-traceability-matrix.md) | RTM |
| [`high-level-architecture.md`](high-level-architecture.md) | HLA |
| [`detailed-design.md`](detailed-design.md) | DD |
| [`test-plan.md`](test-plan.md) | Test plan |
| [`adrs/README.md`](adrs/README.md) | Accepted ADR index |

---

## Governance

Repository operational contract: [`AGENTS.md`](../AGENTS.md) (repository root).

**Out of scope until separately authorized:** downstream orchestration runbooks; Packaging Phase B installers and signing (see `phase-gate-record.md`).
