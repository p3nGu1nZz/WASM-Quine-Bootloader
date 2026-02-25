# Example WASM Kernels

This directory contains three minimal WebAssembly kernels that demonstrate
the host import interfaces provided by the WASM Quine Bootloader.

| File | Host imports used | Purpose |
|------|-------------------|---------|
| `spawn_kernel.wat` | `env.log`, `env.spawn` | Asks the bootloader to create a sibling instance |
| `kill_kernel.wat` | `env.log`, `env.kill_instance` | Requests that instance 0 be terminated |
| `weight_kernel.wat` | `env.log`, `env.record_weight` | Sends a dummy neural-matrix blob to the host |

Each kernel exports a single `run(ptr:i32, len:i32)` function and one page
of linear memory named `memory`, matching the bootloader's calling convention.

## Building from WAT source

If you have [WABT](https://github.com/WebAssembly/wabt) installed:

```bash
wat2wasm spawn_kernel.wat  -o spawn_kernel.wasm
wat2wasm kill_kernel.wat   -o kill_kernel.wasm
wat2wasm weight_kernel.wat -o weight_kernel.wasm
```

Then re-encode to base64 for use in the test suite or `constants.h`:

```bash
base64 -w0 spawn_kernel.wasm
```

The pre-computed base64 strings are embedded in
[`src/core/constants.h`](../src/core/constants.h) as `SPAWN_KERNEL_GLOB`,
`KILL_KERNEL_GLOB`, and `WEIGHT_KERNEL_GLOB` so that no build toolchain is
required to run the tests.

## Running the tests

These kernels are exercised automatically by the `test_wasm` Catch2 suite:

```bash
bash scripts/test.sh
```

The tests boot each kernel via `WasmKernel::bootDynamic`, call
`runDynamic`, and assert that the corresponding callback is fired.

## License

The WAT sources and their compiled binary equivalents are original works
released under the same MIT license as the rest of this project.
