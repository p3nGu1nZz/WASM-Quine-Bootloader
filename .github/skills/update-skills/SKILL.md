---
name: update-skills
description: Review and refresh the repository's agent skills, copilot-instructions, and related memory entries after making changes. Ensures agents stay in sync with documentation and workspace conventions.
---

# update-skills

## Purpose

When the codebase gains new CLI options, documentation conventions, or any behavioural changes, the suite of Copilot agent skills and the `copilot-instructions.md` file must be kept current. This skill guides you through updating those resources and recording important facts to memory.

## Usage

Invoke this skill after modifying:

- Any existing skill under `.github/skills/`
- The `copilot-instructions.md` file
- CLI behaviour, build/run scripts, or major features

The skill ensures:

1. Each skill's `SKILL.md` accurately describes how to perform its action, including new flags or parameters.
2. The `copilot-instructions.md` overview lists all available skills and outlines their responsibilities.
3. New skills are created for recurring tasks (like `telemetry-review` or this skill itself).
4. Memory entries reflect new conventions or file locations (e.g., `docs/specs/`).

## Checklist

- [ ] Review all files under `.github/skills/` and update descriptions, examples, or add new skills as needed.
- [ ] Add any new skill name to the `Agent Skills` section of `copilot-instructions.md`.
- [ ] Update `copilot-instructions.md` details (CLI flags, logs, build/run instructions) if behaviour changed.
- [ ] Use the `memory` tool to record significant new facts (naming conventions, paths, default behaviours).
- [ ] Run a build/test cycle to ensure new tests or code linked by skills are valid.

## Notes

- This skill is meta‑level; it may be run by agents themselves when they alter the environment.
- Keep the description concise; other skills can be referenced rather than duplicated.
