---
name: improve-src
description: Perform bulk code improvements such as fixing compile errors, refactoring, pruning dead code, and eliminating legacy constructs.
---

# improve-src

## Purpose

Perform a bulk sweep of the source tree to fix compile errors, refactor
code, and prune dead or deprecated constructs.  This helps keep the
C++/CMake codebase clean and warning-free.

## Usage

Run during a development maintenance pass or via the `dev_workflow` when
no immediate feature work is underway.  The agent should describe any
changes made and optionally commit them in logical units.

## Behaviour

1. Scan the `src/` and `test/` directories for compilation or lint
   errors by running the build (`build-app`) and observing failures.  Since
   `-Werror` is enabled, warnings will appear as errors; address them at the
   same time as other fixes.
2. Fix obvious issues automatically: missing includes, naming
   inconsistencies, unused variables, and simple logic bugs that are
   clearly wrong (e.g. off-by-one loops, null pointer checks).
3. Refactor code for clarity and maintainability: split large functions,
   remove duplicated logic, rework complex conditionals, and update
   comments to reflect current behaviour.
4. Identify and remove deprecated or dead code paths.  This includes
   code guarded by obsolete `#if`/`#ifdef`, outdated comments, or
   functions never called from anywhere.
5. Where appropriate, update tests in `test/` to match refactored code
   or to add coverage for newly cleaned-up sections.
6. After modifications, rerun `build-app`/`test-app` to ensure the
   repository compiles without warnings and all tests pass.  Fix any
   new test failures that may have been introduced by refactoring.
7. If the changes are extensive, break them into logical commits or
   explain the grouping to the controller.

This skill may be run multiple times during a maintenance pass.  It is
not responsible for adding new features; its goal is to leave the source
cleaner, simpler and free of obvious defects.

## Usage

Invoke this skill from the development workflow (`dev_workflow.prompt.md`)
whenever a full codebase sweep is required.  The agent should report the
changes made or present them for review before committing.
