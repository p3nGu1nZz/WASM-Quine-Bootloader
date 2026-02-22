---
name: introspect-telemetry
description: Guide agents through inspecting bootloader logs and sequence exports for errors, anomalies, or opportunities for enhancement.
---

# introspect-telemetry

Use after running the bootloader (GUI or headless) to analyse
`./bin/logs/*` and `./bin/seq/*` for errors, anomalies or missing data.
Useful before bug reports, feature planning, or when adding new fields.

Behaviour:
1. Pick latest log file; scan for `[error]`/`[warning]`, repeated failures, or retries.
2. Examine newest `seq/<runid>/gen_<n>.txt`: verify header, size, base64, hex
dump; note missing sections or odd size changes; compare generations for mutation patterns.
3. Jot down telemetry you’d like added (e.g. mutation stats, ABI version).
4. Optionally diff logs or export to a tool.

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
