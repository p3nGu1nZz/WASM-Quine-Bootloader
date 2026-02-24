# Multi-Instance Semantics

The bootloader may manage multiple concurrent WASM kernels, each running
its own copy of the program.  This document specifies the behaviour of
instances and the host interfaces for spawning and controlling them.

## Instance Lifecycle

- **Spawn**: A running kernel may call the imported function
  `env.spawn(ptr:length)` to request that the bootloader create a new
  instance.  The bytes pointed to by `ptr`/`length` are interpreted as a
  base64-encoded kernel image (typically the caller's own quine or a
  mutated variant).
- **Storage**: Spawned kernels are kept in a vector inside `App` and may
  be executed in future cycles; initially they are inert until an
  explicit scheduler is implemented.
- **Visibility**: The GUI status bar displays `Instances: N` where `N`
  is the number of spawned kernels.  Future panels (see #16) may allow
  per-instance telemetry and controls.
- **Termination**: There is currently no mechanism to kill individual
  instances; all instances are lost when the parent process exits.

## Host API (Imported Functions)

- `env.spawn(ptr:u32,len:u32)` – create a new instance containing the
  given base64 kernel.  Always linked, even if the kernel chooses not to
  call it.
- In future, `env.kill_instance(id:i32)` may be added to allow programmatic
  termination.

## Bootloader Behaviour

- Spawns are logged via `App::spawnInstance` and appear in telemetry
  exports as an entry in the history log.
- The initial implementation does not execute spawned instances; they
  are available for offline inspection by tools or GUI panels.

## Data Persistence

- Telemetry exports now include an additional section `INSTANCES:`
  listing any base64 blobs received via `env.spawn` during the run.
  Parsers should skip this section if it is absent.

## Notes

This is a lightweight framework to support experimentation with
self-replicating kernels.  The multi-instance model enables ideas such
as competition between sibling programs, ensemble voting, or genetic
crossover.
