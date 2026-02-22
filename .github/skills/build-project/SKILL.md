---
name: build-and-test
description: Build the C++ project using the provided scripts and run the unit test suite. Use this skill when needing to compile, verify, or troubleshoot the native application on Linux or Windows targets.
---

# build-project

Quick reference for building the code and running tests.

```bash
bash scripts/setup.sh             # install deps
bash scripts/setup.sh --clean     # wipe external/ and bin/

bash scripts/build.sh             # linux-debug (default)
bash scripts/build.sh <target>    # linux-release, windows-debug, windows-release
bash scripts/build.sh --clean <target>

bash scripts/test.sh              # build + run all tests
``` 

Targets produce `build/<target>/bootloader(.exe)` and link against
SDL3/wasm3 as needed.  Add sources in `CMakeLists.txt` under
`target_sources(bootloader …)` and `enable_testing()` for new tests.

Troubleshooting: missing SDL3? rerun setup; missing wasm3? `git
submodule update --init`; install Ninja or bump CMake (>3.20) if errors.
