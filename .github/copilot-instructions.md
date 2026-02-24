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

## Known limitations (for documentation updates)

* Telemetry JSON files are currently invalid due to a missing comma;
  update `App::autoExport()` when generating docs or code examples.
* `Trainer::observe` is a stub; documentation should not claim the network
  is learning yet.

## Conventions

- C++17; prefer `string_view`, structured bindings, `[[nodiscard]]`.
- wasm3 host functions use `m3ApiRawFunction`/`m3ApiGetArg`/`m3_GetUserData`.
- GUI code confined to `Gui` class; main and kernel avoid SDL calls.
- Helpers in `src/util.h`; add new utils there.
- Use `#pragma once` everywhere.
- Error paths return `bool`/`optional`, don’t throw.
- Tests: one `test_<module>.cpp`; functions named `test_<feature>`.

## Build & run

> **NOTE for agents:** the `run.sh` script was recently corrected so it now
> uses the `BINARY` variable after `cd`.  When asked to start the application,
> you may safely call `scripts/run.sh` with any flags and the working directory
> need not match the build output.


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

Agent/build scripts must use a `./.tmp` directory in the repo root for temporary files and pipes instead of `/tmp`, keeping all transient artifacts scoped to the project and easing cleanup/debugging.  Do **not** redirect output or create files under `/tmp`; all agents and tools should confine temporary data to the repository (e.g. `./.tmp`).

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
- `setup-project` – install or reset third‑party dependencies (SDL3, ImGui, wasm3, Catch2).
- `build-app` – compile the C++ application and run unit tests using provided scripts.
- `run-app` – launch the bootloader executable in GUI/headless mode or tail its log output.
- `test-app` – build and execute the Catch2 unit test suite.
- `update-docs` – refresh README, copilot‑instructions and other documentation to match code changes.
- `update-specs` – edit or add design specification documents under `docs/`.
- `update-skills` – regenerate skill metadata and copilot‑instructions after altering skills.
- `update-memory` – add or refresh repository-specific facts in the agent's memory store.
- `search-memory` – query the memory store for past facts.
- `repo-facts` – scan the repository and return a comprehensive technical summary.
- `telemetry-review` – analyse runtime logs and sequence exports for anomalies or trends.
- `update-issues` – synchronise GitHub issues with the codebase (close implemented items, update descriptions, spawn new tasks).
- `improve-src` – perform bulk source-code improvements: fix errors, refactor, prune dead code.
- `improve-tests` – strengthen the unit tests for better coverage and robustness.
- `code-review` – conduct an adversarial, comprehensive review of proposed changes.
- `commit-push` – stage local changes, craft a descriptive commit message, and push to the remote.
- `introspect-telemetry` – examine logs/sequence exports to identify run‑time anomalies.
- `timed-run` – launch the bootloader for a specified duration and automatically perform a telemetry review.
- `improve-skills` – refine and compact existing skill documents and prompts.

Each skill has a `SKILL.md` in `.github/skills/`. Add new skills there with YAML frontmatter and concise instructions; longer references can live elsewhere.  Workflow prompts live under `.github/prompts/` and are referenced by the `repo-facts` and `update-issues` skills when following the project workflow.

> **Execution note:** when a user asks the agent to run a specific skill
> (for example, "run `update-docs`" or "improve the skill set"), the
> agent should carry out the actions described in that skill and make the
> corresponding edits.  Merely parroting the checklist without performing
> the work is insufficient; skills are **automated procedures** rather than
> loose guidelines.
