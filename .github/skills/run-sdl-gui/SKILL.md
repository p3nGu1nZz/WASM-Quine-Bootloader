---
name: run-sdl-gui
description: Build (if necessary) and launch the bootloader application in SDL3/ImGui GUI mode on a display-capable system. Useful for visually inspecting the simulation and verifying rendering.
---

# run-sdl-gui

Purpose

Build and launch the graphical SDL3 + ImGui application.

Usage

- Ensure `external/SDL3` and `external/imgui` are present (use `bash scripts/setup.sh`).
- Build the linux-debug target or a chosen target.

Commands

```bash
bash scripts/build.sh linux-debug
bash scripts/run.sh --gui
```

Inputs

- Optional build target.

Outputs

- Launches the SDL3 window displaying the bootloader UI.

Security / Notes

- May require X11 / Wayland display server access when run on CI runners.
