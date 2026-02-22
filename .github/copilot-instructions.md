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
│   ├── gui/window.h / gui/window.cpp # Gui class – SDL3 window, panel rendering, F11 fullscreen toggle
│   ├── util.h / util.cpp       # String and general utilities
│   └── wasm/kernel.cpp         # wasm3 host – boots kernel, wires env.log / env.grow_memory
├── test/                       # Unit tests (one .cpp file per module under test)
├── external/
│   ├── SDL3/                   # SDL3 source (built from source via setup.sh)
│   └── wasm3/                  # wasm3 embedded WASM interpreter
├── scripts/
│   ├── build.sh                # Build wrapper (target: linux-debug, linux-release, …)
│   ├── run.sh                  # Build (if needed) then run the bootloader
│   ├── test.sh                 # Build app + tests then run all tests
│   └── setup.sh                # One-time install of system deps (SDL3 from source, etc.)
└── (legacy web prototype removed)  
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

The project supports two running modes. The C++ port currently implements the SDL3 GUI path; a headless terminal renderer is planned and documented but may not be fully implemented yet. Use the `--gui` flag to enable the graphical mode.

### GUI Mode (SDL3 + ImGui)
Run the graphical application (SDL3 window):

- Background is black by default.  **By design the window now starts fullscreen** using `SDL_WINDOW_FULLSCREEN_DESKTOP`; F11 still toggles between windowed and fullscreen for user control.
- The CLI accepts `--windowed` and `--fullscreen` flags to override the default; `--headless`/`--no-gui`/`--nogui` will disable UI entirely.

### Headless / Terminal Mode
The original design included an ANSI/terminal renderer. Headless mode is enabled with `--headless` (or the negated GUI flags) and exercises the bootloader logic without creating an SDL3 window. Use this for automated runs or CI where no display is available. The CLI parsing lives in `src/cli.*`.

### Telemetry & Logs
The bootloader automatically writes verbose logs under `build/<target>/bin/logs/` and generation exports under `build/<target>/bin/seq/<runid>/`. A new skill `telemetry-review` helps inspect these files for anomalies and evolution trends; update `docs/specs/spec_telemetry.md` whenever you alter the export format.

## Coding Conventions

- **C++17** – use `std::string_view`, structured bindings, `[[nodiscard]]`, etc. where appropriate.
- **wasm3 host functions** must use the `m3ApiRawFunction` macro (returns `const void*`). Read args with `m3ApiGetArg`. Link with `m3_LinkRawFunction`. Access user data via `m3_GetUserData(runtime)`.
- **SDL3 rendering**: all drawing goes through the `Gui` class (`src/gui/window.h`). Do not call SDL functions directly from `main.cpp` or `wasm/kernel.cpp`.
- **Utilities**: string helpers live in `src/util.h`; add new general-purpose helpers there.
- **Header guards**: use `#pragma once` in all headers.
- **Error handling**: prefer returning `bool` / `std::optional` over throwing exceptions in low-level code.
- **Tests**: one `test_<module>.cpp` file per module; each test function is named `test_<feature>`.

## Build & Run

```bash
# One-time setup
bash scripts/setup.sh        # or `setup.sh windows` for cross-build

# Clean third-party sources and rerun setup
bash scripts/setup.sh --clean

# Build (default: linux-debug)
bash scripts/build.sh

# Build a specific target
bash scripts/build.sh linux-release

# Clean then build
bash scripts/build.sh --clean linux-debug

# Run in terminal (headless) mode
bash scripts/run.sh

# Run with SDL3 GUI window
bash scripts/run.sh --gui

# Tail the runtime log
bash scripts/run.sh --monitor

# Build and run all unit tests
bash scripts/test.sh
```

By default the executable is started with its working directory set to the
build target (e.g. `build/linux-debug`).  Runtime log files are written to
`bin/logs/` inside that directory, and automatic telemetry exports live under
`bin/seq/<runid>/` (generation reports, kernel blobs, etc.).

## Important Constraints

- The WASM binary must stay structurally valid — always validate mutations with wasm3 `ParseModule` before accepting them.
- Do not introduce division opcodes (`i32.div_s`, `i32.div_u`, `i32.rem_s`, `i32.rem_u`) in the evolution engine — they trap on divide-by-zero.
- The kernel `run(ptr, len)` function must maintain a balanced operand stack; all generated instruction sequences must be stack-neutral.
- Evolution limits the function body to 32 KB to prevent runaway growth.
- The seed kernel binary constant is defined in `src/wasm/kernel.cpp`; do not change it without also updating the WAT comment alongside it.

## Agent Skills

This repository includes a small set of agent "skills" used by the Copilot agent. The current skills folder contains:

- `setup-project` – installs or refreshes third‑party dependencies (SDL3, ImGui, wasm3, Catch2) and can clean them.
- `build-project` – compiles the project for a specified target.
- `run-app` – launches the bootloader in terminal or GUI mode.
- `test-app` – builds and executes the unit tests.
- `update-docs` – refreshes README and other documentation after changes.
- `update-specs` – edits or adds design specifications in `docs/`.
- `update-skills` – review and refresh agent skills, copilot-instructions, and memory entries when repository behaviour changes.
- `generate-report` – produces a migration/porting report from a code review.

Each skill lives in its own directory under `.github/skills/<skill-name>/` and must contain a `SKILL.md` file conforming to the Agent Skills specification (see [https://agentskills.io/specification](https://agentskills.io/specification)).

A valid `SKILL.md` has YAML frontmatter with at least `name` and `description` fields, followed by Markdown instructions. Frontmatter may also include `license`, `compatibility`, `metadata`, and `allowed-tools` if needed. The Markdown body should explain steps, examples, inputs and outputs, and any edge cases.

Example skills are already provided; follow their pattern when adding new ones. Keep skill files concise (roughly <500 lines) and place longer reference content in accompanying `scripts/`, `references/`, or `assets/` subdirectories if necessary.
