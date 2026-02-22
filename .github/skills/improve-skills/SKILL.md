---
name: improve-skills
description: Analyze existing skill documents and refine them using conversation context and memory. Compact, clarify, and remove deprecated instructions where appropriate.
---

# improve-skills

Refine the set of `.github/skills/*` documents using conversation context
and memory: merge overlaps, remove outdated guidance, and make each skill
succinct but actionable.

Run after major repo changes or periodically for maintenance.  Steps:
1. List skill dirs.
2. For each, read the current `SKILL.md` and consult memory/chat for
   context.  Merge or cross‑reference overlaps, remove legacy details,
   and rewrite succinctly.  Keep YAML frontmatter correct.
3. Optionally add TODO comments.
4. Save changes and log a memory entry (e.g. "skills refreshed on …").

Notes:
- This skill is meta; you may run it on itself.
- When unsure, mark text as deprecated rather than deleting.
- It may call `search-memory` or other skills as helpers.
