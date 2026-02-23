---
name: introspect-telemetry
description: Guide agents through inspecting bootloader logs and sequence exports for errors, anomalies, or opportunities for enhancement.
---

# introspect-telemetry

## Purpose

Guide agents through a deeper inspection of bootloader logs and sequence
exports for errors, anomalies, or opportunities for enhancement.  This
skill complements `telemetry-review` when you need more detailed manual
analysis and is valuable before writing bug reports or planning features.

## Behaviour

1. Identify the most recent log (`ls -t build/*/bin/logs/*.log | head -1`) and
   grep for `[error]`, `[warning]` or the special `[telemetry warning]`
   messages that indicate missing or invalid export fields.  Note recurring
   traps, retries, or long pauses.
2. Navigate to the corresponding `seq/<runid>` directory.  Open several
   `gen_<n>.txt` files and verify each contains a correct header, base64
   kernel, hex dump, disassembly, and history log.  Compare adjacent
   generations to see what mutations occurred.
3. Record any telemetry gaps or opportunities (e.g. track mutation counts,
   ABI version, CPU cycles, kernel-size statistics, or new heuristic
   flags) and consider updating `docs/specs/spec_telemetry.md` to keep the
   spec in sync.  Warnings usually mean the spec needs a new field or the
   exporter/reader is out of date.
4. If helpful, diff log files (`diff -u log1 log2`) or convert exports to
   CSV/JSON for external analysis.
5. Cross-reference findings with the `telemetry-review` skill for a quick
   sanity check; use this skill when deeper manual inspection is needed.

## Usage

Invoke after a run to thoroughly investigate problems or when the
telemetry format has changed.  It can be scripted or run interactively.

**Examples**
```bash
log=$(ls -t build/linux-debug/bin/logs/*.log | head -1)
grep -n "\[error\]\|\[warning\]" "$log"
seqdir=$(basename $(dirname "$log"))
head -20 build/linux-debug/bin/seq/$seqdir/gen_1.txt
```
