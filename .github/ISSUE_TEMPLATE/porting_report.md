---
name: Porting / Migration Report
about: Use this template to report porting, migration, or large refactor findings and actions needed
---

## Short summary

- Summary: (one-line)
- Affected area(s): (e.g., src/, build scripts, CI)
- Priority: (High/Medium/Low)

## Detailed findings

Describe what you examined and what is the problem. Include relevant file paths and short excerpts.

- File: `src/main.cpp` — incorrect SDL initialization check causes immediate exit on success.
- File: `src/log.cpp` — signal handler calls non-async-safe functions (IO) which is unsafe.
- File: `src/` — missing or unimplemented headless terminal mode vs docs.

## Reproduction steps

1. Steps to reproduce the observed behavior
2. Expected result
3. Actual result

## Suggested fixes

- Fix `SDL_Init` check in `src/main.cpp`.
- Replace signal-handler flushing with atomic flag + flush on main loop in `src/log.cpp`.
- Add CLI parsing and headless renderer or align docs.

## Acceptance criteria

- [ ] Critical crash/abort bug in `src/main.cpp` fixed and unit-tested (if feasible).
- [ ] Signal handler no longer performs non-async-signal-safe IO; flush occurs on main thread.
- [ ] README and runtime behavior consistent: `--gui` toggles GUI; default headless mode available or docs updated.
- [ ] New issue template added to `.github/ISSUE_TEMPLATE/porting_report.md`.

## Notes / References

Attach logs, screenshots, and any supporting PRs.
