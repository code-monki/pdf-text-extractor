# Optional embed of code-monki/PDFDocumentView for pte_shell preview (PDFium).
# Set PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT to the library checkout before configure.

if(NOT PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT)
    message(FATAL_ERROR
        "PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT is required when PDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON "
        "(path to PDFDocumentView checkout with CMakeLists.txt)")
endif()

if(NOT EXISTS "${PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "PDFDocumentView not found at ${PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT}")
endif()

# PDFDocumentView CMake reads ENV{QT_PREFIX} when set; propagate from parent configure.
if(DEFINED ENV{QT_PREFIX} AND NOT "$ENV{QT_PREFIX}" STREQUAL "")
    # keep caller-provided Qt kit path
elseif(CMAKE_PREFIX_PATH)
    list(GET CMAKE_PREFIX_PATH 0 _pte_pdfdocumentview_qt_prefix)
    set(ENV{QT_PREFIX} "${_pte_pdfdocumentview_qt_prefix}")
endif()

set(PDFDOCUMENTVIEW_BUILD_EXAMPLES OFF CACHE BOOL "PDFDocumentView examples (pte_shell embed)" FORCE)
set(PDFDOCUMENTVIEW_BUILD_DOCS OFF CACHE BOOL "PDFDocumentView Doxygen (pte_shell embed)" FORCE)
set(PDFDOCUMENTVIEW_INSTALL OFF CACHE BOOL "PDFDocumentView install rules (pte_shell embed)" FORCE)

add_subdirectory(
    "${PDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT}"
    "${CMAKE_BINARY_DIR}/_deps/pdfdocumentview"
)
