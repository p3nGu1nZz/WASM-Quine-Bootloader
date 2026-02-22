---
name: find-memory
description: Search the agent's memory store for facts matching a query. Useful for recalling past decisions, conventions, or structures.
---

# find-memory

## Purpose

Run this skill when you need to check whether an important fact about the repository has been stored previously. It helps avoid duplicate memory entries and guides agents by past knowledge.

## Usage

Provide a keyword or short phrase describing what you want to retrieve. The skill returns matching memory entries with their subjects and citations.

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
