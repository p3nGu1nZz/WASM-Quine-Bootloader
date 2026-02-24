# Neural Matrix Representation

To enable the evolving WASM kernel to store and act upon its learned
experience, we define a simple in-memory graph structure that may be
serialized into the kernel genome and accessed via host imports.

## Format

- The neural matrix is a directed adjacency graph whose nodes are
  instruction byte sequences and whose edge weights represent observed
  transition frequencies.
- In linear memory the graph is encoded as a sequence of records:
  ```text
  [node_count:u32]
  for each node {
      [instr_len:u32] [instr_bytes...]
      [edge_count:u32]
      for each edge {
          [target_node_index:u32]
          [weight:f32]
      }
  }
  ```
- The entire structure must fit within the kernel's 32 KB code section.

## Host Imports

Currently the only host import provided for neural data is a simple
logger that allows the kernel to ship serialized blobs back to the
bootloader.  This is enough to extract a copy of the matrix for
analysis, but richer operations may be added as the evolution engine
matures.

- `env.record_weight(ptr:u32,len:u32)` – send a chunk of bytes to the
  host.  The semantics of the blob are up to the kernel; the intended use
  is to transmit a serialized neural matrix or incremental update.  The
  host will receive the data via the `WeightCallback` supplied to
  `WasmKernel::bootDynamic` and may choose to persist it in telemetry files.

Future releases may also offer the following (currently unimplemented)
imports:

- `env.add_edge(ptr:u32,len:u32,target:u32,weight:f32)` – append an edge
  record.  `ptr`/`len` point to the source instruction sequence in linear
  memory.
- `env.query_weight(ptr:u32,len:u32,target:u32) -> f32` – return the
  current weight of the given transition (0.0 if absent).
- `env.serialize_matrix(ptr:u32)` – write the compressed matrix to
  memory starting at `ptr` and return the byte length; used when saving
  the kernel.

## Kernel Responsibilities

- During execution the kernel should update the matrix in response to
  observed instruction sequences (e.g., after each `run()` call).
- The matrix may be mutated heuristically when the program evolves; the
  bootloader treats the serialized bytes as part of the quine string.
- Children kernels spawned via `env.spawn` will inherit a copy of the
  current matrix and may continue training.

## Future Work

- Define compression schemes for large graphs (spatial hashing, delta
  encoding).
- Provide utilities in the bootloader for offline analysis of the matrix
  snapshots located in telemetry exports.
- Implement richer host imports (`add_edge`, `query_weight`,
  `serialize_matrix`) so that kernels can manipulate the matrix at runtime
  and query learned weights during execution.
- **Experience Inheritance** – when a kernel spawns a child via
  `env.spawn`, the serialized matrix (or a compressed snapshot thereof)
  should travel with the genome.  Children should load this data on
  startup and continue training without starting from scratch.  One
  proposal is to attach the blob as a suffix to the base64 quine string
  with a small header indicating its length.