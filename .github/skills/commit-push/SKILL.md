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

- Run whenever you've made local changes that should be committed and
  uploaded.
- Supply a succinct commit message; the agent will use it with the MCP
  push tool.

## Behaviour

1. Identify modified/added files in the workspace.
2. Prepare a list of those file paths and their updated contents (read
   from the filesystem).
3. Call the `mcp_github_push_files` tool with the branch name, array of
   `{path,content}` pairs, and the commit message.
4. Optionally create a pull request using `gh pr create` or the
   corresponding MCP tool if the branch is not the default.

## Example

```
# prompt to agent:
"I added new skills and updated docs; commit and push the changes."
```

The agent would then perform or output something like:
```js
await mcp_github_push_files({
  branch: "main",
  files: [
    { path: "README.md", content: "...updated content..." },
    { path: "src/cli.cpp", content: "..." },
    // etc.
  ],
  message: "docs: update skills list and README"
});
```

## Notes

- This skill does not run git itself; it produces the commands a user should
  execute.  In CI contexts the commands could be executed automatically.
- Follow the project's commit message conventions (e.g. prefix with `docs:`,
  `feat:`, `fix:` as appropriate).
