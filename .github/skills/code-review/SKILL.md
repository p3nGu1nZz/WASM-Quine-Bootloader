---
name: code-review
description: Perform an adversarial, comprehensive code review and suggest improvements across efficiency, usability, security, performance, and features.
---

# code-review

This skill guides the agent to read the entire repository and carry out an
independent, adversarial code review.  The reviewer should look for
bugs, inefficiencies, fragile abstractions, security risks, missing
error handling, and opportunities for new features or refactors.  The
review should be constructive: document concerns, explain why they
matter, and suggest concrete fixes or enhancements.

## Expectations

1. Scan all source (`src/` and `test/`), scripts (`scripts/*.sh`),
   CMake files and docs under `docs/`.  Use `grep_search`/`semantic_search`
   to locate patterns such as `parseCli`, `autoExport`, `sequenceDir`,
   and to ensure the review touches every major subsystem (util, GUI,
   wasm, training modules, etc.).
2. Examine the unit tests for coverage gaps and duplications; note
   missing tests or fragile constructs that would benefit from mocking
   or additional assertions.
3. Look at the shell helper scripts (`build.sh`, `run.sh`, `test.sh`) –
   they are part of the user experience and can harbour portability or
   usability issues (e.g. temp file handling, error messages).
4. Consider security aspects: unbounded input parsing, `exec` usage,
   directory traversal when reading telemetry, and any use of system
   calls or file I/O that could be misused.
5. Note performance hotspots (frequent base64 decodes, vector
   allocations, GUI rendering), and propose lightweight refactors such as
   caching, preallocation or algorithmic changes.
6. Search for any outdated conventions or comments (e.g. references to
   SDL2) and ensure the review recommendations reflect the current
   architecture (SDL3, `core` library, etc.).
7. Propose code changes or new modules, with reasoning and (if
   appropriate) code snippets.
8. Generate **a list of roughly twelve actionable next steps** that
   translate the review findings into tasks (e.g. "Add argument validation
   to `parseCli`", "Cache decoded kernels in exporter" or "write spec for
   neural matrix serialization").  These will become GitHub issue drafts
   when the user selects them.
9. Where issues are proposed, clearly mark them so they can be
   programmatically converted to actual GitHub issues by subsequent
   agent actions.  Use a simple format like "[ISSUE] Title: ... Body: ...".


## Usage

Invoke the skill when the user wants a thorough audit of the current
codebase and a prioritized set of follow-up actions.  After receiving
suggestions, the agent should ask the user which tasks to form GitHub
issues for, then create them accordingly.

The review should be adversarial: think like a malicious user trying to
break the system or an engineer maintaining code long-term.  The goal is
not to criticise but to make the project more robust and feature-rich.

Task-oriented prompts within this skill should not themselves create
issues; they only propose candidate titles/descriptions.  Another tool
call will handle issue creation once the user has made selections.
