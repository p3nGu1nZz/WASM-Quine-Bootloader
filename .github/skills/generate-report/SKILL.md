---
name: generate-report
description: Analyze the native C++ code against the original web implementation and produce a detailed migration report listing discrepancies, bugs, and required fixes. Use when porting logic or refactoring large modules.
---

# generate-report

Purpose

Produce a concise porting report summarizing missing features, bugs, and suggested fixes when migrating implementations (e.g., web → native).

Usage

- Run after reviewing `src/` and `web/` code, or use automated heuristics to detect mismatches.

Outputs

- A Markdown report in `docs/PORTING_REPORT.md` and an issue body ready in `docs/ISSUE_BODY_FOR_GITHUB.md`.

Acceptance Criteria

- Lists critical bugs to fix before merge.
- Includes suggested fixes and acceptance criteria for each item.

Security / Notes

- Do not include secrets or large binary dumps in the report.
