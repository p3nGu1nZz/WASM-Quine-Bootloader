# Unit Tests

This directory contains unit tests for the WASM Quine Bootloader C++ project.

## Layout

One `.cpp` file per module under test:

| File | Tests for |
|------|-----------|
| `test_wasm_kernel.cpp` | `src/wasm/kernel.cpp` – boot, run, terminate |
| `test_util.cpp` | `src/util.h` – string helpers, Base64, LEB128 |
| `test_evolution.cpp` | WASM binary mutation / evolution logic |

## Running Tests

```bash
bash scripts/test.sh
```

This builds the project (if needed), builds all test targets, and runs them via `ctest`.

## Writing a New Test

See `.github/prompts/test-app.prompt.md` for the full guide, including how to register
a new test in `CMakeLists.txt`.
