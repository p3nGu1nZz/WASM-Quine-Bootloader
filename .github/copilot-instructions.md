# WASM Quine Bootloader – Copilot Instructions

## Overview

WASM Quine Bootloader runs a self‑replicating WebAssembly kernel that
writes its own base64 source, verifies the quine, then mutates and reboots
itself each generation.  The native C++17 app (CMake/Ninja build) displays
the simulation with an SDL3/ImGui HUD or runs headless with ANSI output.
No web stack involved; wasm3 provides the interpreter.

## Structure

Key directories:
- `src/` – application logic, GUI, wasm modules, CLI parser.
- `test/` – Catch2 unit tests.
- `scripts/` – helpers: `setup.sh`, `build.sh`, `run.sh`, `test.sh`.
- `external/` – third‑party libs built by `setup.sh` (SDL3, wasm3, imgui, Catch2).
- `docs/` – design, architecture; `docs/specs/` holds component specifications.

`CMakeLists.txt` defines `core` library and `bootloader` executable.

External submodule trees are ignored; use `scripts/setup.sh` to populate.

## Build targets

- `linux-debug` (default), `linux-release`, `windows-debug`, `windows-release`.
- Output: `build/<TARGET>/bootloader` (`.exe` on Windows).

Use `bash scripts/build.sh [<target>]` (add `--clean` to wipe).  Setup
dependencies first with `bash scripts/setup.sh`.

## Runtime modes

- **GUI** (default): SDL3/ImGui window starts maximized. Pass
  `--windowed`/`--fullscreen` to change; `--headless`/`--no-gui` disables UI.
- **Headless**: `--headless` runs without a window (useful for CI). The CLI
  parser is in `src/cli.*`.

Telemetry/logs:
- Logs → `build/<target>/bin/logs/*.log`.
- Exports → `build/<target>/bin/seq/<runid>/gen_<n>.txt` plus kernel blobs.
  Use `telemetry-review` skill to analyse; keep `docs/specs/spec_telemetry.md`
  up to date.

## Conventions

- C++17; prefer `string_view`, structured bindings, `[[nodiscard]]`.
- wasm3 host functions use `m3ApiRawFunction`/`m3ApiGetArg`/`m3_GetUserData`.
- GUI code confined to `Gui` class; main and kernel avoid SDL calls.
- Helpers in `src/util.h`; add new utils there.
- Use `#pragma once` everywhere.
- Error paths return `bool`/`optional`, don’t throw.
- Tests: one `test_<module>.cpp`; functions named `test_<feature>`.

## Build & run

```bash
# Setup dependencies (one-time or after cleaning)
bash scripts/setup.sh [--clean] [windows]

# Build targets
bash scripts/build.sh [<target>]      # default linux-debug
bash scripts/build.sh --clean <target> # clean then build

# Run
bash scripts/run.sh [--headless|--windowed]   # GUI by default
bash scripts/run.sh --monitor                 # tail logs

# Tests
bash scripts/test.sh                          # build + run all tests
```

Executables run from their build folder; logs/seq created there.

## Constraints

- Mutated WASMs must parse (`wasm3::ParseModule`).
- Never add division opcodes (traps on zero).
- `run(ptr,len)` must leave a balanced operand stack.
- Code section limited to 32 KB.
- Seed kernel constant in `src/wasm/kernel.cpp` must match its WAT comment.

## Agent skills

Available skills:
- `setup-project` – install externals.
- `build-project` – compile the code.
- `run-app` – launch GUI/headless or tail logs.
- `test-app` – build and run tests.
- `update-docs` – refresh README and related docs.
- `update-specs` – edit specs under `docs/specs/`.
- `update-skills` – update skill docs and copilot-instructions.
- `update-memory` – write repository facts to memory.
- `find-memory` – search the memory store.
- `telemetry-review` – inspect logs/seq exports.
- `generate-report` – create porting/migration reports.

Each skill has a `SKILL.md` in `.github/skills/`. Add new skills there with YAML frontmatter and concise instructions; longer references can live elsewhere.
