# Command-line Interface – Specification

## Purpose

Define the behaviour of the bootloader executable's command-line arguments. This spec ensures the launcher and the scripts remain in sync, and that automated tests can validate parsing logic.

## Inputs

- `--gui` (default when no flag present) – launch SDL3/ImGui GUI.
- `--headless`, `--no-gui`, `--nogui` – disable GUI and run in headless/terminal mode.
- `--fullscreen` – request a maximized (borderless) window when GUI is enabled; this is *not* exclusive fullscreen.
- `--windowed` – request a windowed window when GUI is enabled.

Unrecognised flags are ignored by `parseCli()`; `scripts/run.sh` may pass additional arguments through to the bootloader instance for future use.

### New Flags (planned)

- `--telemetry-level=<none|basic|full>` – control verbosity of telemetry
  exports; `none` disables files entirely, `basic` writes header+size,
  `full` writes all sections including mutation stats.
- `--telemetry-dir=<path>` – override default `bin/seq` directory for
  export files.
- `--mutation-strategy=<random|blacklist|smart>` – select evolution
  behaviour.  `blacklist` enables the heuristic outlined in
  `docs/specs/spec_heuristics.md`.
- `--heuristic=<none|blacklist>` – shorthand to toggle only the heuristic
  without changing the underlying mutation policy.
- `--profile` – print per-generation timing and memory usage to the log.
- `--max-gen=<n>` – stop the run after `n` successful generations.  Useful
  for automated tests.

Flags that accept values may also be specified as `--flag value`
according to typical Unix conventions.

## Behaviour

1. Parse `argc`/`argv` in `parseCli()`.
2. Default state: `useGui=true`, `fullscreen=true`.
3. Flags may override state; the last flag wins for contradictory options (e.g. `--windowed --fullscreen`).
4. The main loop in `main.cpp` interprets `CliOptions` to decide whether to initialize SDL video or just the timer subsystem, and whether to add `SDL_WINDOW_MAXIMIZED` to window flags (formerly SDL_WINDOW_FULLSCREEN_DESKTOP in SDL2).
5. Headless mode still constructs an `App` object and runs `app.update()` on a fixed 60 Hz timer so the core logic is exercised; no renderer or GUI is created.

## Constraints

- SDL and ImGui dependencies must only be initialised if `useGui` is true, to allow running on CI with no display.
- The `--fullscreen` flag currently maps to `SDL_WINDOW_FULLSCREEN_DESKTOP` which does not change the display mode; it may be changed in future if windowed scaling issues arise.

## Open Questions

- Do we need support for specifying a run-time limit from the CLI, or should callers use external tools (e.g. `timeout`)?

