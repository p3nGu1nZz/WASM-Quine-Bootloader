Porting review: add issue template and porting report (no code changes)

This PR adds:
- `.github/ISSUE_TEMPLATE/porting_report.md` — new issue template for porting/migration reports
- `docs/PORTING_REPORT.md` — in-repo porting report summary
- `docs/ISSUE_SUMMARY.md` — copy of issue body (for reviewers)

This PR does NOT change any runtime code. It prepares an issue template and documentation to track the required fixes.

Related issue: TODO (will link once issue is created)

Acceptance criteria:
- [ ] The repository contains `.github/ISSUE_TEMPLATE/porting_report.md` for future porting reports
- [ ] A GitHub issue describing the SDL_Init bug, unsafe logger signal handler, and headless mode mismatch is opened and referenced here
- [ ] No production code modified in this PR

