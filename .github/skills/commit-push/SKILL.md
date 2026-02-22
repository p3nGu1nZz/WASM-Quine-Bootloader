---
name: commit-push
description: Stage local changes, create a descriptive commit message, and push to the remote repository. Use this skill to record work after making modifications.
---

# commit-push

## Purpose

Automate the common sequence of git commands used to record and upload
changes to the source repository.  Although agents cannot run git directly,
they can generate an appropriate commit message and instruct a human or
another process to execute the commands.

## Usage

- Invoke after making code/doc edits you intend to keep.
- Provide a concise but descriptive commit message summarising the work.

## Behaviour

1. List unstaged changes (`git status --short`).
2. Stage all modified/added files: `git add -A` (or allow specifying subset).
3. Create a commit: `git commit -m "<message>"`.
4. Push to the current branch (`git push`).

## Example

```
# prompt to agent:
"I updated the README and added a new skill.  Please commit and push."
```

The agent may respond with:
```
git add -A
git commit -m "docs: update README and add commit-push skill"
git push
```

## Notes

- This skill does not run git itself; it produces the commands a user should
  execute.  In CI contexts the commands could be executed automatically.
- Follow the project's commit message conventions (e.g. prefix with `docs:`,
  `feat:`, `fix:` as appropriate).
