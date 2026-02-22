---
name: "Porting request"
about: "Track porting the project to a new platform or toolchain"
title: "Port: [platform] - short description"
labels: "porting"
assignees: ""
---

<!--
Related issue: <issue>
-->

### Summary
Short description of what's being ported and why.

### Target platform
- OS / Architecture (e.g. Linux/arm64, Windows/x64, WASI):
- Toolchain / Compiler (e.g. Clang, MSVC, MinGW, Emscripten):

### Scope
- Components to port (e.g. wasm runtime, SDL3 GUI, build scripts):

### Steps to reproduce / Build
1. Describe how the project is currently built.
2. List steps attempted on the target platform.

### Acceptance criteria
- [ ] Repository builds on the target platform using documented steps
- [ ] Existing tests (if any) pass on the target platform
- [ ] CI/workflows updated if necessary to cover the platform
- [ ] Documentation (README or docs/) updated with platform-specific instructions

### Additional context
Add logs, error messages, links to CI runs, or additional notes here.
