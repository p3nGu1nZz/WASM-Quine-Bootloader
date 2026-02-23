---
name: telemetry-review
description: Analyse the runtime logs and telemetry exports produced by the bootloader to spot anomalies, performance issues, or evolution trends. Useful for understanding how the WASM kernel is mutating and when failures occur.
---

# telemetry-review

## Purpose

Analyse runtime logs and telemetry exports produced by the bootloader to
spot anomalies, performance issues, or evolution trends.  It is useful for
understanding how the WASM kernel is mutating and when failures occur.

## Behaviour

Inspect bootloader outputs under `build/<target>/bin/logs/` and
`build/<target>/bin/seq/<runid>/` after executing the app.  The goal is to:

- validate quine/evolution cycles and kernel integrity across generations
- spot `[error]`, `[warning]` or `[telemetry warning]` lines, retries, or
  unexpected kernel size changes
- watch for new advisor/exporter messages about file I/O or empty decodes
- ensure each `gen_<n>.txt` export contains its header, base64 blob, hex dump,
  disassembly section and history entries
- detect telemetry omissions, serialization glitches, parse failures, or
  missing fields indicated by telemetry warnings

## Usage

Run this skill after a bootloader session (GUI or headless).  Provide the
paths to the log/seq directories or let the agent discover them.  If
issues are found, update the telemetry spec or create an issue with sample
exports.
- collect data points useful for performance or mutation analysis

**Workflow**

1. Launch the bootloader (headless or GUI) for a representative period.
2. Use `ls -t` to identify the newest log file, then `grep` for errors and
   count generation cycles.
3. Navigate into the corresponding `seq/<runid>` folder and inspect the
   first few `gen_*.txt` files with `head`/`grep`.
4. If anomalies are found, note them, update `docs/specs/spec_telemetry.md`,
   or open a GitHub issue with sample exports attached.
5. Optionally write a small script or use the data in analysis tools (Python,
   CSV) to visualize mutation trends.

**Examples**

```bash
# find new log and look for problems
latest=$(ls -t build/*/bin/logs/*.log | head -1)
grep -n "\[error\]\|\[warning\]" "$latest"
# inspect first generation export
head -20 build/linux-debug/bin/seq/$(ls -1 build/linux-debug/bin/seq | tail -1)/gen_1.txt
```

**Notes**

- The telemetry export format is defined in `App::exportHistory()` and may
  change; keep specs in `docs/specs/spec_telemetry.md` updated accordingly.
- When adding new telemetry fields, run this skill to ensure the data is
  serialized properly and the history exports remain parseable.
- Compliment `introspect-telemetry` for a quicker review or when you
  need scriptable checks.
- Consider coupling this skill with `introspect-telemetry` for a deeper
  diagnostic session.


