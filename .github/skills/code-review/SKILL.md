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

1. Scan all source, tests, scripts, and build files in the workspace.
2. Identify areas where the code could be more efficient, simpler, more
   secure, or more user-friendly.
3. Propose code changes or new modules, with reasoning and (if
   appropriate) code snippets.
4. Generate **a list of roughly twelve actionable next steps** that
   translate the review findings into tasks (e.g. "Add argument validation
   to `parseCli`", "Cache decoded kernels in exporter").  These will
   become GitHub issue drafts when the user selects them.
5. Where issues are proposed, clearly mark them so they can be
   programmatically converted to actual GitHub issues by subsequent
   agent actions.

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
