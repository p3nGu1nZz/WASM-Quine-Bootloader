---
name: commit-push
description: Stage local changes, create a descriptive commit message, and push to the remote repository. Use this skill to record work after making modifications.
---

# commit-push

## Purpose

Stage local changes, build a commit message, and push the files to GitHub
using the available `mcp_github_push_files` (or equivalent `gh` CLI) tool.  
Agents should perform the push themselves rather than deferring to a user.

## Usage

- Invoke after making any local modifications that you want recorded in
  version control and visible on GitHub.
- Provide a succinct but descriptive commit message; the agent will use
  it verbatim with the MCP push tool.  Mention issue numbers or
  relevant subsystems when appropriate.
- The skill may also be run automatically by higher‑level workflows like
  `update-docs` or `improve-src` when they modify files.

## Behaviour

1. Run `git status` (or equivalent library call) to determine which files
   are new/modified/deleted and make sure the working tree is clean after
   staging.
2. Read each file’s contents from disk and build a list of
   `{path, content}` objects; for deletions, include the path with an
   empty string and later remove it via git.
3. Attempt to call `mcp_github_push_files` using the current branch and
   commit message.  Include every changed file so the remote state matches
   the workspace.
4. If the push fails because the local branch is behind the remote, run
   `git pull --rebase` (or perform an equivalent merge) and retry the
   push.  Agents may also fall back to shell git commands in cases where
   `push_files` is unavailable or unsuitable.  On merge conflicts the
   agent should either resolve trivial whitespace/style clashes or report
   the conflicting paths for manual intervention.  Do **not** force‑push
   unless explicitly directed by the user.
5. When the branch is not the repository default, create a pull request
   using either `gh pr create` or `mcp_github_create_pull_request`.  The
   commit description should be clear and reference any related issue or
   planning note.  Prefer **squash** or **rebase‑merge** methods for
   consolidating iterative commits unless the user asks for a merge
   commit.
6. If merge conflicts occur during the rebase or pull step, report the
   conflicting paths back to the user and pause for manual intervention,
   or attempt to auto-resolve trivial conflicts (e.g. whitespace).
7. After a successful push (and optional PR creation), log the URL of the
   remote commit/PR so the user can review it.

## Example

```
# prompt to agent:
"I added new skills and updated docs; commit and push the changes."
```

The agent would then perform or output something like:
```js
// gather changed paths from `git status` and read files
const changes = [
  { path: "README.md", content: "...updated content..." },
  { path: "src/cli.cpp", content: "..." },
  // etc.
];
await mcp_github_push_files({
  branch: currentBranch,
  files: changes,
  message: "docs: update skills list and README"
});
// if push fails due to remote > local, perform git pull --rebase
// then retry, or use `git push` as fallback
```