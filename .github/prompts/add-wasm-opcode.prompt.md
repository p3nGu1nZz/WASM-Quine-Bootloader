# Add a New WASM Opcode

Extend the WASM parser and evolution engine to recognise and generate a new WebAssembly opcode.

## Files to Modify

| File | Change |
|------|--------|
| `utils/wasmParser.ts` | Add the opcode to `getOpcodeName()` and handle its operand encoding in `parseInstructions()` |
| `utils/wasmEvolution.ts` | Optionally add a genome generator that uses the new opcode |

## Step-by-Step Instructions

### 1. Add the opcode name (`utils/wasmParser.ts`)

Open `getOpcodeName()` and add a `case` for the new opcode byte:

```ts
case 0xNN: return "opcode.name";
```

### 2. Handle operand parsing (`utils/wasmParser.ts`)

Inside `parseInstructions()`, the `if/else if` chain after `const opcode = bytes[ptr]` decides how many operand bytes to consume. Add a branch for your opcode:

- **No operands** (e.g., `drop`, `i32.add`): no change needed – the default `len = 1` is correct.
- **One LEB128 operand** (e.g., `local.get`, `br`, `call`): add the opcode to the existing `else if` that calls `decodeLEB128`.
- **Two LEB128 operands** (e.g., `memory.init`): add a new `else if` that calls `decodeLEB128` twice.

Example – adding `br` (opcode `0x0C`, one LEB128 label index):

```ts
} else if (opcode === 0x0C) { // br
    const leb = decodeLEB128(bytes, ptr + 1);
    len += leb.length;
    args = Array.from(bytes.slice(ptr + 1, ptr + len));
}
```

### 3. Add a genome generator (`utils/wasmEvolution.ts`) – optional

Only add a generator if the new opcode can appear in a **stack-neutral** sequence. Follow the pattern of existing generators:

```ts
const generateMyOpcode = (): number[] => {
    // Must be stack-neutral: every push must be matched by a drop.
    return [/* opcode bytes */];
};
```

Register it inside `getGenome()` with an appropriate probability (update existing thresholds so they still sum to 1).

### 4. Update `describeSequence` if needed

If the opcode has a custom display format (e.g., it takes a named argument), add a branch inside the `map` callback in `describeSequence`:

```ts
if (i.opcode === 0xNN && i.args.length > 0) return `${name} ${i.args[0]}`;
```

### 5. Validate

1. Run `npm run dev` and open the app.
2. Confirm no TypeScript errors in the terminal.
3. If you added a genome generator, wait for several boot cycles and confirm no "EVOLUTION REJECTED" entries in the System Log.
4. Run `npm run build` to confirm the production build is clean.

## Reference: Opcode Encoding Patterns

| Pattern | Example opcodes | Operand encoding |
|---------|-----------------|------------------|
| No operand | `nop`, `drop`, `i32.add` | none |
| Single LEB128 | `local.get`, `local.set`, `local.tee`, `call`, `br` | 1× unsigned LEB128 |
| Two LEB128 | `call_indirect` | 2× unsigned LEB128 |
| Block type byte | `if`, `block`, `loop` | 1 byte (type tag, e.g., `0x40` = void) |
