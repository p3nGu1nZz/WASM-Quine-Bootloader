---
name: update-issues
description: Scan open issues and close those already implemented; bring issue descriptions up to date with the current codebase, then create new issues from code-review suggestions.
---

# update-issues

This skill helps the agent keep GitHub issues in sync with the evolving
repository.

## Behaviour

1. List all open issues using `mcp_github_list_issues`.
2. For each issue:
   * Determine whether the work has already been merged or otherwise
     implemented on `main` (e.g. by checking for linked PRs, commit
     messages, or searching for relevant code changes).  Pay special
     attention to issues about warnings or build failures – these may be
     fixed by the new `-Werror` enforcement or by refactors.
   * If the issue is resolved, close it using `mcp_github_issue_write`.
   * If the issue is still relevant but the codebase has changed,
     update the issue body/labels/comments to reflect the current
     state (for example, remove outdated steps or mark the scope).
3. After processing existing issues, run the `code-review` skill to
   generate a fresh batch of candidate tasks.
4. Present the generated `[ISSUE] Title… Body…` suggestions back to the
   controller; when the human selects some, create those issues via
   `mcp_github_create_issue` or `create-issues` skill.

If any modifications to issue bodies or the addition of new issues cause
local documentation or planning notes to change, the agent should also
consider invoking `commit-push` to persist those updates (e.g. when
issue templates or tracking files are edited).

## Usage

Invoke this skill when you want the agent to synchronise the issue
tracker with the repository state—typically at the start of a work
session, before or after running `code-review`, or periodically during
the development cycle.

The skill will not itself close or create issues automatically unless the
controller confirms; instead it will produce instructions or tool
calls that can be executed by the next agent prompt.