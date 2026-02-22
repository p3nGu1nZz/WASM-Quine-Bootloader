````skill
---
name: setup-project
description: Install or reset all third-party dependencies used by the project (SDL3, ImGui, wasm3, Catch2). Use this skill when configuring a new development machine or regenerating externals.
---

# setup-project

## Purpose

Ensure the repository has all required external libraries and tools. The script
handles Linux system packages, clones/updates source trees, and builds SDL3 and
Catch2. It can also completely wipe previously downloaded dependencies when
invoked with `--clean`.

## Usage

```bash
# typical one-time install
bash scripts/setup.sh

# remove all external files + bin/ and run setup again
bash scripts/setup.sh --clean

# also set up Windows cross-compile dependencies:
bash scripts/setup.sh windows

# clean then setup Windows dependencies
bash scripts/setup.sh --clean windows
```

## Behaviour

1. Optionally deletes previous `external/` and `bin/` directories if `--clean` is provided.
2. Installs required system packages (`build-essential`, `cmake`, SDL3 deps,
   fonts, etc.).
3. Optionally installs MinGW-w64 when `windows` argument is passed.
4. Creates `external/` layout and clones or updates wasm3, ImGui, Catch2.
5. Builds SDL3 for Linux (and Windows if requested) and installs into
   `external/SDL3/*`.
6. Builds Catch2 and installs its headers for use by tests.

The script emits colour-coded `[INFO]/[WARN]/[ERROR]` messages and
`===== Section =====` headings for clarity.

## Inputs

- `windows` (optional) – configure Windows cross compile tools.
- `--clean` (optional) – wipe existing externals and bin/ then proceed.

## Outputs

- Populated `external/` directory with SDL3, ImGui, wasm3, Catch2 sources and
  built libraries.
- `bin/` folder created under the working directory if tests or runs generate
  files; cleaned on `--clean`.
- Colorized log messages printed during execution.

## Notes

- Running with `--clean` after a build will automatically rebuild dependencies
  so the command behaves like an idempotent install.
- Designed for Ubuntu/WSL but should work on other Debian-derived systems.
````