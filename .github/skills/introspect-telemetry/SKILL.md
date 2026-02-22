---
name: introspect-telemetry
description: Guide agents through inspecting bootloader logs and sequence exports for errors, anomalies, or opportunities for enhancement.
---

# introspect-telemetry

Use after running the bootloader (GUI or headless) to analyse
`./bin/logs/*` and `./bin/seq/*` for errors, anomalies or missing data.
Useful before bug reports, feature planning, or when adding new fields.

Behaviour:
1. Identify the most recent log (`ls -t build/*/bin/logs/*.log | head -1`) and
   grep for `[error]`/`[warning]` or unexpected state transitions.  Note
   recurring traps or long pauses.
2. Navigate to the corresponding `seq/<runid>` directory.  Open several
   `gen_<n>.txt` files and verify each contains a correct header, base64
   kernel, hex dump, disassembly, and history log.  Compare adjacent
   generations to see what mutations occurred.
3. Record any telemetry gaps or opportunities (e.g. track mutation counts,
   ABI version, CPU cycles, or kernel-size statistics) and consider
   updating `docs/specs/spec_telemetry.md`.
4. If helpful, diff log files (`diff -u log1 log2`) or convert exports to
   CSV/JSON for external analysis.
5. Cross-reference findings with the `telemetry-review` skill for a quick
   sanity check; use this skill when deeper manual inspection is needed.

**Examples**
```bash
log=$(ls -t build/linux-debug/bin/logs/*.log | head -1)
grep -n "\[error\]\|\[warning\]" "$log"
seqdir=$(basename $(dirname "$log"))
head -20 build/linux-debug/bin/seq/$seqdir/gen_1.txt
```
