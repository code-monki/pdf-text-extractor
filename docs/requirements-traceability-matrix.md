# Requirements Traceability Matrix (RTM)

Project Name: pdf-text-extractor
Version: 0.1
Date (YYYY-MM-DD): 2026-05-01
Maintained By: Codex
Status: Implementation In Progress; Requirement-to-Test Mapping Added
Requirement Version Reference: `docs/software-requirements-specification.md` v0.1
Architecture Version Reference: `docs/high-level-architecture.md` v0.1 approved
Design Version Reference: `docs/detailed-design.md` v0.1 approved
Test Plan Version Reference: `docs/test-plan.md` v0.1 approved  
ADR Index Reference: `docs/adrs/README.md`  
Phase Gate Record Reference: `docs/phase-gate-record.md`  
CLI Reference (`pte_bootstrap`): `docs/cli-reference.md`  
Qt Shell (layout slice): `docs/ui-shell.md`

---

# 1. RTM Authority Declaration

Confirm:

- Requirements approved? Yes
- Architecture approved? Yes
- Detailed Design approved? Yes
- Test Plan aligned? Yes, approved
- Traceability Consolidation authorized? Yes
- Advancement to Test Planning authorized? Yes
- Advancement to Implementation authorized? Yes

Traceability is complete through approved Detailed Design and approved Test Planning. Implementation mappings are updated incrementally as implemented units land; packaging and orchestration mappings remain pending until their lifecycle phases produce approved artifacts.

---

# 2. Purpose

This RTM establishes stable requirement identifiers and approved mappings through Architecture and Detailed Design. It remains the authoritative traceability artifact for later mapping to tests, implementation, packaging, and orchestration.

At this phase:

- requirements are approved
- architecture is approved
- detailed design is approved
- traceability consolidation is complete
- test planning is approved
- implementation is active

Implementation mappings are filled incrementally as code lands. Packaging and Orchestration phases must fill their pending mapping columns when those phases are authorized.

---

# 3. Core Traceability Matrix

