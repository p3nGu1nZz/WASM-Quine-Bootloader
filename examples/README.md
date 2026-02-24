# Example WASM Kernels

This directory contains small WebAssembly Text (WAT) modules that demonstrate
the host imports available to every kernel running inside the WASM Quine
Bootloader.  They are kept as `.wat` files so the design intent is readable
without a binary viewer.

## Kernels

| File | Host import exercised | Purpose |
|---|---|---|
| `spawn_kernel.wat` | `env.spawn` | Passes the received source blob straight back to the host, requesting that a copy of itself is recorded as a new instance. |
| `kill_kernel.wat` | `env.kill_instance` | Calls `kill_instance(0)` to ask the host to remove the first active instance. |
| `weight_kernel.wat` | `env.record_weight` | Sends a 4-byte dummy weight matrix at memory offset 0, demonstrating the neural-feature recording path. |

## Building

Each module can be compiled with
[wat2wasm](https://github.com/WebAssembly/wabt) from the WebAssembly Binary
Toolkit:

```bash
wat2wasm spawn_kernel.wat  -o spawn_kernel.wasm
wat2wasm kill_kernel.wat   -o kill_kernel.wasm
wat2wasm weight_kernel.wat -o weight_kernel.wasm
```

The resulting `.wasm` files are not checked in to avoid repository bloat.
Instead, the equivalent pre-computed base64 blobs are embedded directly in
`test/test_examples.cpp` and verified automatically as part of the test suite.
If you change a `.wat` file you must regenerate the corresponding base64
constant in the test.  The generation script lives in `.tmp/` and is not
tracked; the easiest approach is:

```bash
python3 - << 'EOF'
import base64, sys
with open("spawn_kernel.wasm", "rb") as f:
    print(base64.b64encode(f.read()).decode())
EOF
```

## License

All WAT source in this directory is original work released under the same
license as the rest of the project.
