---
name: run-app
description: Run the WASM Quine Bootloader executable in either terminal/headless mode or GUI mode. Use this skill to start the application after building it.
---

# run-app

Purpose

Run the bootloader application in whichever mode is required. By default the skill runs in terminal/headless mode; pass `--gui` to open the SDL3/ImGui window.

Usage

- Ensure `external/SDL3` and `external/imgui` are present (use `bash scripts/setup.sh`).
- Build the linux-debug target or a chosen target.
- When reinitializing dependencies, run `bash scripts/setup.sh --clean` to
  wipe and reclone external libs.

Commands

```bash
# build the application if necessary
bash scripts/build.sh linux-debug

# run in terminal/headless mode (default)
bash scripts/run.sh

# run in GUI mode
bash scripts/run.sh --gui

# run and tail the log file (monitor mode)
bash scripts/run.sh --monitor
```

Inputs

- Optional build target.

Outputs

- In terminal mode: the bootloader renders panels to stdout using ANSI escape codes.
- In GUI mode: an SDL3 window opens showing the same panels rendered with ImGui.
- Log files are written to `build/<target>/bin/logs/`; telemetry exports to
  `build/<target>/bin/seq/<runid>/`.

Security / Notes

- May require X11 / Wayland display server access when run on CI runners.
