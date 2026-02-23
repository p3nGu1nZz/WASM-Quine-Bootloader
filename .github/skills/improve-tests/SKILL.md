---
name: improve-tests
description: Enhance and expand the repository's unit tests for better coverage and robustness.
---

# improve-tests

## Purpose

Enhance and expand the unit-test suite to improve coverage and robustness
whenever the code changes or new features are added.

## Behaviour

1. Audit existing `test/*.cpp` files to see which modules and functions
   are exercised.  Use `grep_search`/`semantic_search` to locate untested
   symbols in `src/`.  Tests must compile under `-Werror`.
2. Identify missing scenarios, boundaries and failure cases (invalid CLI
   args, truncated telemetry, file errors, GUI init failures, etc.).
   Add tests for any new CLI flags such as `--heuristic` or
   `--mutation-strategy`.
3. Propose or write Catch2 test cases for each gap, with clear assertions
   and comments.  Include code snippets where helpful.
4. Ensure every new source file has a corresponding `test_<module>.cpp`.
   For files containing TODOs/stubs, create tests that guide their
   completion.
5. Test scripts (`scripts/*.sh`) too; if they contain logic, write
   CMake tests or small shell helpers verifying their behaviour.
6. Run `bash scripts/test.sh` to compile and execute the suite; fix any
   regressions or newly discovered bugs.
7. Summarise changes, note uncovered issues, and suggest future tests
   (e.g. fuzzing mutation logic or heuristic decay mode).

## Usage

Run this skill before refactors or when adding features.  The agent
should report new/modified tests and remaining weak spots.  Adhere to the
convention: each module has a `test_<module>.cpp` and tests use
`TEST_CASE("...","[tag]")`.

## Usage

Invoke when you want to boost confidence in the codebase or before
significant refactors.  It is especially useful after adding new
modules such as the neural network, advisor, or when scripting changes
could affect behaviour.  The skill is responsible for both auditing
existing tests and authoring new ones.

Tests created by this skill should follow the project's naming
convention: each source module gets a corresponding `test_<module>.cpp`
file and test functions are named `TEST_CASE("...", "[tag]")`.

When the user asks for concrete results, the agent should summarise the
changes and point out any remaining weak spots or planned future tests.
