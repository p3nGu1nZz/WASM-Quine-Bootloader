# Neural Network Policy – RL Trainer

The bootloader maintains an on-device reinforcement-learning policy that
predicts kernel quality from opcode statistics and guides the mutation
engine toward longer-surviving kernels.

---

## Architecture (current)

```
Input (kFeatSize = 1024 floats)
  │  indices 0-255  : WASM opcode frequency counts
  │  indices 256-1023: reserved for future features
  ▼
Layer 0  Dense  1024 → 16     ReLU
  ▼
Layer 1  Dense    16 → 1024   ReLU   (bridge)
  ▼
Layer 2  Dense  1024 → 1024   ReLU
  ▼
Layer 3  LSTM   1024 → 1024   Xavier-initialised; h/c state persists
                               across observations in a training epoch
  ▼
Layer 4  Dense  1024 → 16     ReLU
  ▼
Layer 5  Dense    16 → 1      ReLU   (scalar reward prediction)
```

All dense weights are zero-initialised (so `forward(zeros) = 0`).
LSTM gate weights use Xavier-uniform initialisation
(`±sqrt(6/(inSize+hiddenSize))`) with a fixed LCG seed for
reproducibility; LSTM biases are zero.

---

## Feature Extraction (`src/core/feature.cpp`)

`Feature::extract(entry)` decodes the kernel base64 string, parses the
WASM code section, counts per-opcode occurrences, and returns a
`kFeatSize`-element float vector.  Indices 256-1023 are currently zero
and reserved for future features (e.g. section-size statistics).

---

## Reward & Loss (`src/core/train.cpp`, `src/core/loss.cpp`)

| Symbol | Value |
|--------|-------|
| reward | `entry.generation` (higher generation = longer survival = better) |
| normReward | `reward / maxRewardSeen` |
| loss | `(prediction − normReward)²` (MSE) |
| avgLoss | exponential moving average, α = 0.1 |

---

## Training Update

After each `Trainer::observe()` call the delta rule is applied to every
**dense** layer.  Each layer's weights are nudged using the layer's own
input activation and the global output error:

```
w[l][o,i] -= lr * diff * acts[l][i]    (lr = 0.005)
```

Biases are **not** updated so that `forward(zeros) = 0` is preserved.
The LSTM layer participates in the forward pass (providing temporal
context) but its weights are not updated by this rule; they retain their
Xavier-initialised values and evolve only via future BPTT.

`forwardActivations()` stores every layer's input in `acts[l]` so that
the weight updates can access them without a second forward pass.

---

## Model Persistence

`Trainer::save(path)` / `Trainer::load(path)` serialise the full network
state including layer type (0 = DENSE, 1 = LSTM), in/out sizes, all
weights, and all biases.  The LSTM weight tensor has size
`4 × (in + hidden) × hidden`; the bias tensor has size `4 × hidden`.
A mismatch in type, in-size, or out-size causes `load()` to return
`false`.

---

## Neural Matrix Host Import (existing)

The kernel may call `env.record_weight(ptr, len)` to ship a serialised
weight blob back to the host via the `WeightCallback` supplied to
`WasmKernel::bootDynamic`.  This mechanism is independent of the C++
policy above and is intended for future kernel-side learning.

Future host imports (currently unimplemented):

- `env.add_edge(ptr, len, target, weight)` – append a graph edge.
- `env.query_weight(ptr, len, target) → f32` – query a transition weight.
- `env.serialize_matrix(ptr) → len` – write the compressed adjacency
  graph to linear memory.

---

## Future Work

- **BPTT for LSTM** – implement back-propagation through time so the
  LSTM gate weights are trained.
- **Richer features** – populate indices 256-1023 with section-size
  statistics, base64 entropy, generation delta, etc.
- **Experience inheritance** – propagate a compressed weight snapshot
  from parent to child kernel via `env.spawn`.
- **Compression** – spatial hashing / delta encoding for large graphs.
