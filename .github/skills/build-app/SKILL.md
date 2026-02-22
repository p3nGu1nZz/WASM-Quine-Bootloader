---
name: build-app
description: Build the C++ application and run unit tests using provided scripts. Use when compiling or verifying the project on any target.
---

# build-app

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
