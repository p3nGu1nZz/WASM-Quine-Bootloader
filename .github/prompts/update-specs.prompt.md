# Update Specs / Design Specifications

Update or add design specifications that describe how a feature of the WASM Quine Bootloader should behave before or after implementation.

## What Are Specs?

Specs are plain-text or Markdown files that describe the *intended* behaviour of a component before the code is written (or to capture what was built). They live alongside the code they describe and are checked into the repository.

## Spec File Locations

| Component | Spec file |
|-----------|-----------|
| WASM kernel boot & quine verification | `docs/spec_wasm_kernel.md` |
| Evolution / mutation engine | `docs/spec_evolution.md` |
| Terminal rendering (headless mode) | `docs/spec_terminal_renderer.md` |
| SDL3 GUI window | `docs/spec_gui.md` |
| CLI argument parsing | `docs/spec_cli.md` |

Create the `docs/` directory if it does not yet exist.

## Spec File Structure

Each spec file should contain:

```markdown
# <Component Name> – Specification

## Purpose
One paragraph: what problem this component solves and why it exists.

## Inputs
List of all inputs (function arguments, CLI flags, environment variables, files).

## Outputs / Side Effects
What the component produces or changes.

## Behaviour
Numbered steps describing exactly what the component does, in order.
Include error conditions and how they are handled.

## Constraints
Any hard limits (e.g. max binary size, allowed opcodes, stack invariants).

## Open Questions
Unresolved design decisions (remove entries once decided).
```

## How to Update an Existing Spec

1. Open the relevant `docs/spec_*.md` file.
2. Update the section(s) affected by the change.
3. Mark resolved open questions by moving them to a `## Decisions` section with the date and rationale.
4. Run `bash scripts/test.sh` to confirm the implementation still matches the spec.

## Linking Specs to Code

Add a one-line comment at the top of the relevant `.cpp` file:

```cpp
// Spec: docs/spec_wasm_kernel.md
```

This makes it easy to find the spec from the source and vice versa.
