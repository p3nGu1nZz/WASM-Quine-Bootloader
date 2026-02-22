---
name: build-and-test
description: Build the C++ project using the provided scripts and run the unit test suite. Use this skill when needing to compile, verify, or troubleshoot the native application on Linux or Windows targets.
---

# build-project

## Quick Commands

```bash
# One-time dependency setup (only needed once per machine)
bash scripts/setup.sh

# Clean dependencies & build directory then rerun setup
bash scripts/setup.sh --clean

# Build default target (linux-debug)
bash scripts/build.sh

# Build a specific target (linux-release, windows-debug, windows-release)
bash scripts/build.sh <target>

# Clean build directory and rebuild a target
display \"build.sh --clean <target>\"
```

The build script prints coloured `[build]` messages and suppresses
spurious warnings from third‑party sources for a tidy console output.

### Build Targets

| Target | Output binary |
|--------|---------------|
| `linux-debug` | `build/linux-debug/bootloader` |
| `linux-release` | `build/linux-release/bootloader` |
| `windows-debug` | `build/windows-debug/bootloader.exe` |
| `windows-release` | `build/windows-release/bootloader.exe` |

## How `scripts/build.sh` Works

1. Accepts an optional target argument (default: `linux-debug`).
2. Runs `cmake -S . -B build/<TARGET>` with any target-specific flags (e.g. MinGW toolchain for Windows targets).
3. Runs `cmake --build build/<TARGET>` using Ninja.
4. Exits non-zero on any error.

## Modifying `CMakeLists.txt`

- Add new source files to the `target_sources(bootloader …)` block.
- Add new test executables in the `enable_testing()` section at the bottom.
- Link new external libraries via `target_link_libraries`.
- Keep C++ standard set to **C++17** (`set(CMAKE_CXX_STANDARD 17)`).

## Troubleshooting

- **SDL3 not found**: run `bash scripts/setup.sh` to build SDL3 from source into `external/SDL3/build/`.
- **wasm3 compile errors**: check `external/wasm3/` is present (it is a git submodule – run `git submodule update --init`).
- **Ninja not found**: install with `sudo apt install ninja-build` (Linux) or `choco install ninja` (Windows).
- **CMake version too old**: project requires CMake ≥ 3.20.
