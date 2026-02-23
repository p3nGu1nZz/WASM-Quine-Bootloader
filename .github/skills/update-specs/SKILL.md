---
name: update-specs
description: Edit or add design specification documents in `docs/` to match current implementation or planned features. Use when behaviour of a component changes.
---

# update-specs

## Purpose

Edit or add design specification documents to reflect current or planned
behaviour.  Specs form the authoritative reference for components such as
CLI, telemetry, GUI, and the WASM kernel.

This skill corresponds to the instructions in `.github/prompts/update-specs.prompt.md`.

## Spec locations

Most specs live under `docs/` or `docs/specs/` (CLI, telemetry, GUI, wasm kernel, evolution, terminal renderer).

## Instructions

1. Open the appropriate `docs/spec_*.md` file.  If no spec exists for the
   component, create one under `docs/specs/` using the standard file
   format below.
2. Update the **Behaviour** section for the changed feature, and add
   examples or sample exports when applicable (especially for telemetry
   or CLI behaviour).
3. Add or resolve `## Open Questions` entries as needed; mark any
   non-obvious design decisions for later review.
4. Add a comment in the code pointing to the spec (e.g. `// Spec:
   docs/specs/spec_cli.md`) so future readers can locate it quickly.
5. Run `bash scripts/test.sh` to verify nothing broke.

## File format reminder

Spec files use the format:

```markdown
# Component – Specification

## Purpose

## Inputs

## Outputs / Side Effects

## Behaviour

## Constraints

## Open Questions
```

Refer to the prompt file for more details.
