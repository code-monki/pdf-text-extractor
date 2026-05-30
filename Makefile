# File: Makefile
# Purpose: Top-level orchestration entry point for pdf-text-extractor.
# Architectural context: HLA-WORK, HLA-VALID, HLA-PKG, HLA-SAFE.
# Requirement references: FR-017, FR-018, FR-025, FR-031, NFR-011, NFR-012, NFR-014.
# Test references: TC-FR-017, TC-FR-018, TC-FR-025, TC-FR-031, TC-NFR-011, TC-NFR-012, TC-NFR-014.

SHELL := /bin/sh

CMAKE ?= cmake
CTEST ?= ctest
CXX ?= c++

BUILD_DIR ?= build
# Optional Qt 6 kit path for pte_shell (separate from default headless build).
SHELL_BUILD_DIR ?= build-qt-shell
QT_PREFIX ?=
PDFDOCUMENTVIEW_ROOT ?=
BUILD_TYPE ?= Debug
CTEST_OUTPUT_ON_FAILURE ?= 1
ENRICH_MANIFEST ?= tests/fixtures/enrichment/lint.manifest.tsv

# `make all` builds the default tree always; when QT_PREFIX is set it also builds pte_shell.
ALL_DEFAULT_DEPS := build
ifneq ($(strip $(QT_PREFIX)),)
ALL_DEFAULT_DEPS += shell
endif

.DEFAULT_GOAL := help

.PHONY: help
help:
	@printf '%s\n' 'pdf-text-extractor orchestration targets:'
	@printf '%s\n' ''
	@printf '  %-14s %s\n' 'help' 'Show this target list.'
	@printf '  %-14s %s\n' 'tool-check' 'Report required local build tools.'
	@printf '  %-14s %s\n' 'all' 'Default tree ($(BUILD_DIR): library, tests, pte_bootstrap); pte_shell too if QT_PREFIX is set.'
	@printf '  %-14s %s\n' 'setup' 'Prepare the local build tree.'
	@printf '  %-14s %s\n' 'configure' 'Configure CMake into $(BUILD_DIR).'
	@printf '  %-14s %s\n' 'build' 'Build all configured targets (library, tests, pte_bootstrap CLI).'
	@printf '  %-14s %s\n' 'shell-configure' 'Configure CMake into $(SHELL_BUILD_DIR) with pte_shell enabled (needs QT_PREFIX).'
	@printf '  %-14s %s\n' 'shell' 'Configure and build pte_shell only (see docs/ui-shell.md).'
	@printf '  %-14s %s\n' 'shell-run' 'Launch pte_shell from $(SHELL_BUILD_DIR) (build with make shell first).'
	@printf '  %-14s %s\n' 'shell-go' 'Build pte_shell (needs QT_PREFIX) and launch it.'
	@printf '  %-14s %s\n' 'test' 'Run the CTest suite with named test output.'
	@printf '  %-14s %s\n' 'check' 'Run tool-check, configure, build, and test.'
	@printf '  %-14s %s\n' 'enrich-lint' 'Run pte_enrich --lint-only for one source/map set.'
	@printf '  %-14s %s\n' 'enrich-lint-fixtures' 'Run scripts/enrich_lint_manifest.sh on tests/fixtures/enrichment/lint.manifest.tsv.'
	@printf '  %-14s %s\n' 'clean' 'Remove $(BUILD_DIR) and $(SHELL_BUILD_DIR) (full CMake trees).'
	@printf '  %-14s %s\n' 'distclean' 'clean plus remove dist/package artifact dirs (see target).'
	@printf '  %-14s %s\n' 'clean-build' 'Run distclean, configure, build, and test.'
	@printf '  %-14s %s\n' 'package' 'Run CPack (TGZ/ZIP) into dist/ — see docs/packaging-plan.md.'
	@printf '%s\n' ''
	@printf '%s\n' 'Variables:'
	@printf '  %-14s %s\n' 'BUILD_DIR' 'Build directory, default: build'
	@printf '  %-14s %s\n' 'BUILD_TYPE' 'CMake build type, default: Debug'
	@printf '  %-14s %s\n' 'CMAKE' 'CMake executable, default: cmake'
	@printf '  %-14s %s\n' 'CTEST' 'CTest executable, default: ctest'
	@printf '  %-14s %s\n' 'CXX' 'C++ compiler, default: c++'
	@printf '  %-14s %s\n' 'SHELL_BUILD_DIR' 'Qt shell CMake tree, default: build-qt-shell'
	@printf '  %-14s %s\n' 'QT_PREFIX' 'Qt 6 CMAKE_PREFIX_PATH (required for shell targets)'
	@printf '  %-14s %s\n' 'PDFDOCUMENTVIEW_ROOT' 'PDFDocumentView checkout (optional; default ../PDFDocumentView)'
	@printf '  %-14s %s\n' 'ENRICH_SOURCE' 'Source PDF for make enrich-lint (required).'
	@printf '  %-14s %s\n' 'ENRICH_OUTLINE_MAP' 'Outline map path for make enrich-lint (optional).'
	@printf '  %-14s %s\n' 'ENRICH_LINK_MAP' 'Link map path for make enrich-lint (optional).'
	@printf '  %-14s %s\n' 'ENRICH_PYTHON' 'Python executable override for make enrich-lint (optional).'
	@printf '  %-14s %s\n' 'ENRICH_SCRIPT' 'Script override for make enrich-lint (optional).'
	@printf '  %-14s %s\n' 'ENRICH_MANIFEST' 'TSV path for make enrich-lint-fixtures (default: tests/fixtures/enrichment/lint.manifest.tsv).'

