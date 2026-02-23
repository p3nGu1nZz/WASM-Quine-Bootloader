---
name: search-memory
description: Search the agent's memory store for facts matching a query. Useful for recalling past decisions, conventions, or structures.
---

# search-memory

## Purpose

Query the agent’s memory store for facts matching a keyword or phrase. This
helps recall past decisions, repository conventions, or previously
recorded details.

## Behaviour

- Performs a substring search over all memory entries.
- Returns a list of matches including subject, fact, citations and reason.

## Usage

Ask the agent to run `search-memory` before adding new facts with
`update-memory`, e.g. ``Use search-memory to look up facts about "cli
behaviour" or "spec path".``

## Notes

- Read-only; it never alters memory.
- Useful for preventing duplicate entries when updating memory.
