# Design Document — WASM Quine Bootloader

## 1. Goals

The project explores **self-referential computation** by combining two concepts:

1. A **WASM quine** — a WebAssembly binary whose only job is to write its own
   source representation into a host buffer and call a log import.
2. A **genetic mutation engine** — each successful quine cycle produces a
   slightly mutated descendant binary that is then validated and run.

The desktop application makes the simulation **observable in real time** via a
multi-panel Dear ImGui HUD rendered through SDL3.

---

## 2. Simulation Loop

Each frame (≈ 60 Hz) the `App::update()` drives the `BootFsm`:

```
Frame N
│
├─ BootFsm::current() == IDLE
│     → App::startBoot()         (transition → BOOTING)
│
├─ BootFsm::current() == BOOTING
│     → wait bootSpeed ms        (scales down with generation)
│     → transition → LOADING_KERNEL
│
├─ BootFsm::current() == LOADING_KERNEL
│     → advance loading progress LOAD_STEP bytes/frame
│     → when done: WasmKernel::bootDynamic()
│     → transition → EXECUTING
│
├─ BootFsm::current() == EXECUTING
│     → step through instructions at stepSpeed ms/instr
│     → at opcode CALL (0x10): WasmKernel::runDynamic()
│         ├─ host env.log callback → App::onWasmLog()
│         │     ├─ output == kernel?  → transition → VERIFYING_QUINE
│         │     └─ mismatch          → handleBootFailure()
│         └─ host env.grow_memory    → App::onGrowMemory()
│
├─ BootFsm::current() == VERIFYING_QUINE
│     → wait rebootDelayMs (2 s default)
│     → doReboot(success=true)   (generation++, apply mutation)
│     → transition → IDLE
│
├─ BootFsm::current() == REPAIRING
│     → wait 1 500 ms
│     → doReboot(success=false)  (revert to stable kernel)
│     → transition → IDLE
│
└─ BootFsm::current() == SYSTEM_HALT
      → no-op
```

---

## 3. Mutation Strategy

On each successful quine execution `evolveBinary()` is called.  It operates
exclusively on the **code section** of the WASM binary and applies one of four
mutation types, chosen probabilistically:

| Mutation type | Probability | Description |
|---|---|---|
| **Insert** | ~30 % | Splice a random safe instruction into the code section |
| **Delete** | ~20 % | Remove a random non-essential instruction |
| **Modify** | ~30 % | Replace a random instruction with a semantically similar one |
| **Append** | ~20 % | Append one or more instructions from the known-good pool |

Known-good mutation sequences accumulate over generations and are fed back into
the `append` path to bias later evolutions toward proven expansions.

After mutation the binary is base-64 encoded and validated by checking:
- WASM magic bytes `\0asm` at offset 0
- Successful re-parse by `extractCodeSection()`

Any mutation that produces an invalid binary is rejected and the stable kernel
is used instead.

---

---

## 5. WASM Host Imports

The kernel module imports two host functions from the `env` namespace:

| Import | Signature | Behaviour |
|---|---|---|
| `env.log` | `(i32 ptr, i32 len) → void` | Read `len` bytes from WASM memory at `ptr`; compare to current kernel base64 |
| `env.grow_memory` | `(i32 pages) → void` | Flash the memory-growing indicator for 800 ms |

---

## 6. Telemetry Export

Pressing **EXPORT** writes `quine_telemetry_gen<N>.txt` containing:

- Header (generation, kernel size, timestamp)
- Current kernel in base64
- Full hex dump of the raw WASM bytes
- Disassembly listing (index, address, opcode, args)
- History log (every EXECUTE / EVOLVE / REPAIR event)