.PHONY: tool-check
tool-check:
	@command -v "$(CMAKE)" >/dev/null 2>&1 || { printf '%s\n' 'Missing required tool: cmake'; exit 1; }
	@command -v "$(CTEST)" >/dev/null 2>&1 || { printf '%s\n' 'Missing required tool: ctest'; exit 1; }
	@command -v "$(CXX)" >/dev/null 2>&1 || { \
		printf '%s\n' 'Missing required tool: C++ compiler ($(CXX))'; \
		exit 1; \
	}
	@"$(CMAKE)" --version | sed -n '1p'
	@"$(CTEST)" --version | sed -n '1p'
	@"$(CXX)" --version | sed -n '1p'

.PHONY: all
all: $(ALL_DEFAULT_DEPS)

.PHONY: setup
setup: configure

.PHONY: configure
configure: tool-check
	@"$(CMAKE)" -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)"

.PHONY: build
build: configure
	@"$(CMAKE)" --build "$(BUILD_DIR)"

.PHONY: shell-configure
shell-configure: tool-check
	@test -n "$(QT_PREFIX)" || { \
		printf '%s\n' 'QT_PREFIX is required (path to Qt 6 kit, e.g. export QT_PREFIX=$$HOME/Qt/6.9.3/macos). See docs/ui-shell.md.'; \
		exit 1; \
	}
	@_pdfdocview_root="$(PDFDOCUMENTVIEW_ROOT)"; \
	if [ -z "$$_pdfdocview_root" ] && [ -f "../PDFDocumentView/CMakeLists.txt" ]; then \
		_pdfdocview_root="$$(cd .. && pwd)/PDFDocumentView"; \
	fi; \
	if [ -z "$$_pdfdocview_root" ] || [ ! -f "$$_pdfdocview_root/CMakeLists.txt" ]; then \
		printf '%s\n' 'PDFDocumentView checkout required (set PDFDOCUMENTVIEW_ROOT or clone sibling ../PDFDocumentView).'; \
		exit 1; \
	fi; \
	QT_PREFIX="$(QT_PREFIX)" "$(CMAKE)" -S . -B "$(SHELL_BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-DPDF_TEXT_EXTRACTOR_BUILD_QT_SHELL=ON \
		-DPDF_TEXT_EXTRACTOR_PDFDOCUMENTVIEW_ROOT="$$_pdfdocview_root" \
		-DCMAKE_PREFIX_PATH="$(QT_PREFIX)"

.PHONY: shell
shell: shell-configure
	@"$(CMAKE)" --build "$(SHELL_BUILD_DIR)" --target pte_shell
	@printf '%s\n' "Built pte_shell under $(SHELL_BUILD_DIR)."
	@printf '%s\n' "  Launch:    make shell-run"
	@printf '%s\n' "  Or macOS:  open $(SHELL_BUILD_DIR)/src/ui/pte_shell.app"

