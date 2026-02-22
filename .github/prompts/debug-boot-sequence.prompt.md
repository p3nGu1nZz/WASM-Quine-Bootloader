# Debug Boot Sequence Issues

Diagnose and fix issues in the WASM Quine Bootloader boot sequence that is driven by the state machine in `App.tsx`.

## Boot Sequence Overview

```
IDLE → BOOTING → LOADING_KERNEL → EXECUTING → VERIFYING_QUINE → IDLE  (success)
                                ↘ REPAIRING → IDLE                      (failure)
```

The state machine lives in `App.tsx`. The main loop is `runBootSequence()` (a `useCallback`). State transitions are driven by `useEffect` watching `systemState`.

## Common Failure Modes

### 1. "Output checksum mismatch (Self-Replication Failed)"
**Cause**: The WASM `run` function output does not equal `currentKernel` (Base64 string).  
**Investigate**:
- `handleWasmLog` in `App.tsx` compares `outputString === currentKernel`.
- `runDynamic` in `services/wasmService.ts` writes `sourceGlob` (the Base64 string) into WASM linear memory at offset 0 and calls `run(0, encodedSource.length)`.
- The WASM module must call `env.log(0, len)` where `len` equals `encodedSource.length`.
- Check that `KERNEL_GLOB` in `constants.ts` matches the expected WAT-compiled binary (hex shown in the comment).

### 2. "Execution Timeout: System Hung"
**Cause**: The WASM `run` export was never called (no `call` opcode detected during the instruction walk), or the call never triggered `env.log`.  
**Investigate**:
- The 10-second watchdog is set in `runBootSequence` just before the instruction loop.
- Check `parsedInstructions` – if it is empty the code falls back to "Blind Run".
- Verify `extractCodeSection` in `utils/wasmParser.ts` can parse the current kernel binary.
- A mutation may have removed the `call` instruction (opcode `0x10`). In this case `evolveBinary` should be called with the *stable* kernel to regenerate.

### 3. "EVOLUTION REJECTED: ..."
**Cause**: `evolveBinary` produced a structurally invalid WASM binary.  
**Investigate**:
- The rejection is caught in `handleWasmLog` with `new WebAssembly.Module(bytes)`.
- Check `utils/wasmEvolution.ts` – the mutation may have produced an unbalanced stack.
- Look for recently added genome generators; verify they are stack-neutral.
- Check the LEB128 reconstruction (`encodeLEB128` / `decodeLEB128` in `utils/wasmParser.ts`).

### 4. Infinite REPAIRING loop
**Cause**: Every repaired kernel also fails verification, so `retryCount` keeps incrementing.  
**Investigate**:
- `handleBootFailure` calls `evolveBinary(stableKernel, ...)` – the stable kernel is the *last verified* binary. If the stable kernel is somehow invalid, the app is stuck.
- Reset the stable kernel to `KERNEL_GLOB` (from `constants.ts`) by refreshing the page.
- Check that `setStableKernel(currentKernel)` is only called on verified success inside `handleWasmLog`.

## Debugging Tools

- Open the browser DevTools **Console** – WASM errors (trap, compile error) are printed there.
- The **System Log** panel in the UI shows all `addLog` entries; filter by colour (red = error, yellow = warning, purple = mutation).
- The **Kernel Source** panel (right column) highlights mutated bytes in yellow and expanded bytes in green.
- Add a temporary `console.log` inside `handleWasmLog` to print `outputString` and `currentKernel` side by side.
- Use `wasmKernel.bootDynamic(kernel, ...)` directly in the browser console after importing the module.

## Checklist for a Fix

- [ ] Identify which state the system gets stuck in (check `systemState` via React DevTools).
- [ ] Reproduce the failure in the browser console by calling `evolveBinary` with the failing kernel.
- [ ] Validate the repaired binary: `new WebAssembly.Module(Uint8Array.from(atob(binary), c => c.charCodeAt(0)))`.
- [ ] Ensure the fix does not break the success path (quine verification still passes).
- [ ] Run `npm run build` to confirm no TypeScript or Vite errors.
