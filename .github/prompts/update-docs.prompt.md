# Update Documentation

Update the project documentation to reflect code changes made to the WASM Quine Bootloader C++ project.

## Files to Update

| File | When to update |
|------|----------------|
| `README.md` | New features, changed build steps, new running modes, new dependencies |
| `.github/copilot-instructions.md` | New source files, changed conventions, new build targets, new constraints |
| `src/*.h` | Public API changes – update Doxygen-style comments on changed functions |
| `scripts/*.sh` | Script behaviour changes – update the usage comment at the top of each script |

## README.md Structure

The README should always contain these sections in order:

1. **Project banner / title**
2. **What it is** – one-paragraph description of the WASM quine concept and C++ implementation
3. **Prerequisites** – OS, CMake version, Ninja, SDL3 dependencies
4. **Quick start** – `bash scripts/setup.sh` then `bash scripts/run.sh`
5. **Running modes** – terminal (default) and `--gui`
6. **Building** – `bash scripts/build.sh [target]` with target table
7. **Testing** – `bash scripts/test.sh`
8. **Project structure** – directory tree with one-line descriptions

## Updating Inline Source Comments

- Use `///` (triple-slash) Doxygen comments on all `public` class methods in headers.
- Use `// —` section separator comments inside `.cpp` files for major logical blocks.
- Keep comments describing *why*, not *what* the code does (the code itself says what).

## Updating `.github/copilot-instructions.md`

When adding a new source file, add a line to the **Repository Structure** tree.  
When adding a new build target, add a row to the **Build Targets** table.  
When adding a new constraint (e.g. a new forbidden opcode), add it to **Important Constraints**.

## Checklist After Any Significant Change

- [ ] `README.md` prerequisites still accurate?
- [ ] Build steps in README match current `scripts/build.sh`?
- [ ] New public API functions documented in header files?
- [ ] `copilot-instructions.md` repository structure up-to-date?
- [ ] New constraints or conventions recorded in `copilot-instructions.md`?
