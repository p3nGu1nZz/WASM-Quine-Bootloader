---
name: find-memory
description: Search the agent's memory store for facts matching a query. Useful for recalling past decisions, conventions, or structures.
---

# find-memory

Search stored memory entries by keyword or phrase to recall past facts and
avoid duplication.  The skill returns subjects, facts, citations, and reasons.

Example prompt:
```
Use find-memory to look up facts about "cli behaviour" or "spec path".
```

## Behaviour

- The skill performs a substring search over existing memory entries.
- Returns the subject, fact, citations, and reason for each match.

## Notes

- This skill does not modify memory; it is read-only.
- Use it before running update-memory to avoid redundant facts.
