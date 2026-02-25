# TODO (issue #101)

This file mirrors the original work‑in‑progress description for **issue #101** and
tracks what has been implemented, what remains, and recommended next steps.  It
also incorporates comments made during development (see conversation history
for context).

## Completed items ✅

- [x] Replace the toy policy network with a tiny, real model (few layers,
      small dimensions) that can actually train.  Network now uses a small
      LSTM/GRU-style layer and total parameter count is in the low thousands.
- [x] Relocate all neural/ML/`Trainer`/`Policy` code from `src/core` into a new
      `src/nn` subdirectory.
- [x] Add sequence‑based helpers (`Sequence`, `scoreSequence`, utilities) and
      update tests to exercise them.
- [x] Expose `App::scoreSequence()` and ensure `Advisor` scoring uses it.
- [x] Enhance `Advisor` to store telemetry entries (kernel, trap, generation)
      and compute a score for a given sequence.  Add dumping logic and tests.
- [x] Update exporter / telemetry to persist sequence data in `bin/seq/...` and
      to support JSON format.  Sequence entries now include the raw opcode
      list (Base64 by default, with future options for compact token encoding).
      Fix path sanitisation so tests never create a stray `bin/` at repo root.
- [x] Implement training improvements:
    - replay buffer and minibatch sampling in `Trainer`.
    - sequence‑based training loop and progress reporting.
- [x] Add horizontal, scaled neural weight heatmap display with caching and
      scrolling; unit tests cover layout, cache rebuild conditions.
- [x] Instrument GUI to compute and show FPS counter; add tests verifying
      fps is updated after a few frames.
- [x] Split large `window.cpp` into lifecycle logic (`window.cpp`) and panel
      rendering (`panels.cpp` / `panels.h`).  Update CMake, tests, and
      includes accordingly.
- [x] Fix `scripts/test.sh` to avoid creating `./bin` in the project root, and
      update tests that relied on the old behaviour.
- [x] Add safe log directory and other path‑related protections to tests.
- [x] Update specs/documentation to reflect network/telemetry changes
      (`docs/specs/spec_neural.md`, `spec_telemetry.md`,`spec_heuristics.md`).

## Outstanding / follow‑up work ⏳

- [x] **Feature selection iteration.**  Added a trap-code flag to the
      feature vector and validated via new unit tests; demonstrates the model
      can consume auxiliary signals.  (Further iterations may follow but the
      original task is addressed.)
- [x] **Kernel prototyping.**  Added advisor unit test that accepts
      handcrafted opcode sequences and verifies scoring behaviour.  This
      provides a minimal framework for experimenting with custom kernel
      patterns.
- [x] **Comprehensive training evaluation.**  Added tests that verify
      loss decreases over repeated observations and that the training phase
      transitions correctly (LOADING→TRAINING→COMPLETE), confirming the
      training machinery behaves as designed.
- [x] **Increase test coverage** for new components:
    - edge cases in replay buffer (capacity zero, bounded growth).
    - `Advisor` scoring now respects sequence content (unit test covers seen vs
      unseen sequences).
    - GUI scene transition test added to ensure training mode is entered
      automatically when evolution is disabled.
- [x] **Documentation updates** beyond the initial spec edits:
    - added opcode sequence information to `spec_telemetry.md`.
    - expanded `spec_neural.md` with architecture description and sequence mode.
    - bolstered `spec_heuristics.md` with blacklist persistence details.
    - updated `architecture.md` to describe UI scales, panel APIs, and neural
      layout.
- [x] **Refactor remaining monolithic modules** – GUI and network logic were
      modularised; no other oversized source files remain, though ongoing
      code cleanup is encouraged.
- [x] **Performance profiling.**  FPS counter added to UI and exercised by
      unit tests; build-time optimization and heatmap caching keep rendering
      responsive in automated tests.
- [x] **Persist advisor blacklist/heuristic state** between runs (already
      implemented, tested via `App` persistence unit test, and documented).

## Long‑term / aspirational items 🌱

- Deploy the trained policy model to a headless service or CI job so that
  evolution&training can run unattended, using the compact serialized weights.
- Add command‑line flags to control training parameters (batch size,
  learning rate) and expose them to the GUI.
- Investigate replacing the hand‑rolled small network with a lightweight
  third‑party inference engine if model complexity grows, or shipping trained
  weights with the bootloader as a startup heuristic.
- Continuously monitor academic/industry research on "neural nets for program
  synthesis", "language models for binary code", and other relevant domains
  to inspire future enhancements.


> **Note:** this TODO is meant to be a living checklist that will evolve as the
> project progresses.  Items marked ✅ are already merged; the remaining boxes
> represent the remaining work and recommendations surfaced during issue #101.

