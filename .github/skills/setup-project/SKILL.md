````skill
---
name: setup-project
description: Install or reset all third-party dependencies used by the project (SDL3, ImGui, wasm3, Catch2). Use this skill when configuring a new development machine or regenerating externals.
---

# setup-project

## Purpose

Install or reset all third-party dependencies used by the project:
SDL3, Dear ImGui, wasm3 and Catch2. This skill is used when setting up a
new development machine, after cleaning the externals directory, or when
cloning the repo for the first time.

## Behaviour

- Deletes `external/` and `bin/` when `--clean` is passed.
- Installs required system packages (`cmake`, `build-essential`, fonts,
  etc.) and may require `sudo`.
- Clones or updates the third‑party repositories and builds SDL3 (and
  Catch2 headers) into `external/`.
- Optionally installs MinGW‑w64 when invoked with the `windows` arg.
- Emits coloured `[INFO]/[WARN]/[ERROR]` messages and section headers for
  readability.
- Safe to rerun; it skips already-populated directories unless `--clean`.

## Usage

```bash
bash scripts/setup.sh              # normal install
bash scripts/setup.sh --clean      # wipe external/ & bin/ then install
bash scripts/setup.sh windows      # add Windows cross-compile support
bash scripts/setup.sh --clean windows
```

This script targets Ubuntu/WSL but generally works on Debian-derived
systems.  It can also be used on macOS with manual adjustments.
````