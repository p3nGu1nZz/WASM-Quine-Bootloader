# WASM Quine Bootloader – Copilot Instructions

## Project Overview

**WASM Quine Bootloader** is a self-replicating WebAssembly (WASM) bootloader implemented in **C++ with SDL3 and wasm3**. A minimal WASM binary (the "kernel") is instantiated by the wasm3 interpreter, executes itself, outputs its own source (Base64-encoded binary), verifies the output matches its own source, then *evolves* the binary before rebooting into the next generation — a quine that runs, mutates, and visualises itself.

## Technology Stack

- **Language**: C++17
- **Build System**: CMake + Ninja
- **GUI Rendering**: SDL3 (window mode; toggled to fullscreen with F11)
- **WASM Runtime**: wasm3 (embedded interpreter)
- **Terminal Rendering**: ANSI escape codes (default headless/server mode, refreshes like `top`)
- **Unit Tests**: custom test runner in `test/`
- **No web framework, no Node.js** – all code is native C++

## Repository Structure

```
/
├── CMakeLists.txt              # Top-level CMake build definition
├── src/
│   ├── main.cpp                # Entry point – parses --gui flag, starts terminal or GUI loop
│   ├── gui.h / gui.cpp         # Gui class – SDL3 window, panel rendering, F11 fullscreen toggle
│   ├── util.h / util.cpp       # String and general utilities
│   └── wasm_kernel.cpp         # wasm3 host – boots kernel, wires env.log / env.grow_memory
├── test/                       # Unit tests (one .cpp file per module under test)
├── external/
│   ├── SDL3/                   # SDL3 source (built from source via setup.sh)
│   └── wasm3/                  # wasm3 embedded WASM interpreter
├── scripts/
│   ├── build.sh                # Build wrapper (target: linux-debug, linux-release, …)
│   ├── run.sh                  # Build (if needed) then run the bootloader
│   ├── test.sh                 # Build app + tests then run all tests
│   └── setup.sh                # One-time install of system deps (SDL3 from source, etc.)
└── web/                        # Legacy TypeScript/React prototype (not built by CMake)
```

## Build Targets

| Target | Description |
|--------|-------------|
| `linux-debug` | Debug build for Linux (default) |
| `linux-release` | Optimised release build for Linux |
| `windows-debug` | Cross-compiled debug build via MinGW-w64 |
| `windows-release` | Cross-compiled release build via MinGW-w64 |

Binary output: `build/<TARGET>/bootloader` (Linux) or `build/<TARGET>/bootloader.exe` (Windows).

## Running Modes

### Terminal / Headless Server Mode (default)
Running `./bootloader` (or via `scripts/run.sh`) without `--gui` renders all output to the terminal using ANSI escape sequences. The display refreshes in-place (like the `top` command) showing system panels, log output, memory state, and generation counters.

### GUI Mode
Running `./bootloader --gui` opens an SDL3 window:
- Background is black by default, windowed mode on launch.
- **F11** toggles between windowed and fullscreen. A second press returns to windowed.
- All panels rendered to the SDL3 surface mirror the terminal mode layout.

## Coding Conventions

- **C++17** – use `std::string_view`, structured bindings, `[[nodiscard]]`, etc. where appropriate.
- **wasm3 host functions** must use the `m3ApiRawFunction` macro (returns `const void*`). Read args with `m3ApiGetArg`. Link with `m3_LinkRawFunction`. Access user data via `m3_GetUserData(runtime)`.
- **SDL3 rendering**: all drawing goes through the `Gui` class (`src/gui.h`). Do not call SDL functions directly from `main.cpp` or `wasm_kernel.cpp`.
- **Utilities**: string helpers live in `src/util.h`; add new general-purpose helpers there.
- **Header guards**: use `#pragma once` in all headers.
- **Error handling**: prefer returning `bool` / `std::optional` over throwing exceptions in low-level code.
- **Tests**: one `test_<module>.cpp` file per module; each test function is named `test_<feature>`.

## Build & Run

```bash
# One-time setup
bash scripts/setup.sh

# Build (default: linux-debug)
bash scripts/build.sh

# Build a specific target
bash scripts/build.sh linux-release

# Run in terminal (headless) mode
bash scripts/run.sh

# Run with SDL3 GUI window
bash scripts/run.sh --gui

# Build and run all unit tests
bash scripts/test.sh
```

## Important Constraints

- The WASM binary must stay structurally valid — always validate mutations with wasm3 `ParseModule` before accepting them.
- Do not introduce division opcodes (`i32.div_s`, `i32.div_u`, `i32.rem_s`, `i32.rem_u`) in the evolution engine — they trap on divide-by-zero.
- The kernel `run(ptr, len)` function must maintain a balanced operand stack; all generated instruction sequences must be stack-neutral.
- Evolution limits the function body to 32 KB to prevent runaway growth.
- The seed kernel binary constant is defined in `src/wasm_kernel.cpp`; do not change it without also updating the WAT comment alongside it.