<table style="font-size: 0.85em;">
  <thead>
    <tr>
      <th style="white-space: nowrap;">Req ID</th>
      <th style="white-space: nowrap;">Req Type</th>
      <th style="white-space: nowrap;">HLA Component ID</th>
      <th style="white-space: nowrap;">DD Artifact</th>
      <th style="white-space: nowrap;">Implementation Unit</th>
      <th style="white-space: nowrap;">Test Case ID</th>
      <th style="white-space: nowrap;">Packaging Ref</th>
      <th style="white-space: nowrap;">Orchestration Ref</th>
      <th style="white-space: nowrap;">Validation Status</th>
      <th style="white-space: nowrap;">Evidence Ref</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="white-space: nowrap;">FR-001</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-INV, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.1, 4.3, 4.6</td>
      <td style="white-space: nowrap;">src/core/source_inventory.*; src/core/pdf_inspection_poppler.*; tests/core_tests.cpp; tests/fixtures/hello.pdf</td>
      <td style="white-space: nowrap;">TC-FR-001</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-002</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-DIAG, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.3, 4.11</td>
      <td style="white-space: nowrap;">src/core/source_inventory.* (inventoryWithPopplerInspection); src/core/pdf_inspection_poppler.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-002</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-003</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-INV</td>
      <td style="white-space: nowrap;">DD 4.3</td>
      <td style="white-space: nowrap;">src/core/source_inventory.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-003</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-004</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4</td>
      <td style="white-space: nowrap;">src/core/volume_bootstrap.*, src/core/validation_repair.*, src/core/work_folder_initializer.*, src/core/json.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-004</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-005</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-META</td>
      <td style="white-space: nowrap;">DD 4.5</td>
      <td style="white-space: nowrap;">src/core/work_folder_initializer.*, src/core/json.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-005</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-006</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-META</td>
      <td style="white-space: nowrap;">DD 4.1, 4.5</td>
      <td style="white-space: nowrap;">src/core/volume_metadata.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-006</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-007</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-META</td>
      <td style="white-space: nowrap;">DD 4.1, 4.5</td>
      <td style="white-space: nowrap;">src/core/volume_metadata.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-007</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-008</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-META</td>
      <td style="white-space: nowrap;">DD 4.5</td>
      <td style="white-space: nowrap;">src/core/volume_metadata.*, src/core/work_folder_initializer.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-008</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-009</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-META</td>
      <td style="white-space: nowrap;">DD 4.5, 5.2</td>
      <td style="white-space: nowrap;">src/core/work_folder_initializer.*, src/core/page_id.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-009</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-010</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-META</td>
      <td style="white-space: nowrap;">DD 4.1, 4.5</td>
      <td style="white-space: nowrap;">src/core/volume_metadata.*, src/core/work_folder_initializer.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-010</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-011</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-TEXT</td>
      <td style="white-space: nowrap;">DD 4.4, 4.9, 5.5</td>
      <td style="white-space: nowrap;">src/core/reviewed_page_text.*, src/core/work_folder_initializer.*, src/core/page_id.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-011</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-012</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-EXT</td>
      <td style="white-space: nowrap;">DD 4.7</td>
      <td style="white-space: nowrap;">src/core/candidate_text.*, src/core/poppler_embedded_text.*, src/core/candidate_generation_poppler.*, src/core/process_runner.*, src/core/work_folder_initializer.*; tests/core_tests.cpp; tests/fixtures/hello.pdf</td>
      <td style="white-space: nowrap;">TC-FR-012</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Adapter slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-013</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-OCR</td>
      <td style="white-space: nowrap;">DD 4.8</td>
      <td style="white-space: nowrap;">src/core/candidate_text.*, src/core/tesseract_ocr.*, src/core/candidate_generation_poppler.*, src/core/process_runner.*, src/core/work_folder_initializer.*; tests/core_tests.cpp; tests/fixtures/hello.pdf</td>
      <td style="white-space: nowrap;">TC-FR-013</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Adapter slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-014</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-PDF, HLA-OCR</td>
      <td style="white-space: nowrap;">DD 4.6, 4.8</td>
      <td style="white-space: nowrap;">src/core/ocr_routing.*, src/core/page_quality.*, src/core/poppler_page_render.*, src/core/candidate_generation_poppler.*, src/core/process_runner.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-014</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Adapter slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-015</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-EXT, HLA-OCR, HLA-TEXT</td>
      <td style="white-space: nowrap;">DD 4.1, 4.7, 4.8, 4.9</td>
      <td style="white-space: nowrap;">src/core/candidate_text.*, src/core/reviewed_page_text.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-015</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-016</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-TEXT</td>
      <td style="white-space: nowrap;">DD 4.1, 4.9</td>
      <td style="white-space: nowrap;">src/core/reviewed_page_text.*, src/core/text_normalization.*, src/core/file_change_detector.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-016</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-017</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-APP, HLA-REVIEW</td>
      <td style="white-space: nowrap;">DD 4.1, 4.2, 4.10, 5.3</td>
      <td style="white-space: nowrap;">src/core/work_folder_initializer.*, src/core/review_state.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-017</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-018</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-REVIEW, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.10, 4.16</td>
      <td style="white-space: nowrap;">src/core/review_state.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-018</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-019</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6</td>
      <td style="white-space: nowrap;">src/core/readiness_summary.*, src/core/work_folder_initializer.*, src/core/work_folder_validator.*, src/core/poppler_page_render.*, src/core/candidate_generation_poppler.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-019</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-020</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6</td>
      <td style="white-space: nowrap;">src/core/readiness_summary.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-020</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-021</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-REVIEW</td>
      <td style="white-space: nowrap;">DD 4.1, 4.10</td>
      <td style="white-space: nowrap;">src/core/review_workflow.*, src/core/readiness_summary.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-021</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-022</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-APP, HLA-TEXT, HLA-REVIEW</td>
      <td style="white-space: nowrap;">DD 4.1, 4.2, 4.9, 4.10</td>
      <td style="white-space: nowrap;">src/core/reviewed_page_text.*, src/core/file_change_detector.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-022</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-023</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK, HLA-TEXT</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4, 4.9</td>
      <td style="white-space: nowrap;">src/core/work_folder_backup.*, src/core/artifact_cleanup.*, src/core/candidate_text.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-023</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-024</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-EXT, HLA-OCR, HLA-DIAG</td>
      <td style="white-space: nowrap;">DD 4.7, 4.8, 4.11, 5.4</td>
      <td style="white-space: nowrap;">src/core/page_quality.*, src/core/volume_extraction_pipeline.*, src/core/candidate_generation_poppler.*, src/core/ocr_routing.*, src/core/readiness_summary.*, src/core/work_folder_initializer.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-024</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-025</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-VALID, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.12, 4.16</td>
      <td style="white-space: nowrap;">src/core/work_folder_initializer.*, src/core/json.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-025</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-026</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-VALID</td>
      <td style="white-space: nowrap;">DD 4.2, 4.12</td>
      <td style="white-space: nowrap;">src/core/validation_repair.*, src/core/work_folder_backup.*, src/core/artifact_cleanup.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-026</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-027</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-APP, HLA-DIAG, HLA-VALID, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.1, 4.2, 4.11, 4.12, 4.16</td>
      <td style="white-space: nowrap;">src/core/readiness_summary.*, src/core/review_workflow.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-027</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-028</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.1, 4.15</td>
      <td style="white-space: nowrap;">src/ui/main.cpp, src/ui/shell_main_window.*, src/ui/review_session_facade.*; tests/tc_fr028_probe.cpp (when shell enabled)</td>
      <td style="white-space: nowrap;">TC-FR-028</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">Qt Widgets pte_shell + tc_fr028_probe when PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON; metadata UI still deferred</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-029</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-CONFIG</td>
      <td style="white-space: nowrap;">DD 4.4, 4.13</td>
      <td style="white-space: nowrap;">src/core/output_config.*, src/core/source_inventory.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-029</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-030</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK, HLA-CONFIG, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4, 4.13, 4.16</td>
      <td style="white-space: nowrap;">src/core/artifact_cleanup.*, src/core/output_config.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-030</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-031</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-OCR, HLA-DIAG, HLA-DEP, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.8, 4.11, 4.14, 4.15</td>
      <td style="white-space: nowrap;">src/core/readiness_summary.*, src/core/dependency_capability.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-031</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-032</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4</td>
      <td style="white-space: nowrap;">src/core/reviewed_page_text.*, src/core/file_change_detector.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-FR-032</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-033</td>
      <td style="white-space: nowrap;">FR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-THEME</td>
      <td style="white-space: nowrap;">DD 4.1, 4.17, 5.4, 7</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-FR-033</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-001</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-WORK, HLA-CONFIG, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.3, 4.4, 4.13, 4.16, 7</td>
      <td style="white-space: nowrap;">src/core/local_path_intent.*, src/core/output_config.*, src/core/source_inventory.*, src/core/volume_bootstrap.*, src/ui/review_session_facade.cpp; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-001</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">Hosted URI scheme rejection; TC-NFR-001 tests; core library has no network client APIs</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-002</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-CONFIG, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.4, 4.13, 4.16, 7</td>
      <td style="white-space: nowrap;">src/core/output_config.*, src/core/artifact_cleanup.*, src/core/work_folder_backup.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-002</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-003</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-PDF, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.3, 4.6, 4.16, 7</td>
      <td style="white-space: nowrap;">src/core/file_digest.*, src/core/source_inventory.*; tests/core_tests.cpp (TC-NFR-003 hash asserts in Poppler fixture tests)</td>
      <td style="white-space: nowrap;">TC-NFR-003</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">Shared sha256HexFile; hash equality after candidate gen, volume extract, bootstrap; review/validation UI paths not exhaustively hashed</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-004</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-VALID, HLA-DEP</td>
      <td style="white-space: nowrap;">DD 4.3, 4.7, 4.8, 4.11, 4.12, 4.14, 7</td>
      <td style="white-space: nowrap;">src/core/dependency_capability.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-004</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-005</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-META, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-VALID, HLA-REVIEW</td>
      <td style="white-space: nowrap;">DD 4.2, 4.5, 4.7, 4.8, 4.10, 4.11, 4.12, 7</td>
      <td style="white-space: nowrap;">src/core/page_quality.*, src/core/readiness_summary.*, src/core/work_folder_backup.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-005</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-006</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6, 7</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-NFR-006</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-007</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK, HLA-TEXT, HLA-REVIEW, HLA-VALID</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4, 4.9, 4.10, 4.12, 7</td>
      <td style="white-space: nowrap;">src/core/file_change_detector.*, src/core/work_folder_backup.*, src/core/artifact_cleanup.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-007</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-008</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-META, HLA-CONFIG</td>
      <td style="white-space: nowrap;">DD 4.3, 4.5, 4.13, 7</td>
      <td style="white-space: nowrap;">src/core/output_config.*, src/core/source_inventory.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-008</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-009</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF, HLA-PKG, HLA-THEME</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6, 4.15, 4.17, 7</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-NFR-009</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-010</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-DEP</td>
      <td style="white-space: nowrap;">DD 4.2, 4.7, 4.8, 4.11, 4.14, 7</td>
      <td style="white-space: nowrap;">src/core/dependency_capability.*, src/core/ocr_routing.*, src/core/page_quality.*, src/core/readiness_summary.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-010</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-03</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-011</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-META, HLA-TEXT, HLA-VALID, HLA-CONFIG</td>
      <td style="white-space: nowrap;">DD 4.4, 4.5, 4.9, 4.12, 4.13, 5, 7</td>
      <td style="white-space: nowrap;">src/core/work_folder_initializer.*, src/core/json.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-011</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-012</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-OCR, HLA-TEXT, HLA-DIAG, HLA-VALID, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.8, 4.9, 4.11, 4.12, 4.16, 7</td>
      <td style="white-space: nowrap;">src/core/validation_repair.*, src/core/readiness_summary.*, src/core/reviewed_page_text.*, src/core/file_change_detector.*, src/core/work_folder_validator.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-012</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-013</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-DEP, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.14, 4.15, 7, 9</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-NFR-013</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-014</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-DEP, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.14, 4.15, 9</td>
      <td style="white-space: nowrap;">src/core/dependency_capability.*; tests/core_tests.cpp</td>
      <td style="white-space: nowrap;">TC-NFR-014</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Implemented slice passing</td>
      <td style="white-space: nowrap;">ctest core_tests 2026-05-02</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-015</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PKG, HLA-THEME</td>
      <td style="white-space: nowrap;">DD 4.1, 4.15, 4.17, 7</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-NFR-015</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-016</td>
      <td style="white-space: nowrap;">NFR</td>
      <td style="white-space: nowrap;">HLA-THEME, HLA-UI</td>
      <td style="white-space: nowrap;">DD 4.1, 4.17, 7</td>
      <td style="white-space: nowrap;">Pending implementation</td>
      <td style="white-space: nowrap;">TC-NFR-016</td>
      <td style="white-space: nowrap;">Pending packaging</td>
      <td style="white-space: nowrap;">Pending orchestration</td>
      <td style="white-space: nowrap;">Planned</td>
      <td style="white-space: nowrap;">SRS v0.1</td>
    </tr>
  </tbody>
