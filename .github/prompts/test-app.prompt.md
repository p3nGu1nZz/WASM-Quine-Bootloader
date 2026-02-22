# Test the Application

Build and run the unit tests for the WASM Quine Bootloader C++ project.

## Quick Commands

```bash
# Build the app and tests, then run all tests
bash scripts/test.sh

# Run a single test binary directly
./build/linux-debug/test_<module>
```

## How `scripts/test.sh` Works

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

1. Create `test/test_<module>.cpp`.
2. Include the module header: `#include "../src/<module>.h"`.
3. Write test functions named `test_<feature>()` returning `bool`:

```cpp
#include "../src/util.h"
#include <cassert>
#include <cstdio>

bool test_base64_roundtrip() {
    const std::string input = "hello";
    const std::string encoded = util::base64Encode(input);
    const std::string decoded = util::base64Decode(encoded);
    return decoded == input;
}

int main() {
    int failed = 0;
    auto run = [&](const char* name, bool result) {
        std::printf("[%s] %s\n", result ? "PASS" : "FAIL", name);
        if (!result) ++failed;
    };

    run("base64_roundtrip", test_base64_roundtrip());

    std::printf("\n%d test(s) failed.\n", failed);
    return failed > 0 ? 1 : 0;
}
```

4. Register the new test executable in `CMakeLists.txt`:

```cmake
add_executable(test_util test/test_util.cpp src/util.cpp)
target_include_directories(test_util PRIVATE src external/wasm3/source)
target_compile_features(test_util PRIVATE cxx_std_17)
add_test(NAME test_util COMMAND test_util)
```

## Troubleshooting

- **Test not found by ctest**: verify the `add_test(NAME … COMMAND …)` line is present in `CMakeLists.txt`.
- **Link errors**: make sure all `.cpp` files needed by the test are listed in `add_executable`.
- **wasm3 headers missing**: add `external/wasm3/source` to `target_include_directories`.
