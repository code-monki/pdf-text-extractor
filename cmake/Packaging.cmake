# Packaging and CPack (Gate 8 — NFR-014).
# Produces portable archives (TGZ/ZIP) of installed binaries and libraries.
# See docs/packaging-plan.md.

include(GNUInstallDirs)

install(TARGETS pdf_text_extractor_core
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS pte_bootstrap
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS pte_enrich
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS
    ${CMAKE_SOURCE_DIR}/scripts/pdf_enrich_apply.py
    ${CMAKE_SOURCE_DIR}/scripts/enrich_lint_manifest.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL)
    install(TARGETS pte_shell
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

set(CPACK_PACKAGE_NAME "pdf-text-extractor")
set(CPACK_PACKAGE_VENDOR "pdf-text-extractor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Local PDF text extraction and review tools")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_GENERATOR "TGZ;ZIP")
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

# Collect artifacts under ./dist (gitignored) at project root for predictable handoff paths.
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")

include(CPack)
