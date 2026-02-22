---
name: introspect-telemetry
description: Guide agents through inspecting bootloader logs and sequence exports for errors, anomalies, or opportunities for enhancement.
---

# introspect-telemetry

## Purpose

Provide a systematic process for analysing runtime data produced by the
bootloader (`./bin/logs/*` and `./bin/seq/*`).  Useful after a run to
spot failures, unexpected behaviour, or areas where telemetry could be
expanded.

## When to run

- After executing the bootloader for a period (GUI or headless).
- Prior to filing bugs or planning feature improvements based on runtime
  evidence.
- When adding new logging or export fields, to verify they appear correctly.

## Behaviour

1. Locate the most recent log file in `build/<target>/bin/logs/`.
2. Scan for lines tagged `[error]` or `[warning]`, and note timestamps.
3. Check for repeated verification failures or boot retries.
4. Open the newest `seq/<runid>` directory and inspect `gen_<n>.txt` files:
   - Confirm header, kernel size, base64 payload, hex dump.
   - Look for missing sections, malformed data, or unexpected size
     changes.
   - Compare successive generations for mutation patterns.
5. Note any missing telemetry you wish the bootloader to export (e.g. ABI
   version, mutation statistics).
6. Optionally run diff between log files or dump logs to a tool for search.

## Example instructions

```
# prompt the agent:
"Please inspect the latest telemetry run and tell me if any generations
failed verification or if the exported kernel sizes are shrinking."
```

The agent may respond with a summary of anomalies and suggestions for
what additional fields could be logged.

## Notes

- This skill does not modify data; it only provides inspection guidance.
- Use in combination with `telemetry-review` when you need to track trends
  over multiple runs.
