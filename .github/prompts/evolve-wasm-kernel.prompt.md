# Evolve the WASM Kernel

Modify the WASM binary evolution logic in `utils/wasmEvolution.ts` to add a new mutation strategy or genome generator.

## Context

The evolution engine in `evolveBinary()` mutates the **code section** of a WASM binary (Base64-encoded). It locates the code section via a hand-written LEB128 parser, slices out the instruction bytes, applies one of four `EvolutionAction` strategies (MODIFY / INSERT / DELETE / ADD), then reconstructs a valid binary.

A **genome** is a small byte sequence that is stack-neutral (it leaves the operand stack unchanged). Every genome pushes value(s) onto the stack and then discards them with `drop` (opcode `0x1A`).

## Rules for Safe Genomes

- Must be **stack-neutral**: every pushed value must be popped before the sequence ends.
- Do NOT use `i32.div_s` (0x6D) or `i32.div_u` (0x6E) – they trap on zero.
- Do NOT use `i32.rem_s` (0x6F) or `i32.rem_u` (0x70) – they trap on zero.
- Branches (`br`, `br_if`) must stay within a balanced `block`/`if`/`loop`/`end` scope.
- Local indices must be within bounds: the `run` function has two i32 params (indices `0` and `1`) and no declared locals.
- Keep sequences short (≤ 16 bytes) to avoid bloating the binary beyond the 32 KB limit.

## Common Opcode Reference

| Opcode | Mnemonic       | Stack effect          |
|--------|----------------|-----------------------|
| `0x01` | `nop`          | –                     |
| `0x04` | `if (void)`    | pop i32               |
| `0x0B` | `end`          | –                     |
| `0x1A` | `drop`         | pop i32               |
| `0x20` | `local.get n`  | push i32              |
| `0x21` | `local.set n`  | pop i32               |
| `0x22` | `local.tee n`  | pop then push i32     |
| `0x41` | `i32.const v`  | push i32 (LEB128 v)   |
| `0x6A` | `i32.add`      | pop×2, push i32       |
| `0x6B` | `i32.sub`      | pop×2, push i32       |
| `0x6C` | `i32.mul`      | pop×2, push i32       |
| `0x71` | `i32.and`      | pop×2, push i32       |
| `0x72` | `i32.or`       | pop×2, push i32       |
| `0x73` | `i32.xor`      | pop×2, push i32       |

## Instructions

1. Open `utils/wasmEvolution.ts`.
2. Add a new generator function following the pattern of `generateRandomConstDrop`, `generateSafeMath`, `generateLocalTee`, or `generateIfTrue`. Name it `generate<Description>`.
3. Ensure the returned `number[]` is stack-neutral (see rules above).
4. Register the new generator inside `getGenome()` by adding a probability branch (update the existing probability thresholds so they still sum to 1).
5. Add the new opcode(s) to `getOpcodeName()` in `utils/wasmParser.ts` if they are not already present.
6. Update the `describeSequence` helper in `wasmEvolution.ts` if the new opcode needs a custom display format.
7. Validate the mutation by running the app (`npm run dev`) and confirming that evolutions are accepted (no "EVOLUTION REJECTED" log entries caused by the new genome).