.PHONY: shell-run
shell-run:
	@case "$$(uname -s)" in \
	Darwin) \
	  _app="$(SHELL_BUILD_DIR)/src/ui/pte_shell.app"; \
	  if [ ! -d "$$_app" ]; then _app="$(SHELL_BUILD_DIR)/src/ui/$(BUILD_TYPE)/pte_shell.app"; fi; \
	  if [ ! -d "$$_app" ]; then \
	    printf '%s\n' 'pte_shell.app not found under $(SHELL_BUILD_DIR)/src/ui/. Build first: make shell QT_PREFIX=…'; \
	    exit 1; \
	  fi; \
	  printf '%s\n' "Launching pte_shell: $$(cd "$(CURDIR)" && pwd)/$$_app"; \
	  open "$$_app" ;; \
	Linux) \
	  _bin="$(SHELL_BUILD_DIR)/src/ui/pte_shell"; \
	  if [ ! -x "$$_bin" ]; then _bin="$(SHELL_BUILD_DIR)/src/ui/$(BUILD_TYPE)/pte_shell"; fi; \
	  if [ ! -x "$$_bin" ]; then \
	    printf '%s\n' 'pte_shell not found under $(SHELL_BUILD_DIR)/src/ui/. Build first: make shell QT_PREFIX=…'; \
	    exit 1; \
	  fi; \
	  "$$_bin" ;; \
	MINGW*|MSYS*|CYGWIN*) \
	  _exe="$(SHELL_BUILD_DIR)/src/ui/pte_shell.exe"; \
	  if [ ! -f "$$_exe" ]; then _exe="$(SHELL_BUILD_DIR)/src/ui/$(BUILD_TYPE)/pte_shell.exe"; fi; \
	  if [ ! -f "$$_exe" ]; then \
	    printf '%s\n' 'pte_shell.exe not found. Build first: make shell QT_PREFIX=…'; \
	    exit 1; \
	  fi; \
	  "$$_exe" ;; \
	*) \
	  printf '%s\n' 'shell-run is not mapped for this OS; run pte_shell from $(SHELL_BUILD_DIR)/src/ui/.'; \
	  exit 1 ;; \
	esac

.PHONY: shell-go
shell-go: shell shell-run

.PHONY: test
test: build
	@"$(CTEST)" --test-dir "$(BUILD_DIR)" --output-on-failure --verbose

.PHONY: check
check: tool-check configure build test

.PHONY: enrich-lint
enrich-lint: build
	@test -n "$(ENRICH_SOURCE)" || { \
		printf '%s\n' 'ENRICH_SOURCE is required. Example:'; \
		printf '%s\n' '  make enrich-lint ENRICH_SOURCE=/path/volume.pdf ENRICH_OUTLINE_MAP=/path/outline-map.json ENRICH_LINK_MAP=/path/link-map.json'; \
		exit 1; \
	}
	@test -n "$(ENRICH_OUTLINE_MAP)$(ENRICH_LINK_MAP)" || { \
		printf '%s\n' 'At least one map is required: ENRICH_OUTLINE_MAP and/or ENRICH_LINK_MAP.'; \
		exit 1; \
	}
	@_cmd='"$(BUILD_DIR)/pte_enrich" --source "$(ENRICH_SOURCE)" --lint-only'; \
	if [ -n "$(ENRICH_OUTLINE_MAP)" ]; then _cmd="$$_cmd --outline-map \"$(ENRICH_OUTLINE_MAP)\""; fi; \
	if [ -n "$(ENRICH_LINK_MAP)" ]; then _cmd="$$_cmd --link-map \"$(ENRICH_LINK_MAP)\""; fi; \
	if [ -n "$(ENRICH_PYTHON)" ]; then _cmd="$$_cmd --python \"$(ENRICH_PYTHON)\""; fi; \
	if [ -n "$(ENRICH_SCRIPT)" ]; then _cmd="$$_cmd --script \"$(ENRICH_SCRIPT)\""; fi; \
	eval "$$_cmd"

.PHONY: enrich-lint-fixtures
enrich-lint-fixtures: build
	@case "$(ENRICH_MANIFEST)" in \
	/*) _manifest="$(ENRICH_MANIFEST)";; \
	*) _manifest="$(CURDIR)/$(ENRICH_MANIFEST)";; \
	esac; \
	PTE_ENRICH="$(CURDIR)/$(BUILD_DIR)/pte_enrich" "$(CURDIR)/scripts/enrich_lint_manifest.sh" "$$_manifest"

.PHONY: enrich-lint-manifest
enrich-lint-manifest: enrich-lint-fixtures

.PHONY: clean
clean:
	@rm -rf "$(BUILD_DIR)" "$(SHELL_BUILD_DIR)"
	@printf '%s\n' "Removed $(BUILD_DIR) and $(SHELL_BUILD_DIR)."

.PHONY: distclean
distclean: clean
	@rm -rf dist artifacts package packages out output outputs

.PHONY: clean-build
clean-build: distclean configure build test

.PHONY: package
package: build
	@"$(CMAKE)" --build "$(BUILD_DIR)" --target package
	@printf '%s\n' 'Packaging complete. Archives under dist/ (gitignored). See docs/packaging-plan.md.'
