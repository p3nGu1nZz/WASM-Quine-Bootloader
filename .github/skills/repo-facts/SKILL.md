---
name: repo-facts
description: Inspect the repository deeply and produce a comprehensive technical summary including architecture, design, current issues, blockers, and other relevant facts.
---

# repo-facts

This skill is intended to help agents (or subagents) gain a detailed
understanding of the project without performing code changes.  It is
useful when planning large features, onboarding new contributors, or
preparing high-level reports.

## Behaviour

1. Perform a thorough scan of the repository using searches, file reads,
   and other available tools (`grep_search`, `semantic_search`, etc.).
2. Identify key components, their purposes, and how they interrelate (e.g.
   main application logic, GUI layer, WASM mutation engine, tests, scripts,
   docs, build system).
3. Summarise design decisions and architectural patterns, drawing on
   documentation (`docs/`), comments in code, and directory structure.
4. Enumerate the current set of open GitHub issues or blockers, including
   any recurring themes or high-priority tasks.  Use `mcp_github_list_issues`
   if necessary.
5. Highlight any known constraints, conventions, or unusual mechanics that
   a developer should be aware of (e.g. `.tmp` usage, SDL3 requirement,
   mutation strategy constraints).
6. Produce a written report that could be returned directly to the caller
   or fed into subsequent planning prompts.

The skill does **not** modify the repository or create issues; it is purely
informational.

## Usage

Invoke `repo-facts` when an agent needs a comprehensive snapshot of the
project state.  It can be used by higher-level workflows, subagents, or
human queries like “what does this repo do and what does it look like?”.

The output should be a structured summary that covers architecture,
key files, build/run instructions, testing setup, and any outstanding
issues or decision points.