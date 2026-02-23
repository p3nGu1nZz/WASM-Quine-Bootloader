This prompt defines a **development maintenance workflow** that the Copilot
coding agent can invoke when asked to tidy, update or improve the repository
itself.  It chains together a fixed sequence of housekeeping skills rather
than driving feature work or issue-based development.

Use this workflow whenever you want the agent to perform a routine pass on
the project – refreshing documentation, updating its own skills and memory,
and committing any incidental changes.

---

You are the development agent for the **WASM‑Quine‑Bootloader** repository.
Your job is not to implement new features, but to run through a series of
maintenance tasks that keep the project healthy.

Execute the following steps in order, invoking the named skills or tools:

1. **search-memory** – look up any existing facts you might need for the
   subsequent steps or to avoid duplicating work.
2. **improve-skills** – review all current skill documents and refine them
   for clarity, remove redundancies, and ensure they accurately reflect the
   repository’s behaviour.
3. **improve-src** – perform a bulk sweep of the source tree fixing build
   errors, refactoring code, and pruning dead or deprecated constructs.
4. **update-skills** – regenerate the `copilot-instructions.md` file and
   any other skill-related metadata so everything stays in sync with the
   latest `SKILL.md` contents.
5. **commit-push** – if any local files were modified by the previous actions,
   stage them and push a commit (and open a PR if appropriate).
6. **update-docs** – refresh README, specs summaries, README sections, or
   other high‑level documentation to incorporate any recent changes.
7. **update-specs** – open and adjust individual spec documents under
   `docs/specs/` if the behaviour has drifted or new options have been
   added upstream of this workflow.
8. **update-memory** – record any new long‑lived facts about the repository
   that arose during this pass (e.g. new conventions, new tools).

When invoked, the agent should produce clear instructions or calls to the
above skills in sequence.  If any step requires human confirmation (for
example, committing changes or deciding which docs to update), ask the
controller.

---

This workflow is intended for periodic housekeeping rather than feature
development.  It can be triggered manually by referencing
`.github/prompts/dev_workflow.prompt.md` in your prompt to the Copilot
agent.