</table>

---

# 4. Deterministic-Probabilistic Boundary Traceability

<table style="font-size: 0.85em;">
  <thead>
    <tr>
      <th style="white-space: nowrap;">Req ID</th>
      <th style="white-space: nowrap;">Boundary ID</th>
      <th style="white-space: nowrap;">Validation Harness</th>
      <th>Containment Logic</th>
      <th style="white-space: nowrap;">Fallback Ref</th>
      <th style="white-space: nowrap;">Observability Ref</th>
      <th style="white-space: nowrap;">Drift Validation</th>
      <th style="white-space: nowrap;">Status</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="white-space: nowrap;">FR-012</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-002</td>
      <td>Raw embedded candidate isolated from reviewed text</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;"><code>src/core/candidate_text.*</code>; <code>page-quality.json</code> integration pending</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-013</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-001, TC-BND-OCR-002</td>
      <td>Raw OCR candidate isolated from reviewed text</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;"><code>src/core/candidate_text.*</code>; <code>page-quality.json</code> integration pending</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-014</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-003</td>
      <td>Paper Capture OCR candidate routing remains review-gated</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">routing diagnostics planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-015</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-002</td>
      <td>Human comparison and replacement of candidates</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;"><code>src/core/candidate_text.*</code>; selected source metadata update pending</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-016</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-001</td>
      <td>Human-reviewed text is authoritative for acceptance</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">review state planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-018</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-001</td>
      <td>Only accepted pages enter release/indexing by default</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">readiness summary planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">FR-024</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-004, TC-BND-OCR-005</td>
      <td>Quality flags expose suspicious candidate output</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">diagnostics planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-004</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-004</td>
      <td>Reproducibility metadata planned</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">extraction diagnostics planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-005</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-004</td>
      <td>Page-level auditability planned</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">diagnostics and review state planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-012</td>
      <td style="white-space: nowrap;">OCR-BND-001</td>
      <td style="white-space: nowrap;">TC-BND-OCR-006</td>
      <td>Reports omit substantive text</td>
      <td style="white-space: nowrap;">DD 4.8, 7</td>
      <td style="white-space: nowrap;">safe reports planned</td>
      <td style="white-space: nowrap;">Tool/version capture; no exact text assertion</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
  </tbody>
