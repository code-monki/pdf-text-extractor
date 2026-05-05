# File: src/ui/CMakeLists.txt
# Purpose: Qt Widgets reviewer shell target (optional build).
# Architectural context: HLA-UI, DD 4.1, ADR-0004 (Widgets fallback).
# Requirement references: FR-006, FR-028, NFR-009.

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(pte_shell
    app_theme.cpp
    app_theme.hpp
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
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

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
