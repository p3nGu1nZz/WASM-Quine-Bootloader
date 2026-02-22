# WASM Quine Bootloader

A self-replicating, self-evolving WebAssembly kernel visualizer — native **C++17** desktop application using **SDL3** and **Dear ImGui**.

For technical design details see **[docs/design.md](docs/design.md)** and **[docs/architecture.md](docs/architecture.md)**.

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

> **Note:** `external/` is excluded from the repository (`.gitignore: /external/*`).
> Run `bash scripts/setup.sh` to populate it.

---

## Quick Start (Windows WSL2 / Ubuntu)

### Step 1 — Install dependencies and build

```bash
bash scripts/setup.sh
```

By default the build scripts will pick up whatever C and C++ compilers are first on your `PATH` (for example `/usr/bin/gcc`/`/usr/bin/g++`).
If you are running under WSL and have a non‑default compiler installed (clang, a different gcc version, etc.), you can force the project to use it by exporting the standard environment variables **before** invoking `build.sh`:

```bash
# use g++-12 instead of the system default
export CC=/usr/bin/gcc-12
export CXX=/usr/bin/g++-12
bash scripts/build.sh
```

The `build.sh` helper now passes those overrides through to `cmake`; you can also pass them manually via `-DCMAKE_C_COMPILER`/`-DCMAKE_CXX_COMPILER` if you prefer.

This script will:
1. Install system packages (`build-essential`, `cmake`, `ninja-build`, SDL3 system deps, fonts)
2. Clone **Dear ImGui** → `external/imgui`
3. Clone **wasm3** → `external/wasm3`
4. Install or build **SDL3** for Linux → `external/SDL3/linux`
5. Build the `linux-debug` target → `build/linux-debug/bootloader`

### Step 2 — Run

```bash
bash scripts/run.sh           # headless mode
bash scripts/run.sh --gui      # SDL3 GUI
bash scripts/run.sh --monitor  # launch and tail runtime log
```

By default the executable is started with its working directory set to the
build target (e.g. `build/linux-debug`).  Runtime log files are written to
`bin/logs/` inside that directory, and automatic telemetry exports live under
`bin/seq/<runid>/` (generation reports, kernel blobs, etc.).

The build script prints colored, prefixed `[build]` messages (green for info,
yellow for warnings, red for errors) on top of the usual cmake/ninja output to
make important steps easy to scan.

The `--monitor` option runs the bootloader in the background and tails
`bin/logs/*.log` in real time so you can watch system messages as evolution
proceeds.

### Cleanup

`bash scripts/build.sh --clean` removes the entire `build/` directory along
with CMake/Ninja caches.  It also deletes any generated `bin/` hierarchy at the
repo root, wiping old logs, sequence exports or temporary files produced by
runs or tests.  This leaves the source tree pristine.

---

## Build Targets

| Target | Platform | Build type | Output binary |
|---|---|---|---|
| `linux-debug`    | Linux | Debug   | `build/linux-debug/bootloader` |
| `linux-release`  | Linux | Release | `build/linux-release/bootloader` |
| `windows-debug`  | Windows (MinGW) | Debug   | `build/windows-debug/bootloader.exe` |
| `windows-release`| Windows (MinGW) | Release | `build/windows-release/bootloader.exe` |

```bash
bash scripts/build.sh                    # linux-debug (default)
bash scripts/build.sh linux-release
bash scripts/build.sh windows-debug     # requires: bash scripts/setup.sh windows
bash scripts/build.sh windows-release   # requires: bash scripts/setup.sh windows
bash scripts/build.sh --clean           # remove build/ dir + caches
bash scripts/build.sh --clean linux-debug   # clean then build linux-debug
```
### Windows cross-compile setup

```bash
bash scripts/setup.sh windows   # installs MinGW-w64 + builds SDL3 for Windows
bash scripts/build.sh windows-release
```

---

## Running the Bootloader

```bash
# Default (linux-debug)
bash scripts/run.sh

# Specific target
bash scripts/run.sh linux-release

# Direct (after build)
./build/linux-debug/bootloader
```

### Runtime keyboard controls

| Key / Button | Action |
|---|---|
| `Space` | Pause / Resume the simulation |
| `Q` / `Esc` | Quit |
| **PAUSE SYSTEM** button | Same as Space |
| **RESUME SYSTEM** button | Same as Space (shown when paused) |
| **EXPORT** button | Write telemetry report → `quine_telemetry_gen<N>.txt` |
| **COPY** button | Copy current kernel base64 to clipboard |

### HUD panels

| Panel | Description |
|---|---|
| **Top bar** | GEN · STATE · UPTIME · RETRIES – control buttons |
| **System Log** | Colour-coded ring-buffer of up to 1 000 events; auto-scrolls |
| **Instruction Stack** | WASM opcode list with live program-counter highlight |
| **Kernel Source** | Base64 diff view: header (blue), mutation (yellow), expansion (green) |
| **Memory Map** | Heat-decay block visualizer of WASM linear memory activity |
| **Status Bar** | RUNNING/PAUSED indicator |

---

## Simulation States (FSM)

```
IDLE ──▶ BOOTING ──▶ LOADING_KERNEL ──▶ EXECUTING
                                              │          │
                                       VERIFYING_QUINE  │
                                              │          ▼
                                           (reboot)  REPAIRING
                                              │          │
                                              └────┬─────┘
                                                   ▼
                                                 IDLE
```

| State | Colour | Description |
|---|---|---|
| IDLE | — | Waiting; immediately transitions to BOOTING |
| BOOTING | 🟡 yellow | Brief delay; scales with generation |
| LOADING_KERNEL | 🔵 blue | Byte-by-byte kernel loading animation |
| EXECUTING | 🟢 green | WASM kernel running; instruction-step visualizer active |
| VERIFYING_QUINE | 🟣 purple | Quine check passed; brief hold before reboot |
| REPAIRING | 🟠 orange | Quine check failed; adaptive mutation + retry |
| SYSTEM_HALT | 🔴 red | Unrecoverable error |

---

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | 3.2.x | Window, renderer, events |
| [Dear ImGui](https://github.com/ocornut/imgui) | master | Immediate-mode GUI |
| [wasm3](https://github.com/wasm3/wasm3) | main | WebAssembly interpreter |

All three are fetched by `scripts/setup.sh`.

---

## Original TypeScript Web App

## Original TypeScript Web App (archived)

The original TypeScript/React reference implementation was previously stored in `web/` but has been removed from this repository after a successful native C++ port. If you need the original prototype, consult the repository history or contact the maintainers.
