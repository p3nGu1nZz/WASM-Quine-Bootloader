# WASM Quine Bootloader

<div align="center">
<img width="1200" height="475" alt="GHBanner" src="https://github.com/user-attachments/assets/0aa67016-6eaf-458a-adb2-6e31a0763ed6" />
</div>

A self-replicating, self-evolving WebAssembly kernel visualizer — native C++17
desktop application using **SDL3** and **Dear ImGui**.

---

## Features

| Feature | Description |
|---|---|
| WASM Quine | A minimal WebAssembly binary that echoes its own source (base-64) back through a host import |
| Self-Evolution | The binary mutates each generation (insert / delete / modify / append) and validates itself |
| Memory Visualizer | Real-time SDL3 canvas heatmap of WASM heap activity |
| Instruction Stream | Step-by-step WASM opcode visualizer with program counter |
| Terminal Log | Color-coded system event log (info / success / warning / error / mutation) |
| Era System | Four visual themes as the kernel evolves (Primordial → Expansion → Complexity → Singularity) |
| Export | Write a full telemetry report (hex dump, disassembly, history) to a text file |

---

## Project Layout

```
.
├── CMakeLists.txt            # CMake build definition
├── web/                      # Original TypeScript/React web app (reference)
│   ├── App.tsx
│   ├── components/
│   ├── services/
│   ├── utils/
│   └── ...
├── src/                      # C++17 application source
│   ├── main.cpp              # SDL3 init and main loop
│   ├── gui.h / gui.cpp       # Dear ImGui rendering (all panels)
│   ├── util.h / util.cpp     # String/time utilities
│   ├── app.h / app.cpp       # State machine and boot sequence
│   ├── types.h               # SystemState, SystemEra, LogEntry, BootConfig
│   ├── constants.h           # KERNEL_GLOB (base64 WASM), DEFAULT_BOOT_CONFIG
│   ├── base64.h              # Inline base64 encode / decode
│   ├── wasm_parser.h/.cpp    # WASM binary parser (LEB-128, code section)
│   ├── wasm_evolution.h/.cpp # WASM mutation engine
│   └── wasm_kernel.h/.cpp    # WasmKernel – wasm3 integration
├── scripts/
│   ├── setup.sh              # One-shot dependency installer + builder
│   ├── build.sh              # Build for a specific target
│   ├── run.sh                # Build if needed, then launch
│   └── toolchain-mingw64.cmake  # MinGW-w64 CMake toolchain (Windows cross-compile)
├── docs/
│   └── README.md             # This file
└── external/                 # Populated by scripts/setup.sh (not committed)
    ├── imgui/                # Dear ImGui
    ├── wasm3/                # wasm3 interpreter
    ├── SDL3/                 # SDL3 (built for Linux)
    └── SDL3-windows/         # SDL3 (cross-compiled for Windows, optional)
```

> **Note:** The `external/` directory is **not committed** to the repository.
> It is created and populated by running `bash scripts/setup.sh`.

---

## Quick Start (Windows WSL2 / Ubuntu)

### 1 — Run the setup script

```bash
bash scripts/setup.sh
```

This will:
1. Install system packages (`build-essential`, `cmake`, `ninja-build`, SDL3 dependencies, fonts)
2. Clone **Dear ImGui** into `external/imgui`
3. Clone **wasm3** into `external/wasm3`
4. Install or build **SDL3** for Linux
5. Build the project (`linux-debug` by default)

### 2 — Run the app

```bash
bash scripts/run.sh
# or directly:
./build/linux-debug/bootloader
```

---

## Build Targets

| Target | Platform | Optimisation | Binary |
|---|---|---|---|
| `linux-debug`    | Linux | Debug   | `build/linux-debug/bootloader` |
| `linux-release`  | Linux | Release | `build/linux-release/bootloader` |
| `windows-debug`  | Windows (MinGW) | Debug   | `build/windows-debug/bootloader.exe` |
| `windows-release`| Windows (MinGW) | Release | `build/windows-release/bootloader.exe` |

```bash
bash scripts/build.sh                    # linux-debug (default)
bash scripts/build.sh linux-release
bash scripts/build.sh windows-debug     # needs: bash scripts/setup.sh windows
bash scripts/build.sh windows-release   # needs: bash scripts/setup.sh windows
```

### Windows cross-compile setup

```bash
bash scripts/setup.sh windows   # installs MinGW-w64 + builds SDL3 for Windows
bash scripts/build.sh windows-release
```

---

## Controls

| Key / Button | Action |
|---|---|
| `Space` | Pause / Resume the simulation |
| `Q` / `Esc` | Quit |
| **PAUSE SYSTEM** button | Same as Space |
| **EXPORT** button | Write telemetry report to `quine_telemetry_gen<N>.txt` |
| **COPY** button | Copy current kernel base64 to clipboard |

---

## How It Works

1. **Boot** — The app loads a minimal WASM binary (the "kernel") via the
   [wasm3](https://github.com/wasm3/wasm3) interpreter.
2. **Execute** — `run(ptr, len)` is called with the kernel's own base-64
   source in WASM memory. Inside, the module calls `env.log(ptr, len)`.
3. **Verify** — If the echoed bytes match the original source, the quine
   succeeds and the generation counter increments.
4. **Evolve** — The binary is mutated (insert / delete / append / modify
   instructions in the code section) and the cycle repeats.
5. **Repair** — On verification failure the engine reverts to the last stable
   kernel and applies an adaptive mutation.

---

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | 3.2.x | Window, renderer, events |
| [Dear ImGui](https://github.com/ocornut/imgui) | master | Immediate-mode GUI |
| [wasm3](https://github.com/wasm3/wasm3) | main | WebAssembly interpreter |

All three are fetched automatically by `scripts/setup.sh`.

---

## Original TypeScript Web App

The original TypeScript/React web application is preserved in `web/` for
reference. To run it:

```bash
cd web
npm install
npm run dev
```

---

## License

See [LICENSE](../LICENSE) if present, otherwise all rights reserved.
