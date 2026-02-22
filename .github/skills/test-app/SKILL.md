---
name: test-app
description: Build and run the unit tests for the WASM Quine Bootloader application. Use when verifying correctness of individual modules after changes.
---

# test-app

## Purpose

Compile the code and execute the project's unit-test binaries. This skill mirrors the content of `.github/prompts/test-app.prompt.md` and is intended for CI and local test runs.

## Quick Commands

```bash
# Build the app and tests, then run all tests
bash scripts/test.sh

# Run a single test binary directly
./build/linux-debug/test_<module>
```

## Behaviour

1. Calls `bash scripts/build.sh` to ensure the main application is up-to-date.
2. Builds all test targets defined in `CMakeLists.txt` under `enable_testing()`.
3. Runs `ctest --output-on-failure` inside the build directory.
4. Reports a summary: number of tests passed / failed.
5. Exits non-zero if any test fails.

## Test Directory Layout

```
test/
├── test_wasm_kernel.cpp    # Tests for src/wasm_kernel.cpp (boot, run, terminate)
├── test_util.cpp           # Tests for src/util.h utility functions
└── test_evolution.cpp      # Tests for WASM binary mutation logic
```

## Writing a New Test

(see prompt file for details)

## Troubleshooting

(see prompt file for troubleshooting tips)
