---
name: telemetry-review
description: Analyse the runtime logs and telemetry exports produced by the bootloader to spot anomalies, performance issues, or evolution trends. Useful for understanding how the WASM kernel is mutating and when failures occur.
---

# telemetry-review

Inspect bootloader outputs in `build/<target>/bin/logs/` and
`bin/seq/<runid>/` after running the app to:
- check quine/evolution correctness
- spot errors, retries or shrinking kernel sizes
- verify generation reports (gen_<n>.txt) include base64 dumps and fields
- detect missing telemetry or parse failures

Typical steps: run for a while, open latest log searching `[error]`/`[warning]`,
list latest seq directory, `head` the first gen file, and note anomalies.  
Optionally summarize or file an issue.

## Notes

- The telemetry export format is defined in `App::exportHistory()` and may change; keep specs in `docs/specs/spec_telemetry.md` updated accordingly.
- When adding new telemetry fields, run this skill to ensure the data is serialized properly.

