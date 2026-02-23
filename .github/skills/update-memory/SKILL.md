---
name: update-memory
description: Add or refresh repository-specific facts in the agent's memory. Use this skill to keep long-lived metadata up to date after structural or behavioural changes.
---

# update-memory

## Purpose

Record important repository facts—conventions, paths, behaviours, new features—into the agent memory store so future sessions recall them. This acts like a lightweight checklist for persistent knowledge.

## When to run

- After adding or modifying files/directories that agents should remember (e.g., relocating specs, adding new CLI flags).
- After changing coding conventions or adding new test patterns.
- Periodically to summarise recent changes before starting a large task.

## Behaviour

1. Run `git log --stat -n 5` (or examine workspace diffs) to see what’s
   changed since the last invocation of this skill.
2. From the diff/commit messages, pick out lasting, repo‑wide facts such as
   new directories, renamed components, coding conventions, CLI flags or
   build requirements.  Before adding a new entry, run `search-memory`
   to ensure the fact isn't already stored; avoid duplication.
3. For each fact, call the `memory` tool with a concise subject, the fact
   string, proper citations (file paths/commits), and a detailed reason.  A
   good entry is <200 characters and actionable; see other memory entries
   for examples.
4. If a previously stored fact is now invalid, either overwrite it with a
   corrected statement or add a new entry explaining the deprecation.
5. Optionally, invoke `runSubagent` with the `search-memory` agent to verify
   whether similar facts already exist before adding duplicates.

Agents using this skill may also store meta‑facts such as the date of the
last update so cron‑style reminders can be generated.

## Examples

```js
memory({
  subject: "spec path",
  fact: "All spec markdowns live under docs/specs/",
  citations: "docs/specs/*",
  reason: "Helps agents find specification documents when editing features."
});
```

## Notes

- Memory entries should be short (<200 characters) and actionable.
- Avoid storing ephemeral details (like current branch names) or secrets.
- This skill can be invoked by other skills (e.g. update-docs) after major updates.
