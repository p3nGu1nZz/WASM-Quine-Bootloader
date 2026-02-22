# Architecture Document — WASM Quine Bootloader

## Module Map

```
main.cpp
 └── Gui          (gui/window.h / gui/window.cpp)
      ├── GuiHeatmap    (gui/heatmap.h / gui/heatmap.cpp)
 │    └── [uses] gui/colors.h, util.h, wasm/parser.h
 └── App          (app.h / app.cpp)
      ├── BootFsm       (fsm.h / fsm.cpp)
      ├── AppLogger     (log.h / log.cpp)
      ├── WasmKernel    (wasm/kernel.h / wasm/kernel.cpp)
      ├── [uses] wasm/evolution.h / wasm/evolution.cpp
      ├── [uses] wasm/parser.h / wasm/parser.cpp
      ├── [uses] exporter.h / exporter.cpp
      └── [uses] base64.h, constants.h, types.h, util.h
```

---

## Source File Specifications

### `src/main.cpp`
**Role:** Entry point.

- Initialises SDL3 (video subsystem, window, renderer).
- Creates `Gui` and `App` instances.
- Runs the event loop: polls SDL events → `app.update()` → `gui.renderFrame(app)`.
- Destroys everything in reverse order on exit.

**Dependencies:** `app.h`, `gui/window.h`, `SDL3/SDL.h`, `backends/imgui_impl_sdl3.h`

---

### `src/types.h`
**Role:** Shared plain-old-data types used throughout the codebase.

| Symbol | Kind | Description |
|---|---|---|
| `SystemState` | enum class | FSM states: IDLE, BOOTING, LOADING_KERNEL, EXECUTING, VERIFYING_QUINE, SYSTEM_HALT, REPAIRING |
| `SystemEra` | enum class | Visual era: PRIMORDIAL, EXPANSION, COMPLEXITY, SINGULARITY |
| `LogEntry` | struct | id, timestamp (ms), message, type string |
| `HistoryEntry` | struct | generation, ISO timestamp, size, action, details, success flag |
| `BootConfig` | struct | memorySizePages, autoReboot, rebootDelayMs |

**Dependencies:** `<string>`, `<cstdint>` — no project headers.

---

### `src/constants.h`
**Role:** Compile-time constants.

| Symbol | Description |
|---|---|
| `KERNEL_GLOB` | Base64-encoded minimal WASM binary (the initial quine kernel) |
| `DEFAULT_BOOT_CONFIG` | Default `BootConfig` values |

**Dependencies:** `types.h`

---

### `src/base64.h`
**Role:** Header-only Base64 encode/decode utilities.

| Function | Signature | Description |
|---|---|---|
| `base64_encode` | `(const std::vector<uint8_t>&) → std::string` | Encode raw bytes |
| `base64_decode` | `(const std::string&) → std::vector<uint8_t>` | Decode base64 string |

**Dependencies:** `<string>`, `<vector>`, `<cstdint>` — no project headers.

---

### `src/util.h` / `src/util.cpp`
**Role:** Cross-cutting string and time utilities.

| Function | Description |
|---|---|
| `stateStr(SystemState)` | Human-readable state name |
| `eraStr(SystemEra)` | Human-readable era name |
| `randomId()` | Generate a 9-character alphanumeric ID |
| `nowIso()` | Current UTC time as ISO-8601 string |

**Dependencies:** `types.h`

---

### `src/fsm.h` / `src/fsm.cpp`
**Role:** Finite State Machine governing the boot lifecycle.

| Member | Description |
|---|---|
| `transition(SystemState)` | Move to a new state; fires `OnTransition` callback |
| `current()` | Query the current state |
| `elapsedMs()` | Milliseconds since the last transition |
| `enteredAt()` | SDL tick timestamp of the last transition |
| `setTransitionCallback(cb)` | Register a state-change observer |

Transition history is not recorded here; that is the responsibility of `AppLogger`.

