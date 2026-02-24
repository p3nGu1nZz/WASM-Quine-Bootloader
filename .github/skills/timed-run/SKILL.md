---
name: timed-run
description: Launch the bootloader for a specified duration and automatically perform a telemetry review.
---

# timed-run

## Purpose

Provide a convenient one‑step workflow for experiments where the agent
needs to run the bootloader for a fixed interval, gather logs and
evolution telemetry, and immediately inspect the results.  This is handy
for frequent test cycles or when evaluating changes that may cause
crashes or abnormal behaviour.

## Behaviour

1. Build the project if the binary is missing (invokes `bash scripts/build.sh`).
2. Run the bootloader with `--max-run-ms=<duration_ms>` (defaulting to
   60 s if no duration is provided) and any additional CLI arguments the
   caller specifies.  The run uses the GUI by default; the window will
   start fullscreen unless `--windowed` is passed.
3. Wait for the bootloader to exit automatically when the timer expires or
   another shutdown condition occurs (max-gen, user keypress, signal,
   etc.).  Because the built‑in run‑timeout is used, the process shuts
   down cleanly without a segmentation fault when supervised by external
   tools.
4. After the session completes, invoke the `telemetry-review` skill on
   the newly created `bin/seq/<runid>` directory to highlight any
   anomalies or noteworthy events.  If `--telemetry-level` was not
   specified, the level defaults to `full` so the review has ample data.

## Usage

Tell the agent the duration in seconds and any extra bootloader options:

```
run-timed 60 --telemetry-level full --profile
```

The agent will convert the seconds to milliseconds, run the bootloader,
and then perform the usual log/telemetry analysis described in the
`telemetry-review` skill.

If you prefer to specify an absolute ms timeout yourself, provide the
`--max-run-ms` argument directly; the agent will not modify it.

## Notes

- This skill builds upon and extends the existing `run-app` and
  `telemetry-review` skills; you can still call those individually if you
  only need one of the actions.
- The duration argument is optional; omitting it will still trigger a
  60‑second session by default.
- The skill assumes the working directory is the repository root and that
  the `build` directory exists or can be created.
