---
name: build-and-test
description: Build the C++ project using the provided scripts and run the unit test suite. Use this skill when needing to compile, verify, or troubleshoot the native application on Linux or Windows targets.
---

# build-and-test

Purpose

Run a full build for the chosen target and execute the test harness. Useful for CI and developer verification.

Usage

- Execute from repository root.
- Ensure `external/` dependencies are prepared (via `bash scripts/setup.sh`).

Commands

- Build (default linux-debug):

```bash
bash scripts/build.sh
```

- Run tests:

```bash
bash scripts/test.sh
```

Inputs

- Optional build target (e.g., `linux-release`).

Outputs

- Build artifacts under `build/<target>/`.
- Test runner exit code and logs.

Security / Notes

- Requires `cmake`, `ninja`, `gcc/clang` and system libs as per `scripts/setup.sh`.
