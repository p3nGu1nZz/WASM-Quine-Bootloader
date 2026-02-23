---
name: update-docs
description: Update the project's documentation files (README, copilot-instructions, specs) to reflect code changes. Use when features or conventions change.
---

# update-docs

## Purpose

Refresh the project's documentation (README, copilot-instructions, docs in
`docs/` or `docs/specs/`) whenever code or conventions change. Ensures
users and agents have accurate instructions.

This skill mirrors the contents of `.github/prompts/update-docs.prompt.md`.

## When to run

- After adding new source files or build targets.
- When the README or scripts behaviour changes.
- Whenever coding conventions or constraints are modified.

## What it covers

- README.md structure and prerequisites.
- `.github/copilot-instructions.md` updates.
- Inline comments in headers and source per conventions.
- Script usage comments in `scripts/*.sh`.

## Checklist

- [ ] README prerequisites accurate?
- [ ] README reflects cleanup flags, log/seq directories and coloured output?
- [ ] CLI options (telemetry‑level/dir, mutation‑strategy/heuristic, profile,
      max-gen, etc.) and default fullscreen behaviour are documented.
- [ ] Build steps in README match `scripts/build.sh` and note
      `-Werror` enforcement?
- [ ] Public API documented in headers?
- [ ] `copilot-instructions.md` structure updated with new skills and CLI.
- [ ] Add any new facts to memory (use update-memory skill) and clean outdated entries.
- [ ] New constraints recorded?
