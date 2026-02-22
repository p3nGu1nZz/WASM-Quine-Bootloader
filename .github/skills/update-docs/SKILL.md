---
name: update-docs
description: Update the project's documentation files (README, copilot-instructions, specs) to reflect code changes. Use when features or conventions change.
---

# update-docs

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
- [ ] CLI options and default fullscreen behaviour are documented.
- [ ] Build steps in README match `scripts/build.sh`?
- [ ] Public API documented in headers?
- [ ] `copilot-instructions.md` structure updated with new skills and CLI.
- [ ] New constraints recorded?
