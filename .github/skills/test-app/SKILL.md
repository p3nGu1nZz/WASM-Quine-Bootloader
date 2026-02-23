---
name: test-app
description: Build and run the unit tests for the WASM Quine Bootloader application. Use when verifying correctness of individual modules after changes.
---

# test-app

## Purpose

Build the project and run unit tests to catch regressions.  Tests
are compiled with the same flags as the main project (`-Werror` etc.), so
new test code must also be warning-free.

## Quick Commands

```bash
bash scripts/test.sh                    # build & execute all tests
./build/linux-debug/test_<module>       # run individual test binary
```

The harness exercises export/log code; `test_util_dpi` creates temp files
in `bin/` then cleans them.  Failures are shown with coloured Catch2 output.

On completion `bash scripts/test.sh` prints a summary box with counts of
test executables, total assertions, and duration for quick overview.

Behaviour: invokes `build.sh`, builds all `enable_testing()` targets,
runs `ctest --output-on-failure`, and exits non-zero on failure.

See the `test/` directory for example test files.  New tests should
follow the naming convention `test_<module>.cpp` and use descriptive
`TEST_CASE` names.  Keep them fast and deterministic.  For writing new
tests or troubleshooting consult the prompt file.
