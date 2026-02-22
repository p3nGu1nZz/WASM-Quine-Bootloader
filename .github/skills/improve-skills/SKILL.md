---
name: improve-skills
description: Analyze existing skill documents and refine them using conversation context and memory. Compact, clarify, and remove deprecated instructions where appropriate.
---

# improve-skills

## Purpose

Refactor the repository's skill set itself.  Parse each `SKILL.md` in
`.github/skills/`, consult the conversation history and memory for relevant
context, then produce polished, concise versions of those skills.  Merge
redundant steps, drop obsolete guidance, and ensure each skill remains
actionable.

## When to run

- After a large series of repository modifications that affect how skills
  should be used.
- Periodically as part of maintenance to keep the skill corpus lean and
  accurate.

## Behaviour

1. Enumerate all skill directories under `.github/skills/`.
2. For each skill:
   - Read the existing `SKILL.md` content.
   - Consult memory entries and recent conversation threads for clues
     about feature changes, new commands, or removed behaviors.
   - If another skill has overlapping instructions, merge them or add a
     cross‑reference instead of duplicating.
   - Remove any references to legacy file paths, flags or conventions that
     no longer apply.
   - Rewrite the document to be succinct while preserving all necessary
     guidance; keep examples minimal and representative.
   - Maintain YAML frontmatter and ensure `name` matches directory.
3. Optionally add comments or TODOs in skill files for items requiring
   future attention.
4. Save updated skill documents; record via `memory` that improvement was
   performed (e.g. "skills were refreshed on <date>").

## Notes

- This skill can itself be improved by running after changes are made by
  other skills—i.e., it is meta iterative.
- When in doubt about removing an instruction, prefer retaining it and
  marking it as "deprecated" rather than deleting outright.
- The skill may leverage other skills (e.g., `find-memory`) to gather
  information about conventions.