</table>

---

# 5. Non-Functional Traceability

<table style="font-size: 0.85em;">
  <thead>
    <tr>
      <th style="white-space: nowrap;">NFR ID</th>
      <th style="white-space: nowrap;">Architectural Mechanism</th>
      <th style="white-space: nowrap;">Design Artifact</th>
      <th style="white-space: nowrap;">Test Case</th>
      <th>Packaging Impact</th>
      <th style="white-space: nowrap;">Status</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="white-space: nowrap;">NFR-001</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-WORK, HLA-CONFIG, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.3, 4.4, 4.13, 4.16, 7</td>
      <td style="white-space: nowrap;">TC-NFR-001</td>
      <td>Local source inventory covered; package expectations pending</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-002</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-CONFIG, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.4, 4.13, 4.16, 7</td>
      <td style="white-space: nowrap;">TC-NFR-002</td>
      <td>Ignore/local-only rules pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-003</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-PDF, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.3, 4.6, 4.16, 7</td>
      <td style="white-space: nowrap;">TC-NFR-003</td>
      <td>Inventory hash-stability covered; PDF adapter handling pending</td>
      <td style="white-space: nowrap;">Partial implementation passing</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-004</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-VALID, HLA-DEP</td>
      <td style="white-space: nowrap;">DD 4.3, 4.7, 4.8, 4.11, 4.12, 4.14, 7</td>
      <td style="white-space: nowrap;">TC-NFR-004</td>
      <td>Tool/version metadata pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-005</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-META, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-VALID, HLA-REVIEW</td>
      <td style="white-space: nowrap;">DD 4.2, 4.5, 4.7, 4.8, 4.10, 4.11, 4.12, 7</td>
      <td style="white-space: nowrap;">TC-NFR-005</td>
      <td>Diagnostic artifacts pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-006</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6, 7</td>
      <td style="white-space: nowrap;">TC-NFR-006</td>
      <td>Native UI packaging pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-007</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-WORK, HLA-TEXT, HLA-REVIEW, HLA-VALID</td>
      <td style="white-space: nowrap;">DD 4.2, 4.4, 4.9, 4.10, 4.12, 7</td>
      <td style="white-space: nowrap;">TC-NFR-007</td>
      <td>Backup artifact policy pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-008</td>
      <td style="white-space: nowrap;">HLA-INV, HLA-META, HLA-CONFIG</td>
      <td style="white-space: nowrap;">DD 4.3, 4.5, 4.13, 7</td>
      <td style="white-space: nowrap;">TC-NFR-008</td>
      <td>Corpus configuration pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-009</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PDF, HLA-PKG, HLA-THEME</td>
      <td style="white-space: nowrap;">DD 4.1, 4.6, 4.15, 4.17, 7</td>
      <td style="white-space: nowrap;">TC-NFR-009</td>
      <td>Native application packaging pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-010</td>
      <td style="white-space: nowrap;">HLA-APP, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-DEP</td>
      <td style="white-space: nowrap;">DD 4.2, 4.7, 4.8, 4.11, 4.14, 7</td>
      <td style="white-space: nowrap;">TC-NFR-010</td>
      <td>Progress/log artifact handling pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-011</td>
      <td style="white-space: nowrap;">HLA-WORK, HLA-META, HLA-TEXT, HLA-VALID, HLA-CONFIG</td>
      <td style="white-space: nowrap;">DD 4.4, 4.5, 4.9, 4.12, 4.13, 5, 7</td>
      <td style="white-space: nowrap;">TC-NFR-011</td>
      <td>Schema/version metadata pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-012</td>
      <td style="white-space: nowrap;">HLA-OCR, HLA-TEXT, HLA-DIAG, HLA-VALID, HLA-SAFE</td>
      <td style="white-space: nowrap;">DD 4.8, 4.9, 4.11, 4.12, 4.16, 7</td>
      <td style="white-space: nowrap;">TC-NFR-012</td>
      <td>Safe report packaging pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-013</td>
      <td style="white-space: nowrap;">HLA-DEP, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.14, 4.15, 7, 9</td>
      <td style="white-space: nowrap;">TC-NFR-013</td>
      <td>Multi-platform native application packaging pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-014</td>
      <td style="white-space: nowrap;">HLA-DEP, HLA-PKG</td>
      <td style="white-space: nowrap;">DD 4.14, 4.15, 9</td>
      <td style="white-space: nowrap;">TC-NFR-014</td>
      <td>AppImage, deb, rpm, dmg, and msi packaging targets pending</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-015</td>
      <td style="white-space: nowrap;">HLA-UI, HLA-PKG, HLA-THEME</td>
      <td style="white-space: nowrap;">DD 4.1, 4.15, 4.17, 7</td>
      <td style="white-space: nowrap;">TC-NFR-015</td>
      <td>Accessibility expectations may affect native UI packaging validation</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
    <tr>
      <td style="white-space: nowrap;">NFR-016</td>
      <td style="white-space: nowrap;">HLA-THEME, HLA-UI</td>
      <td style="white-space: nowrap;">DD 4.1, 4.17, 7</td>
      <td style="white-space: nowrap;">TC-NFR-016</td>
      <td>Theme accessibility expectations may affect native UI validation</td>
      <td style="white-space: nowrap;">Planned</td>
    </tr>
  </tbody>
