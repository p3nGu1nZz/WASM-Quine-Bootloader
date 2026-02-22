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

1. Review recent commits or changes since last memory update.
2. Identify persistent facts (naming schemes, directory structures, default behaviours, tools available).
3. Use the `memory` tool to store each fact with a topic, citation, and reason.
4. Optionally delete outdated memories by overwriting them with corrected statements.

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
