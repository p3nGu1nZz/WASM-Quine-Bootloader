Porting: review findings — SDL_Init bug, unsafe signal handler, missing headless mode

Overview
- Reviewed the C++ `src/` files and compared to original `web/` reference. Port is mostly correct, but there are critical problems:

Critical issues
1. `src/main.cpp` — incorrect SDL initialization check treats success as failure (should check `SDL_Init(...) != 0`).
2. `src/logger.cpp` — signal handler calls non-async-signal-safe IO (flush uses iostream): unsafe.
3. Docs vs implementation mismatch: README documents headless/terminal mode and `--gui`; `src/main.cpp` doesn't parse arguments and always runs GUI.

Suggested fixes
- Fix `SDL_Init` check in `src/main.cpp`.
- Replace signal handler flush with atomic/sig_atomic_t flag; flush on main loop.
- Add CLI argument parsing and headless renderer, or update docs.

Acceptance criteria
- [ ] `SDL_Init` check corrected and verified by running the app.
- [ ] Signal handler no longer performs non-async-signal-safe IO; flush occurs on main thread.
- [ ] CLI parsing added with `--gui` option or docs updated to match implementation.
- [ ] New issue template added to `.github/ISSUE_TEMPLATE/porting_report.md`.

Notes
- No code fixes are included in this PR. Branch contains only templates and documentation.
