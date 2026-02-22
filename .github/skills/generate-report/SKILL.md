---
name: generate-report
description: Analyze the native C++ code against the original web implementation and produce a detailed migration report listing discrepancies, bugs, and required fixes. Use when porting logic or refactoring large modules.
---

# generate-report

Create a migration/porting report by comparing native C++ code with the
original web prototype.  Run after examining `src/` (or via heuristics).

Steps:
1. Identify modules whose logic was ported from the TypeScript reference
   implementation (e.g. `wasm/parser`, `evolution`, GUI helpers).
2. Review the corresponding web code (in repository history or external
   archive) and note any behavioural differences, missing features, or
   bugs that were fixed during porting.
3. Draft a Markdown document under `docs/PORTING_REPORT.md` summarising
   each discrepancy, its impact, and possible remediation.
4. Optionally output a GitHub issue template that developers can fill in
   and submit, referencing the files involved.

Outputs a Markdown report and a suggested GitHub issue body.  It should
list critical bugs, suggest fixes, and avoid secrets or large binaries.

**Example**
```markdown
### WASM parser
- Original JS version accepted unvalidated section lengths; C++ parser
  added bounds checks.  Ensure tests cover large random inputs.

### GUI
- Web app used `<canvas>`; native code uses SDL3 heatmap.  behaviour is
  equivalent but layout differs; no action needed.
```