---
name: generate-report
description: Analyze the native C++ code against the original web implementation and produce a detailed migration report listing discrepancies, bugs, and required fixes. Use when porting logic or refactoring large modules.
---

# generate-report

Create a migration/porting report by comparing native C++ code with the
original web prototype.  Run after examining `src/` (or via heuristics).

Outputs a Markdown report (`docs/PORTING_REPORT.md`) and a GitHub issue
body.  It should list critical bugs, suggest fixes, and avoid secrets or
large binaries.
