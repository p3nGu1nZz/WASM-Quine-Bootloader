---
name: improve-skills
description: Analyze existing skill documents and refine them using conversation context and memory. Compact, clarify, and remove deprecated instructions where appropriate.
---

# improve-skills

Refine the set of `.github/skills/*` documents using conversation context
and memory: merge overlaps, remove outdated guidance, and make each skill
succinct but actionable.

Run after major repo changes or periodically for maintenance.  For
example, when you add features like kernel replay, telemetry metrics,
heuristic flags, or new CLI parameters, the skill docs should be updated
accordingly.  Steps:
1. List skill directories and identify candidates for consolidation.
2. For each, read the current `SKILL.md`, then query the memory store or the
   chat history for relevant contextual notes.  Update or rephrase
   procedures to reflect real usage patterns.  Also verify that the
   skill name appears in `copilot-instructions.md` and that any new skill
   has been added there.
3. Add concrete examples or shell snippets where they would help agents
   execute the skill (e.g. build/test commands, grep patterns, replay
   kernels from logs).
4. Remove any obsolete steps or references (e.g., old build flags) and add
   TODO comments if future work is anticipated.
5. Save changes, commit them (see `commit-push` skill), and optionally
   record a memory entry such as "skills refreshed on 2026-02-22 with
   metrics/heuristic guidance".  Updating memory helps future agents
   understand the current set of skills.

**Notes**
- This skill is meta; you may run it on itself.
- When unsure, mark text as deprecated rather than deleting outright.
- It can call `search-memory` or other skills during the review to avoid
  duplicating knowledge.