</table>

---

# 6. Bidirectional Verification Rules

Forward and backward tracing is complete through approved Detailed Design and approved Test Planning. Implementation tracing is now incremental. Full lifecycle tracing cannot be completed until Implementation, Packaging, and Orchestration phases produce approved artifacts.

Current rule for Implementation phase:

- Every approved FR and NFR must appear in this RTM.
- Every approved FR and NFR must map to at least one HLA component, one DD reference, and one Test Case ID.
- Every HLA component must appear in the RTM and DD.
- Implementation mappings must reference actual implemented units.
- No packaging or orchestration mapping may be invented before its lifecycle phase.
- Pending packaging and orchestration mappings must be replaced during the appropriate later phase.

---

# 7. Orphan Detection

Current Implementation orphan check:

- SRS requirements without RTM rows: none known
- RTM requirement rows without SRS requirements: none known
- HLA components without RTM/DD coverage: none known
- DD HLA component references without approved HLA parent: none known
- Test case mappings without requirement basis: none known
- Implementation units without RTM basis: none known
- Package orphans: not applicable; Packaging phase is not authorized

---

# 8. Change Control and Lineage

| Date | Change Summary | Impacted IDs | Phase Rollback Required |
|------|----------------|--------------|-------------------------|
| 2026-05-01 | Initial RTM scaffold created from SRS v0.1 draft. | FR-001 through FR-030; NFR-001 through NFR-012 | No |
| 2026-05-01 | Added platform support NFR and RTM rows. | NFR-013 | No |
| 2026-05-01 | Added dependency reporting, external modification detection, package artifact targets, and accessibility usability requirements. | FR-031, FR-032, NFR-014, NFR-015 | No |
| 2026-05-01 | Completed Requirements stabilization pass and marked RTM ready for approval review. | All FR/NFR IDs | No |
| 2026-05-01 | Requirements approved by project owner; HLA is authorized as the next lifecycle phase. | All FR/NFR IDs | No |
| 2026-05-01 | Draft HLA component mappings added to RTM. | All FR/NFR IDs | No |
| 2026-05-01 | HLA approved by project owner; DD is authorized as the next lifecycle phase. | All FR/NFR IDs | No |
| 2026-05-01 | Draft DD section mappings added to RTM. | All FR/NFR IDs | No |
| 2026-05-01 | Added UI theming requirement, theme accessibility NFR, HLA theme component, and DD theme mappings. | FR-033, NFR-016, HLA-THEME | No |
| 2026-05-01 | DD decisions accepted and DD approved by project owner. | All FR/NFR IDs | No |
| 2026-05-01 | Traceability consolidation performed through approved DD baseline. | All FR/NFR and HLA IDs | No |
| 2026-05-02 | Traceability Consolidation approved and Test Planning authorized. | All FR/NFR and HLA IDs | No |
| 2026-05-02 | Draft Test Plan created and RTM Test Case ID mappings added. | All FR/NFR IDs; OCR-BND-001 | No |
| 2026-05-02 | Test Plan approved and Implementation authorized. | All FR/NFR IDs | No |
| 2026-05-02 | First backend validation implementation slice added and tested. | FR-011, FR-017, FR-018, FR-025, NFR-011, NFR-012 | No |
| 2026-05-02 | Work-folder initialization implementation slice added and tested. | FR-004, FR-005, FR-009, FR-011, FR-017, FR-024, FR-025, NFR-011 | No |
| 2026-05-02 | Dependency capability reporting implementation slice added and tested. | FR-031, NFR-004, NFR-010, NFR-014 | No |
| 2026-05-02 | External file modification detection implementation slice added and tested. | FR-032, NFR-007, NFR-012 | No |
| 2026-05-02 | Reviewed page text load/save implementation slice added and tested. | FR-016, FR-022, FR-032, NFR-007, NFR-012 | No |
| 2026-05-02 | Review transition and release eligibility implementation slice added and tested. | FR-017, FR-018, OCR-BND-001 | No |
| 2026-05-02 | Readiness summary implementation slice added and tested. | FR-024, FR-027, NFR-005, NFR-010, NFR-012 | No |
| 2026-05-02 | Validation repair dry-run planning implementation slice added and tested. | FR-025, FR-026, NFR-007, NFR-012 | No |
| 2026-05-02 | Raw candidate text storage/comparison/restore implementation slice added and tested. | FR-012, FR-013, FR-015, FR-016, FR-022, FR-024, NFR-012, OCR-BND-001 | No |
| 2026-05-02 | Source PDF inventory and duplicate detection implementation slice added and tested. | FR-001, FR-002, FR-003, NFR-001, NFR-003, NFR-004 | No |
| 2026-05-03 | Output configuration, protected backup/restore, and artifact cleanup implementation slices added and tested. | FR-023, FR-026, FR-029, FR-030, NFR-001, NFR-002, NFR-007, NFR-008 | No |
| 2026-05-03 | OCR routing and page-quality diagnostic persistence implementation slices added and tested. | FR-014, FR-024, FR-031, NFR-004, NFR-005, NFR-010, NFR-012, OCR-BND-001 | No |
| 2026-05-03 | Volume metadata, review workflow, and text normalization implementation slices added and tested. | FR-005, FR-006, FR-007, FR-008, FR-010, FR-016, FR-021, FR-024, NFR-005, NFR-008, NFR-011, NFR-012 | No |
| 2026-05-03 | Accepted ADRs, phase-gate record, AGENTS lifecycle refresh; Poppler pdfinfo inspection adapter (`pdf_inspection_poppler`) and tests. | FR-002, NFR-004, governance | No |
| 2026-05-03 | Poppler/Tesseract toolchain adapters: `process_runner`, `poppler_embedded_text`, `poppler_page_render`, `tesseract_ocr`, `candidate_generation_poppler`; synthetic fixture `tests/fixtures/hello.pdf`; aggregate integration test. | FR-012, FR-013, FR-014, NFR-004, NFR-010 | No |
| 2026-05-03 | Inventory `inventoryWithPopplerInspection`; `VolumeCandidateExtractService` + `volume_extraction_pipeline` rewriting `page-quality.json` from extraction; RTM tests. | FR-001, FR-002, FR-024 | No |
| 2026-05-03 | `VolumeBootstrapService` (`volume_bootstrap`): pdfinfo inventory → work-folder init → optional extraction; integration tests. | FR-004, HLA-APP | No |
| 2026-05-03 | Headless CLI `pte_bootstrap` (`src/cli/bootstrap_main.cpp`) calling `VolumeBootstrapService`; tool path overrides. | FR-024, FR-031, NFR-014 | No |
| 2026-05-03 | Operator artifact `docs/cli-reference.md` documenting all `pte_bootstrap` switches; RTM and phase-gate cross-links; DL-007. | FR-024, FR-031, NFR-014, governance | No |
| 2026-05-03 | Qt Quick shell `pte_shell` (optional CMake flag); `docs/ui-shell.md` — mockup vs wireframe adequacy, build instructions; aligns with `pdf-text-extractor-wireframe-v2.svg`. | FR-028, NFR-009 | No |
| 2026-05-04 | Reliability: `file_digest` (`sha256HexFile`); TC-NFR-003 hash equality after Poppler candidate generation, volume extract, and volume bootstrap (synthetic `hello.pdf` fixture). | NFR-003 | No |
| 2026-05-04 | NFR-001: `local_path_intent` rejects hosted `scheme://` paths for output roots, bootstrap corpus/source/work, and shell open; `file://` treated as local; TC-NFR-001 tests. | NFR-001 | No |

