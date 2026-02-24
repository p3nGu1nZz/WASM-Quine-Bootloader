---
name: run-app
description: Run the WASM Quine Bootloader executable in either terminal/headless mode or GUI mode. Use this skill to start the application after building it.
---

# run-app

## Purpose

Start the WASM Quine Bootloader executable in either GUI or headless
mode.  This is the primary way to exercise and debug the application once
it has been built.

## Behaviour

- Ensures required externals (`SDL3`/`ImGui`) exist and that the
  binary has been built (`bash scripts/build.sh`).
- Creates `bin/logs` and `bin/seq` directories if absent.
- Prints the full command line, log/telemetry paths, and after a headless
  run shows the exit code plus the last 20 log lines for immediate
  feedback.  Note that the bootloader now defaults to `--telemetry-level
  full` so every session generates comprehensive exports, and you can
  pass `--max-run-ms` to have the application exit automatically after a
  specified interval (internal watchdog avoids crashes on signals).
- `--monitor` option runs the app in the background and tails log files.
- On systems without a display, you can run under Xvfb or set `DISPLAY`
  appropriately; the GUI requires an X/Wayland server.

## Usage

```bash
bash scripts/run.sh            # GUI (default)
bash scripts/run.sh --headless # terminal-only
bash scripts/run.sh --windowed  # force windowed mode
bash scripts/run.sh --monitor   # run + tail logs
```

Run specific build targets by prefixing the command with the directory or
by invoking the binary directly (e.g. `./build/linux-debug/bootloader`).
