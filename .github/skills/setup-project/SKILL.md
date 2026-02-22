````skill
---
name: setup-project
description: Install or reset all third-party dependencies used by the project (SDL3, ImGui, wasm3, Catch2). Use this skill when configuring a new development machine or regenerating externals.
---

# setup-project

Install or reset third-party dependencies (SDL3, ImGui, wasm3, Catch2).
Run when configuring a new machine or after cleaning externals.

Usage examples:
```bash
bash scripts/setup.sh              # normal install
bash scripts/setup.sh --clean      # wipe external/ & bin/ then install
bash scripts/setup.sh windows      # add Windows cross compile support
bash scripts/setup.sh --clean windows
```

Behaviour:
- Deletes `external/` and `bin/` if `--clean`.
- Installs system packages (cmake, build-essential, fonts, etc.).
- Optionally installs MinGW-w64 with `windows` arg.
- Clones/updates wasm3, ImGui, Catch2 and builds SDL3 + Catch2 headers.
- Emits coloured `[INFO]/[WARN]/[ERROR]` output and section headings.

Outputs populated `external/` and `bin/` dirs; safe to rerun (skips if present).
Designed for Ubuntu/WSL but works on other Debian-derived systems.
````