**Dependencies:** `types.h`, `SDL3/SDL.h`

---

### `src/logger.h` / `src/logger.cpp`
**Role:** Append-only log ring-buffer and immutable history ledger.

| Member | Description |
|---|---|
| `log(msg, type)` | Append a `LogEntry`; deduplicates within 100 ms; caps at 1 000 entries |
| `addHistory(entry)` | Append a `HistoryEntry` (never truncated) |
| `logs()` | Read-only reference to the live log `std::deque` |
| `history()` | Read-only reference to the history `std::vector` |

**Dependencies:** `types.h`, `util.h`, `SDL3/SDL.h`

---

### `src/exporter.h` / `src/exporter.cpp`
**Role:** Generate a human-readable telemetry report.

| Symbol | Description |
|---|---|
| `ExportData` | POD aggregate: generation, era, kernel, instructions, logs, history |
| `buildReport(data)` | Returns a multi-section text string (header, hex dump, disassembly, history log) |

**Dependencies:** `types.h`, `wasm/parser.h`, `base64.h`, `util.h`

---

### `src/app.h` / `src/app.cpp`
**Role:** Top-level simulation orchestrator.

Coordinates `BootFsm`, `AppLogger`, `WasmKernel`, and the mutation engine.
Implements the boot sequence steps (`startBoot`, `tickBooting`, `tickLoading`,
`tickExecuting`, `tickVerifying`, `tickRepairing`, `doReboot`) and the WASM
host callbacks (`onWasmLog`, `onGrowMemory`).

Exposes read-only accessors for everything the `Gui` needs to render.

**Dependencies:** `fsm.h`, `logger.h`, `exporter.h`, `wasm/kernel.h`, `wasm/parser.h`,
`wasm_evolution.h`, `base64.h`, `constants.h`, `util.h`, `types.h`

---

### `src/wasm/kernel.h` / `src/wasm/kernel.cpp`
**Role:** Wrapper around the wasm3 interpreter.

| Member | Description |
|---|---|
| `bootDynamic(b64, logCb, growCb)` | Decode kernel, instantiate wasm3 runtime, link host imports |
| `runDynamic(b64)` | Write base64 into WASM memory, call exported `run(ptr, len)` |
| `terminate()` | Free wasm3 runtime and environment |
| `isLoaded()` | True when a module is ready to execute |

Uses `m3ApiRawFunction` / `m3ApiGetArg` / `m3_LinkRawFunction` from wasm3.
`KernelUserData` (owned as `m_userData`) is passed as wasm3 user-data and
deleted in `terminate()`.

**Dependencies:** `types.h`, wasm3 headers, `base64.h`

---

### `src/wasm/parser.h` / `src/wasm/parser.cpp`
**Role:** Minimal WASM binary parser.

| Symbol | Description |
|---|---|
| `Instruction` | `{ opcode, args, originalOffset, length }` |
| `decodeLEB128(data, offset)` | Decode unsigned LEB-128 integer |
| `encodeLEB128(value)` | Encode integer as unsigned LEB-128 |
| `extractCodeSection(bytes)` | Parse and return the list of `Instruction`s from the code section |
| `parseInstructions(data, start, end)` | Parse a raw byte range into instructions |
| `getOpcodeName(opcode)` | Map WASM opcode byte to mnemonic string |

**Dependencies:** `<vector>`, `<cstdint>` — no project headers.

---

### `src/wasm/evolution.h` / `src/wasm/evolution.cpp`
**Role:** WASM binary mutation engine.

| Symbol | Description |
|---|---|
| `EvolutionResult` | `{ binary (base64), mutationSequence, description }` |
| `evolveBinary(b64, knownInstructions, seed)` | Apply one mutation to the code section; return a new base64 binary |
| `BASE_SAFE_GENOMES` | Hard-coded fallback safe instruction sequences |

