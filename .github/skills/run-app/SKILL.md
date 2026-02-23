---
name: run-app
description: Run the WASM Quine Bootloader executable in either terminal/headless mode or GUI mode. Use this skill to start the application after building it.
---

# run-app

Launch the bootloader in GUI or headless mode (default GUI).  Ensure
`external/SDL3` and `external/imgui` exist (`bash scripts/setup.sh`) and
build first (`bash scripts/build.sh [target]`).

`bash scripts/run.sh [--headless|--windowed]`  (GUI by default)
`bash scripts/run.sh --monitor`  tails logs.

The wrapper ensures `bin/logs` and `bin/seq` exist; logs go in the former and
telemetry files into the latter.  It also prints the exact command line and
paths being used, and after a headless run reports the exit code and shows
the last 20 lines of any log files so you can quickly confirm behaviour
without manually opening them.  Requires an X/Wayland display to open the
window; for CI or machines without a display, run under Xvfb or set
`DISPLAY` appropriately when invoking `scripts/run.sh`.
