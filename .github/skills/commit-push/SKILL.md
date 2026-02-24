---
name: commit-push
description: Stage local changes, create a descriptive commit message, and push to the remote repository. Use this skill to record work after making modifications.
---

# commit-push

## Purpose

Create a Git commit containing **all local changes** and push it to
`origin`.  This skill is intentionally simple: it only invokes the
standard `git` commands, leaving pull request creation and other GitHub
operations to the caller or subsequent skills.

## Usage

- Run after making edits you want tracked in version control.
- Provide a clear and descriptive commit message; the skill will pass
  this string directly to `git commit -m`.
- Higher‑level workflows like `update-docs` or `improve-src` can invoke
  this skill automatically once they’ve modified the workspace.

## Behaviour

1. Change to the repository root and stage everything:
   ```sh
   git add .
   ```
2. Create a new commit:
   ```sh
   git commit -m "<user-supplied message>"
   ```
   If there are no staged changes, the skill reports that there is
   nothing to commit and exits successfully.
3. Push the current branch:
   ```sh
   git push
   ```
4. If the push is rejected because the local branch is behind the
   remote:
   ```sh
   git fetch origin
   git rebase origin/$(git rev-parse --abbrev-ref HEAD)
   git push
   ```
   - Attempt to auto-resolve trivial whitespace/style conflicts.
   - If conflicts remain, report the offending files and pause for
     manual resolution.  Do **not** force push.
5. Surface any errors from the above commands so the controller can
   decide how to proceed further.

## Notes

- This skill does **not** create pull requests or interact with GitHub
  APIs; those actions should be handled by separate skills or manual
  steps.
- It relies solely on the `git` command‑line tool; no MCP helpers are used.

## Example

```
# Controller prompt:
"commit and push the recent telemetry path fix"
```

Shell actions executed by the agent:
```sh
git add .
git commit -m "fix: derive bin/logs and bin/seq from executable location; avoid root bin creation"
git push
```
If the push is rejected, the agent will rebase and retry automatically.
