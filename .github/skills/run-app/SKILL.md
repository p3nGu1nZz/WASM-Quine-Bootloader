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
telemetry files into the latter.  Requires an X/Wayland display to open the
window.