---

# 9. Coverage Validation Checklist

- 100% FR coverage to RTM: Yes
- 100% NFR coverage to RTM: Yes
- 100% FR coverage to design: Yes
- 100% NFR coverage to architecture: Yes
- 100% implementation traceability: No; implementation phase in progress
- 100% requirement-to-test coverage: Yes, draft test plan
- Deterministic-probabilistic boundaries mapped: Yes
- Packaging traceability complete: No; packaging phase not authorized
- Orchestration traceability complete: No; orchestration phase not authorized
- No orphan artifacts: No orphan requirements known in current phase

---

# 10. Test Planning Snapshot

Release is not in scope during Test Planning.

Current implementation state:

- Requirements approved: Yes
- HLA approved: Yes
- DD approved: Yes
- FR/NFR rows complete through DD: Yes
- HLA component coverage complete through DD: Yes
- Deterministic-probabilistic boundary coverage complete through DD: Yes
- Test mappings pending: No; approved Test Plan maps all FR/NFR IDs
- Implementation mappings pending: Yes; Implementation phase in progress
- Packaging/orchestration mappings pending: Yes; later phases not yet approved

---

# 11. Traceability Consolidation Approval

Approved By: Chuck
Role: Project owner
Date: 2026-05-02
Version Incremented: No

---

# 12. Test Planning Approval

Approved By: Chuck
Role: Project owner
Date: 2026-05-02
Version Incremented: No