Uses `std::mt19937` seeded from `std::random_device` for all random choices.

**Dependencies:** `wasm_parser.h`, `base64.h`

---

### `src/gui/window.h` / `src/gui/window.cpp`
**Role:** Dear ImGui backend lifecycle and panel orchestration.

`Gui` creates the ImGui context in `init()`, renders all panels in
`renderFrame()`, and tears everything down in `shutdown()`.

Panel helpers:

| Method | Panel |
|---|---|
| `renderTopBar` | ERA / GEN / STATE / UPTIME / control buttons |
| `renderLogPanel` | System log ring-buffer |
| `renderInstrPanel` | WASM instruction list with IP highlight |
| `renderKernelPanel` | Base64 kernel diff viewer |
| `renderStatusBar` | Bottom status line |

Owns a `GuiHeatmap` instance (`m_heatmap`) which renders the memory panel.

**Dependencies:** `app.h`, `gui/heatmap.h`, `gui/colors.h`, `util.h`, `wasm_parser.h`

---

### `src/colors.h`
**Role:** Header-only colour lookup table.

Inline functions mapping `SystemState`, `SystemEra`, and log-type strings to
`ImVec4` RGBA colours.  No `.cpp` companion needed.

| Function | Description |
|---|---|
| `colorForLogType(type)` | Log entry accent colour |
| `colorForState(state)` | State badge colour |
| `colorForEra(era)` | Era badge colour |
| `bgColorForEra(era)` | SDL3 renderer clear colour (background) |

**Dependencies:** `types.h`, `imgui.h`

---

### `src/heatmap.h` / `src/heatmap.cpp`
**Role:** Memory heatmap visualizer.

`GuiHeatmap` maintains a `std::vector<float>` of per-block heat values that
decay each frame. `draw()` renders into an `ImDrawList`; `renderPanel()` renders
the full panel (header + background + heatmap).

Block geometry scales automatically with kernel size:

| Kernel size | Block px | Bytes/block |
|---|---|---|
| < 256 B | 8 | 1 |
| < 1 KB | 5 | 4 |
| ≥ 1 KB | 3 | 16 |

**Dependencies:** `app.h`, `gui/colors.h`, `util.h`, `imgui.h`

---

## Dependency Graph (simplified)

```
types.h ◄── constants.h
   ▲               ▲
   │               │
base64.h    wasm/parser.h ◄── wasm/evolution.h
   ▲               ▲                  ▲
   │               │                  │
util.h ────────────┤           kernel.h
   ▲               │                  ▲
   │               │                  │
log.h     exporter.h          fsm.h
   ▲               ▲                  ▲
   └───────────────┴──── app.h ───────┘
                              ▲
                              │
             gui/colors.h ─── gui/window.h ─── gui/heatmap.h
                              ▲
                              │
                          main.cpp
```

---

## Build System

| File | Role |
|---|---|
| `CMakeLists.txt` | Single `add_executable(bootloader …)` target; C++17; links SDL3::SDL3 statically |
| `scripts/setup.sh` | Installs apt packages, clones external libs, builds SDL3, builds the project |
| `scripts/build.sh` | Configures + compiles one of four targets; `--clean` wipes build/ |
| `scripts/run.sh` | Builds if needed, then execs the binary |
| `cmake/toolchain-windows-x64.cmake` | CMake toolchain for MinGW-w64 Windows cross-compile from WSL |

External libraries live in `external/` (not committed; populated by `setup.sh`):

| Library | Path | Purpose |
|---|---|---|
| SDL3 (source) | `external/SDL3/src/` | Shared SDL3 source tree |
| SDL3 (Linux)  | `external/SDL3/linux/` | Linux build |
| SDL3 (Windows)| `external/SDL3/windows/` | Windows cross-compile (optional) |
| Dear ImGui | `external/imgui/` | Immediate-mode GUI |
| wasm3 | `external/wasm3/` | WebAssembly interpreter |
