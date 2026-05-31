# File: src/ui/CMakeLists.txt
# Purpose: Qt Widgets reviewer shell target (optional build).
# Architectural context: HLA-UI, DD 4.1, ADR-0004 (Widgets fallback).
# Requirement references: FR-006, FR-028, NFR-009.

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(pte_shell
    app_theme.cpp
    app_theme.hpp
    link_map_editor_dialog.cpp
    link_map_editor_dialog.hpp
    main.cpp
    shell_main_window.cpp
    shell_main_window.hpp
    review_session_facade.cpp
    review_session_facade.hpp
    volume_metadata_dialog.cpp
    volume_metadata_dialog.hpp
)

target_link_libraries(pte_shell PRIVATE
    pdf_text_extractor_core
    PDFDocumentView::pdf_document_view
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_compile_definitions(pte_shell PRIVATE
    "PTE_REPO_ROOT=\"${CMAKE_SOURCE_DIR}\""
    "PTE_APP_VERSION=\"${PROJECT_VERSION}\"")

if(MSVC)
    target_compile_options(pte_shell PRIVATE /W4)
else()
    target_compile_options(pte_shell PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    set_target_properties(pte_shell PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
if(APPLE)
    set_target_properties(pte_shell PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if(TARGET pdf_document_view)
    add_dependencies(pte_shell pdf_document_view)
    if(APPLE OR UNIX)
        set_target_properties(pte_shell PROPERTIES
            BUILD_RPATH "$<TARGET_FILE_DIR:pdf_document_view>")
    endif()
endif()
