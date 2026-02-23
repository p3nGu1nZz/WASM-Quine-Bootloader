---
name: improve-tests
description: Enhance and expand the repository's unit tests for better coverage and robustness.
---

# improve-tests

This skill instructs the agent to examine the existing test suite and
identifiy ways to improve coverage, fill gaps, and add new tests for
recently added code.  It can also generate tests for future features or
refactors as part of the review process.

## Expectations

1. Read all current `test/*.cpp` files and note which modules and
   functions they exercise.  Use `grep_search` or `semantic_search` to
   locate untested symbols in `src/` such as free functions or class
   methods.  Ensure tests compile cleanly under the same `-Werror` flags
   as the main code.
2. Identify missing scenarios, boundary conditions, and failure cases
   (e.g. invalid CLI arguments, empty telemetry entries, file I/O
   errors, GUI initialization errors) that are not already covered.  Add
   tests for new CLI flags like `--heuristic` or `--mutation-strategy`
   and for telemetry exports containing extra fields or warnings.
3. For each identified gap, propose a new Catch2 test case (or modify an
   existing one) with a clear description and assertions.  Include code
   snippets where helpful.
4. When new source files are added (e.g. `feature.cpp`, `advisor.cpp`),
   ensure corresponding tests exist and are reasonably thorough.  If the
   file contains TODOs or stubs, propose tests that will drive their
   implementation.
5. Check the scripts (`scripts/*.sh`) and tools; if they have tricky
   logic, write a CMake test or shell-based check to verify behaviours
   like argument parsing or cleanup.  Update `test/test_util_dpi.cpp`
   or create new helpers as needed.
6. After proposing tests, run `bash scripts/test.sh` to verify they
   compile and pass.  If failures arise, document them and either fix the
   code or adjust the tests accordingly.
7. Report back a summary of what was done: new/modified tests, any
   uncovered issues, and suggestions for further enhancements.  Mention
   additional tests that could be added in future, such as fuzzing the
   kernel mutation logic or exercising the heuristic decay mode.

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
