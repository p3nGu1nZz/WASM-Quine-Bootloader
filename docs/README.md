# WASM Quine Bootloader

<div align="center">
<img width="1200" height="475" alt="GHBanner" src="https://github.com/user-attachments/assets/0aa67016-6eaf-458a-adb2-6e31a0763ed6" />
</div>

A self-replicating, self-evolving WebAssembly kernel visualizer — ported from a
TypeScript/React web app to a **native C++17 desktop application** using
**SDL3** and **Dear ImGui**.

---

## Features

| Feature | Description |
|---|---|
| WASM Quine | A minimal WebAssembly binary that echoes its own source (base-64) back through a host import |
| Self-Evolution | The binary mutates each generation (insert / delete / modify / append) and validates itself |
| Memory Visualizer | Real-time SDL3 canvas heatmap of WASM heap activity |
| Instruction Stream | Step-by-step WASM opcode visualizer with program counter |
| Terminal Log | Color-coded system event log (info / success / warning / error / mutation) |
| Era System | Four visual themes that change as the kernel evolves (Primordial → Expansion → Complexity → Singularity) |
| Export | Write a full telemetry report (hex dump, disassembly, history log) to a text file |

---

## Project Layout

```
.
├── CMakeLists.txt        # CMake build definition
├── setup.sh              # One-shot dependency installer + builder (WSL Ubuntu)
├── src/                  # C++17 application source
│   ├── main.cpp          # SDL3 init, ImGui main loop, rendering
│   ├── app.h / app.cpp   # State machine, boot sequence, WASM lifecycle
│   ├── types.h           # SystemState, SystemEra, LogEntry, BootConfig
│   ├── constants.h       # KERNEL_GLOB (base64 WASM), DEFAULT_BOOT_CONFIG
│   ├── base64.h          # Inline base64 encode / decode
│   ├── wasm_parser.h/.cpp  # WASM binary parser (LEB-128, code section)
│   ├── wasm_evolution.h/.cpp # WASM mutation engine
│   └── wasm_kernel.h/.cpp  # WasmKernel – wasm3 integration
├── scripts/
│   ├── build.sh          # Configure & compile
│   └── run.sh            # Build if needed, then launch
├── docs/
│   └── README.md         # This file
└── external/             # Populated by setup.sh
    ├── imgui/            # Dear ImGui (cloned from GitHub)
    ├── wasm3/            # wasm3 interpreter (cloned from GitHub)
    └── SDL3/             # SDL3 (built from source if not system-installed)
```

---

## Quick Start (Windows WSL2 / Ubuntu)

### 1 — Run the setup script

```bash
bash setup.sh
```

This will:

1. Install system packages (`build-essential`, `cmake`, `ninja-build`, SDL3
   dependencies, fonts, …)
2. Clone **Dear ImGui** into `external/imgui`
3. Clone **wasm3** into `external/wasm3`
4. Install or build **SDL3** (tries `apt` first, then builds from source)
5. Configure and compile the project into `build/`

### 2 — Run the app

```bash
bash scripts/run.sh
# or directly:
./build/wasm_quine_bootloader
```

### 3 — Rebuild after source changes

```bash
bash scripts/build.sh
# or with debug symbols:
BUILD_TYPE=Debug bash scripts/build.sh
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
   source written into WASM memory.  Inside, the module calls
   `env.log(ptr, len)` — our host callback.

3. **Verify** — If the echoed bytes match the original source, the quine
   succeeds.  The generation counter increments.

4. **Evolve** — The binary is mutated (insert / delete / append / modify
   instructions in the code section) and the cycle repeats.

5. **Repair** — If verification fails, the engine reverts to the last stable
   kernel and applies an adaptive mutation.

---

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | 3.2.x | Window, renderer, events |
| [Dear ImGui](https://github.com/ocornut/imgui) | master | Immediate-mode GUI |
| [wasm3](https://github.com/wasm3/wasm3) | main | WebAssembly interpreter |

All three are fetched automatically by `setup.sh`.

---

## Building Manually (advanced)

```bash
# After running setup.sh at least once for the external libraries:
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/wasm_quine_bootloader
```

---

## License

See [LICENSE](../LICENSE) if present, otherwise all rights reserved by the
original author.
