# pdf-text-extractor

Local PDF text extraction and work-folder tooling: headless **`pte_bootstrap`**, optional Qt reviewer **`pte_shell`**, optional in-process demo **`pte_demo_app`**, and enrichment prototype **`pte_enrich`**.

- **Version:** **0.1.0** (see `CMakeLists.txt` and [`CHANGELOG.md`](CHANGELOG.md)).
- **Documentation index:** [`docs/README.md`](docs/README.md).
- **AI / contributor contract:** [`AGENTS.md`](AGENTS.md).

Build (from repository root):

```bash
cmake -S . -B build
cmake --build build
```

Optional targets are CMake-gated; see [`docs/ui-shell.md`](docs/ui-shell.md) (Qt shell) and [`docs/integration-tutorial.md`](docs/integration-tutorial.md) (library + `pte_demo_app`).
