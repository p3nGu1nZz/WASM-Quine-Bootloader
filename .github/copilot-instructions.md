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

Additional CLI flags (see `docs/specs/spec_cli.md`):

- `--telemetry-level` / `--telemetry-dir` – control export verbosity and
destination.  Unknown values generate stderr warnings but fall back to a
sane default.
- `--mutation-strategy` / `--heuristic` – choose evolution policy; blacklist
  heuristic avoids repeated trap-causing edits.
- `--profile` – record timing and memory stats in the log.
- `--max-gen` – limit total successful generations, handy for tests.
- `--save-model` / `--load-model` – persist or restore the trainer model
  between runs.
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

> **Note:** builds now use `-Werror` for GCC/Clang, so resolve any
> compilation warnings (or clean the tree) before retrying.  The core
> targets and unit tests are compiled with the same flags.

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

> **Note:** the build scripts respect the standard `CC`/`CXX` environment
> variables.  Export a different compiler (e.g. `export CXX=/usr/bin/g++-12`)
> before running `build.sh` if you need a non-default toolchain.  The
> wrapper will pass those overrides through to CMake automatically.

> **Maintainer tip:** after adding or renaming skills or modifying
> telemetry/CLI behaviour, run `update-skills` followed by `update-memory`.
> These ensure the agent documentation and memory stay in sync with the
> repository state.  If you also add or change a workflow prompt under
> `.github/prompts/`, update the README and any docs that reference it so
> agents know about the new workflow.

Executables run from their build folder; logs/seq created there.

Agent/build scripts must use a `./.tmp` directory in the repo root for temporary files and pipes instead of `/tmp`, keeping all transient artifacts scoped to the project and easing cleanup/debugging.

## Constraints

- Mutated WASMs must parse (`wasm3::ParseModule`).
- The `App` caches decoded kernel bytes and extracted instruction lists
  (`m_currentKernelBytes`) to avoid repeated base64 decoding during the
  main loop.
- Never add division opcodes (traps on zero).
- `run(ptr,len)` must leave a balanced operand stack.
- Code section limited to 32 KB.
- Seed kernel constant in `src/wasm/kernel.cpp` must match its WAT comment.

## Agent skills

Available skills:
- `setup-project` – install externals.
- `build-app` – compile the code.
- `run-app` – launch GUI/headless or tail logs.
- `test-app` – build and run tests.
- `update-docs` – refresh README and related docs.
- `update-specs` – edit specs under `docs/specs/`.
- `update-skills` – update skill docs and copilot-instructions.
- `update-memory` – write repository facts to memory.
- `search-memory` – search the memory store.
- `repo-facts` – dig through the repository and return a comprehensive technical summary.
- `telemetry-review` – inspect logs/seq exports.
- `update-issues` – synchronise GitHub issues with the codebase (close done items, update descriptions, generate new tasks).
- `improve-src` – sweep source files fixing errors, refactoring and removing dead code.
- `improve-tests` – expand and strengthen the unit test suite for greater coverage.
- `code-review` – perform a thorough adversarial review of code changes and suggest improvements.
- `commit-push` – stage changes, craft a commit message, and push to remote.
- `introspect-telemetry` – review logs and sequence exports for anomalies.
- `improve-skills` – refine and compact all existing skill documents.

Each skill has a `SKILL.md` in `.github/skills/`. Add new skills there with YAML frontmatter and concise instructions; longer references can live elsewhere.  Workflow prompts live under `.github/prompts/` and are referenced by the `repo-facts` and `update-issues` skills when following the project workflow.

> **Execution note:** when a user asks the agent to run a specific skill
> (for example, "run `update-docs`" or "improve the skill set"), the
> agent should carry out the actions described in that skill and make the
> corresponding edits.  Merely parroting the checklist without performing
> the work is insufficient; skills are **automated procedures** rather than
> loose guidelines.
