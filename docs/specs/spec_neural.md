# Neural Network Policy – RL Trainer

The bootloader maintains an on-device reinforcement-learning policy that
predicts kernel quality from opcode statistics and guides the mutation
engine toward longer-surviving kernels.

---

## Architecture (current)

```
Input (kFeatSize = 1024 floats)
  │  indices 0-255  : WASM opcode frequency counts or opcode one-hot when
  │                    sequence training is active
  │  indices 256-1023: reserved for future features (currently zero)
  ▼
Layer 0  Dense  1024 → 32     (compact input projection)
  ▼
Layer 1  Dense   32 → 64      (feeds LSTM)
  ▼
Layer 2  LSTM    64 → 64      (temporal context; state reset on each new
                               telemetry entry during training)
  ▼
Layer 3  Dense   64 → 32      (dimensionality reduction)
  ▼
Layer 4  Dense   32 → 1       (scalar reward prediction)
```

The network has been dramatically shrunk from its original 1024‑wide
layout.  Aside from the reduced layer widths the only remaining recurrent
component is a single LSTM with a 64‑element hidden state; its weights are
currently untouched by weight updates for simplicity.

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

A single entry comprises the kernel that was executed along with its
`generation` count.  Two update modes are supported:

* **Histogram mode** – the original behaviour.  `Feature::extract()` builds a
  1024‑element vector counting opcode frequencies and that vector is fed
  forward through the network.
* **Sequence mode** – if `Feature::extractSequence()` returns a non-empty list
  of opcode bytes then training proceeds one opcode at a time.  Each opcode is
  converted into a one‑hot feature vector (using the first 256 indices) and
  the network's hidden state is advanced; this allows the LSTM to provide
  temporal context.  On each step the same delta rule described below is
  applied, effectively unrolling the network over the sequence.  Sequence
  mode is now the default when telemetry files contain opcode data.

To improve convergence we now maintain a small **replay buffer** of recent
sequence-based examples (default capacity 256).  `Trainer::observe()` always
trains on the current entry and then samples one random item from the buffer
for an additional update, creating a very lightweight "mini‑batch" effect.
New sequence entries are appended to the buffer, and old entries are dropped
FIFO when the capacity is exceeded.  This replay mechanism is transparent to
the rest of the system and is tested via `test_replaySize()`.

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

## Automatic Training Trigger

The host application now includes a simple scheduling mechanism: once the
system has successfully executed **50 generations** during an evolution run,
execution is halted, the accumulated telemetry is reloaded, and a training
phase begins automatically using the newest data.  This removes the need
for manual mode-switching and keeps the training dataset narrowly focused
on the most recent evolutionary events.
