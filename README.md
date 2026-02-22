# WASM Quine Bootloader

A self-replicating, self-evolving WebAssembly kernel visualizer — native **C++17** desktop application using **SDL3** and **Dear ImGui**.

For technical design details see **[docs/design.md](docs/design.md)** and **[docs/architecture.md](docs/architecture.md)**.

Specification documents live under `docs/specs/` (e.g. CLI, telemetry formats).

---

## Features

| Feature | Description |
|---|---|
| WASM Quine | A minimal WebAssembly binary that echoes its own source back through a host import |
| Self-Evolution | Binary mutates each generation (insert / delete / modify / append) and validates itself |
| Memory Visualizer | Real-time SDL3 canvas heatmap of WASM heap activity |
| Instruction Stream | Step-by-step WASM opcode visualizer with program counter |
| Terminal Log | Colour-coded system event log (info / success / warning / error / mutation) |
| Era System | *removed; terminal-only app no longer uses eras* |
| Telemetry Export | Dump full hex / disassembly / history report to a `.txt` file |
| DPI Scaling & Touch UI | UI text and widgets automatically scale with window size for high‑DPI and touch‑friendly use; big buttons, snappy interaction |

---

## Agent Skills

This repo includes a full suite of Copilot agent skills; see `.github/copilot-instructions.md` for a current list.  Key skills include:

- `setup-project`, `build-project`, `run-app`, `test-app`
- `update-docs`, `update-specs`, `update-skills`, `update-memory`
- `search-memory`, `telemetry-review`, `introspect-telemetry`
- `improve-skills`, `generate-report`, `commit-push`

Agents can invoke them via natural language prompts; the skills and
instructions are maintained by the `update-skills`/`improve-skills` tools.

---

## Project Layout

```
.
├── CMakeLists.txt            # CMake build definition (C++17, Ninja)
├── src/                      # C++17 application source
│   ├── main.cpp              # SDL3 init + main loop
│   ├── gui/window.h / gui/window.cpp # Gui class: ImGui backend lifecycle, panel orchestration
│   ├── gui/colors.h          # Header-only colour helpers (state/log → ImVec4)
│   ├── gui/heatmap.h/.cpp    # GuiHeatmap: memory heat-decay visualizer
│   ├── util.h / util.cpp     # stateStr, randomId, nowIso
│   ├── app.h / app.cpp       # App: top-level orchestrator
│   ├── fsm.h / fsm.cpp       # BootFsm: finite state machine
│   ├── log.h / log.cpp # AppLogger: live log ring-buffer + history ledger
│   ├── exporter.h / exporter.cpp # buildReport(): telemetry text report
│   ├── types.h               # SystemState, LogEntry, HistoryEntry, BootConfig
│   ├── constants.h           # KERNEL_GLOB (base64 WASM), DEFAULT_BOOT_CONFIG
│   ├── base64.h              # Inline base64 encode / decode
│   ├── wasm/                  # subdirectory for WASM-related modules
│   │   ├── parser.h/.cpp    # WASM binary parser (LEB-128, code section)
│   │   ├── evolution.h/.cpp # WASM mutation engine
│   │   └── kernel.h/.cpp    # WasmKernel – wasm3 integration
├── scripts/
│   ├── setup.sh              # One-shot dependency installer + initial build
│   ├── build.sh              # Build for a specific target (or --clean)
│   └── run.sh                # Build if needed, then launch
├── cmake/
│   └── toolchain-windows-x64.cmake  # MinGW-w64 CMake toolchain (Windows cross-compile)
├── docs/
│   ├── design.md             # Design goals, simulation loop, mutation strategy
│   └── architecture.md       # Per-file module specs and dependency graph
└── external/                 # Populated by scripts/setup.sh (NOT committed)
    ├── imgui/
    ├── wasm3/
    └── SDL3/
        ├── src/              # SDL3 source tree
        ├── linux/            # Built + installed for Linux
        └── windows/          # Built + installed for Windows (optional)
```
