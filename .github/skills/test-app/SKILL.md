---
name: test-app
description: Build and run the unit tests for the WASM Quine Bootloader application. Use when verifying correctness of individual modules after changes.
---

# test-app

## Purpose

Build the project and run unit tests to catch regressions.

## Quick Commands

```bash
bash scripts/test.sh                    # build & execute all tests
./build/linux-debug/test_<module>       # run individual test binary
```

The harness exercises export/log code; `test_util_dpi` creates temp files
in `bin/` then cleans them.  Failures are shown with coloured Catch2 output.

When `bash scripts/test.sh` completes it prints a summary box listing
how many test executables and total assertions were run along with the
duration, making it easy to see results at a glance.

Behaviour: invokes `build.sh`, builds all `enable_testing()` targets,
runs `ctest --output-on-failure`, and exits non-zero on failure.

See the `test/` directory for example test files.  For writing new tests or
troubleshooting consult the prompt file.
