# File: CMakeLists.txt
# Purpose: Build the initial domain/test implementation for pdf-text-extractor.
# Architectural context: HLA-INV, HLA-WORK, HLA-META, HLA-VALID, HLA-REVIEW, HLA-DEP,
# HLA-TEXT, HLA-EXT, HLA-OCR, HLA-DIAG, HLA-APP, HLA-CONFIG, HLA-SAFE.
# Requirement references: FR-001, FR-002, FR-003, FR-004, FR-005, FR-006, FR-007,
# FR-008, FR-009, FR-010, FR-011,
# FR-012, FR-013, FR-014, FR-015, FR-016, FR-017, FR-018, FR-022, FR-024, FR-025,
# FR-021, FR-026, FR-027, FR-029, FR-030, FR-031, FR-032, NFR-001, NFR-002,
# NFR-003, NFR-004, NFR-005, NFR-007, NFR-008, NFR-010, NFR-011,
# NFR-012, NFR-014.
# Test references: TC-FR-001, TC-FR-002, TC-FR-003, TC-FR-004, TC-FR-005,
# TC-FR-006, TC-FR-007, TC-FR-008, TC-FR-009, TC-FR-010, TC-FR-011,
# TC-FR-012, TC-FR-013, TC-FR-014, TC-FR-015, TC-FR-016,
# TC-FR-017, TC-FR-018, TC-FR-021, TC-FR-022, TC-FR-024, TC-FR-025, TC-FR-026,
# TC-FR-027, TC-FR-029, TC-FR-030, TC-FR-031, TC-NFR-001, TC-NFR-002,
# TC-NFR-003, TC-NFR-004, TC-NFR-005, TC-NFR-008, TC-NFR-010, TC-NFR-011,
# TC-FR-032, TC-NFR-007, TC-NFR-012, TC-NFR-014.
# Packaging: cmake/Packaging.cmake (Gate 8); docs/packaging-plan.md.
# Constraint: Default configure avoids Qt/UI unless PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON.

cmake_minimum_required(VERSION 3.25)

project(pdf_text_extractor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(pdf_text_extractor_core
    src/core/artifact_cleanup.cpp
    src/core/candidate_generation_poppler.cpp
    src/core/candidate_text.cpp
    src/core/dependency_capability.cpp
    src/core/file_change_detector.cpp
    src/core/file_digest.cpp
    src/core/json.cpp
    src/core/local_path_intent.cpp
    src/core/ocr_routing.cpp
    src/core/output_config.cpp
    src/core/page_id.cpp
    src/core/page_quality.cpp
    src/core/pdf_enrichment.cpp
    src/core/page_review_sync.cpp
    src/core/pdf_inspection_poppler.cpp
    src/core/poppler_embedded_text.cpp
    src/core/poppler_page_render.cpp
    src/core/process_runner.cpp
    src/core/readiness_summary.cpp
    src/core/review_state.cpp
    src/core/review_state_update.cpp
    src/core/review_workflow.cpp
    src/core/reviewed_page_text.cpp
    src/core/source_inventory.cpp
    src/core/tesseract_ocr.cpp
    src/core/text_normalization.cpp
    src/core/validation_repair.cpp
    src/core/volume_bootstrap.cpp
    src/core/volume_metadata.cpp
    src/core/volume_extraction_pipeline.cpp
    src/core/work_folder_backup.cpp
    src/core/work_folder_initializer.cpp
    src/core/work_folder_validator.cpp
)

target_include_directories(pdf_text_extractor_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(pdf_text_extractor_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(pdf_text_extractor_core PRIVATE /W4)
else()
    target_compile_options(pdf_text_extractor_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(core_tests
    tests/core_tests.cpp
)

target_link_libraries(core_tests PRIVATE pdf_text_extractor_core)

target_compile_definitions(core_tests PRIVATE
    "PTE_TEST_FIXTURE_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures\"")

add_test(NAME core_tests COMMAND core_tests)

add_executable(pte_bootstrap
    src/cli/bootstrap_main.cpp
)

target_link_libraries(pte_bootstrap PRIVATE pdf_text_extractor_core)

target_compile_features(pte_bootstrap PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(pte_bootstrap PRIVATE /W4)
else()
    target_compile_options(pte_bootstrap PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(pte_enrich
    src/cli/enrich_main.cpp
)

target_link_libraries(pte_enrich PRIVATE pdf_text_extractor_core)
target_compile_features(pte_enrich PRIVATE cxx_std_20)
target_compile_definitions(pte_enrich PRIVATE
    "PTE_REPO_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

if(MSVC)
    target_compile_options(pte_enrich PRIVATE /W4)
else()
    target_compile_options(pte_enrich PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL "Build Qt Widgets reviewer shell (pte_shell); requires Qt 6.5+" OFF)
if(PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL)
    add_subdirectory(src/ui)

    add_executable(tc_fr028_probe
        tests/tc_fr028_probe.cpp
    )
    target_compile_features(tc_fr028_probe PRIVATE cxx_std_20)
    add_dependencies(tc_fr028_probe pte_shell)
    add_test(
        NAME TC-FR-028
        COMMAND tc_fr028_probe "$<TARGET_FILE:pte_shell>"
    )
endif()

# Gate 8 — install rules + CPack (docs/packaging-plan.md).
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake")
