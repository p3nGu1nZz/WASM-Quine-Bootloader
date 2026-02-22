# WASM Quine Bootloader – Copilot Instructions

## Project Overview

**WASM Quine Bootloader** is a React + TypeScript single-page application that simulates a self-replicating WebAssembly (WASM) bootloader. The core concept: a minimal WASM binary (the "kernel") is instantiated, executes itself, outputs its own source (Base64-encoded binary), verifies that the output matches its own source, and then *evolves* the binary before rebooting into the next generation. This is a quine – a program that reproduces itself – implemented in WebAssembly and visualised in the browser.

## Technology Stack

- **Framework**: React 19 with TypeScript
- **Build Tool**: Vite 6
- **Styling**: Tailwind CSS (utility classes via `className`)
- **Runtime**: WebAssembly (browser-native `WebAssembly.instantiate`)
- **No backend** – everything runs client-side in the browser

## Repository Structure

```
/
├── App.tsx                     # Root component – state machine & boot loop
├── constants.ts                # KERNEL_GLOB (Base64 WASM binary) & DEFAULT_BOOT_CONFIG
├── types.ts                    # Shared TypeScript enums and interfaces
├── index.tsx                   # React entry point
├── index.html                  # HTML shell
├── vite.config.ts              # Vite configuration
├── components/
│   ├── TerminalLog.tsx         # Scrolling log panel with coloured log types
│   ├── SystemStatus.tsx        # Header status bar (state, generation, uptime, era)
│   ├── MemoryVisualizer.tsx    # Animated memory grid showing active byte ranges
│   └── InstructionStream.tsx  # WASM instruction list with animated program counter
├── services/
│   └── wasmService.ts          # WasmKernel class – boots, runs, and terminates instances
└── utils/
    ├── wasmParser.ts           # WASM binary parser (LEB128, opcode names, code section)
    └── wasmEvolution.ts        # Genetic mutation engine (INSERT, DELETE, MODIFY, ADD)
```

## Key Concepts

### SystemState (state machine in App.tsx)
`IDLE → BOOTING → LOADING_KERNEL → EXECUTING → VERIFYING_QUINE → IDLE` (success path)  
`EXECUTING → REPAIRING → IDLE` (failure/self-repair path)

### SystemEra (visual theme)
Generations 0–4: **PRIMORDIAL** (cyan), 5–14: **EXPANSION** (green), 15–29: **COMPLEXITY** (purple), 30+: **SINGULARITY** (red).

### WASM Kernel (constants.ts)
`KERNEL_GLOB` is the Base64-encoded seed binary. The WAT source is documented in comments inside `constants.ts`. The binary imports `env.log` (writes output) and `env.grow_memory`, exports `memory` and `run(ptr, len)`.

### Evolution Engine (utils/wasmEvolution.ts)
`evolveBinary(base64, knownInstructions, attemptSeed)` mutates the code section of a WASM binary using one of four strategies (MODIFY/INSERT/DELETE/ADD) chosen by `attemptSeed % 4`. It generates safe instruction sequences (const-drop, math, local.tee, if-true) and validates the result is ≤ 32 KB.

### WASM Service (services/wasmService.ts)
`WasmKernel.bootDynamic(base64, logCallback, growCallback)` decodes the Base64 binary, instantiates it with `WebAssembly.instantiate`, and wires up `env.log` and `env.grow_memory`. `runDynamic(sourceGlob)` writes the source into WASM memory at offset 0 and calls the exported `run` function.

## Coding Conventions

- **TypeScript strict mode** – always type function parameters and return values.
- **React functional components** with hooks only; no class components.
- **`useCallback`** for all event handlers and callbacks passed as props.
- **`useRef`** for mutable state that must not trigger re-renders (abort flags, refs to intervals, etc.).
- **Tailwind utility classes** for all styling. Use `transition-*`, `animate-*`, and `duration-*` for animations.
- **Log types**: use `'info' | 'success' | 'warning' | 'error' | 'system' | 'mutation'` from `LogEntry`.
- **No external UI libraries** – all components are custom-built.
- **Base64 WASM binaries** are stored as plain strings; decode with `atob()`.

## Build & Run

```bash
npm install           # Install dependencies
npm run dev           # Start Vite dev server (default: http://localhost:5173)
npm run build         # Production build to dist/
npm run preview       # Serve the production build locally
```

There are no automated tests in this project.

## Important Constraints

- The WASM binary must stay structurally valid at all times – always validate mutations with `new WebAssembly.Module(bytes)` before accepting them.
- Do not introduce division opcodes (e.g. `i32.div_s`) in mutations – they can trap on zero.
- The `run` function must maintain a balanced operand stack; all generated instruction sequences should be stack-neutral (push then drop).
- Evolution limits the function body to 32 KB to prevent runaway growth.
- The `KERNEL_GLOB` constant in `constants.ts` is the canonical seed; do not change it unless you also update the WAT comment above it.
