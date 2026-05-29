# File: examples/demo_app/CMakeLists.txt
# Purpose: Build the integration demo (pte_demo_app) that links the public core static library.
# Architectural context: HLA-APP consumer (uses documented facades only).
# Requirement references (illustrative): FR-001, FR-004, FR-024, FR-025, FR-027, FR-029.
# Gate context: Gate 9 documentation closure (operator/integrator reference artifact).

add_executable(pte_demo_app
    main.cpp
)

target_link_libraries(pte_demo_app PRIVATE pdf_text_extractor_core)
target_compile_features(pte_demo_app PRIVATE cxx_std_20)

# Pass the synthetic in-tree fixture path so the demo can exercise the
# integration flow without requiring the operator to supply a PDF first.
# Out-of-tree consumers do not get this define; they must pass --source.
target_compile_definitions(pte_demo_app PRIVATE
    "PTE_DEMO_DEFAULT_FIXTURE=\"${CMAKE_SOURCE_DIR}/tests/fixtures/hello.pdf\"")

if(MSVC)
    target_compile_options(pte_demo_app PRIVATE /W4)
else()
    target_compile_options(pte_demo_app PRIVATE -Wall -Wextra -Wpedantic)
endif()
