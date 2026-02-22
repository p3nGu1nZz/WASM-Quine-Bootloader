# Mutation Heuristic – Specification

## Purpose

Describe the adaptive algorithm that monitors kernel execution failures
and biases the evolution engine to avoid repeating destructive edits.
The heuristic helps the bootloader recover more quickly from traps and
improve long-term survival rate of mutated kernels.

## Inputs

- History of generations, including which mutations were applied and
  whether execution resulted in a trap or successful quine verification.
- Optional external parameters provided via CLI flags (e.g.
  `--mutation-strategy` or `--heuristic-decay`).

## Behaviour

1. Maintain a small "blacklist" of mutation patterns that previously led
   to unreachable or other traps (e.g. inserting an `unreachable` opcode
   or appending a balanced `[i32.const,x, drop]` sequence where `x` is
   out of range).
2. After each generation:
   - If execution succeeded, optionally decay the blacklist entries
     (allowing them to be retried later).
   - If execution trapped, record the mutation that preceded the failure
     and add it to the blacklist.
3. When selecting a random mutation for the next generation, consult the
   blacklist and reroll if the candidate matches a forbidden pattern.
4. Expose a CLI flag such as `--heuristic=<none|blacklist|decay>` to
   control whether the heuristic is active and its aggressiveness.
5. The heuristic is local to a run; it is not persisted between program
   invocations (although export reports may record blacklist contents for
   offline analysis).

## Outputs / Side Effects

- The evolving kernel may avoid repeating the same trap-inducing edits,
  reducing the frequency of crashes and repairs.
- Telemetry exports should optionally include a summary of the blacklist
  (e.g. count of entries) when the heuristic is enabled.

## Constraints

- The heuristic logic must not introduce significant overhead; it should
  be simple enough to run in O(1) or O(k) where k is the blacklist size.
- Blacklist comparisons should be structural rather than full binary
  equality to handle mutations that differ only in operand values.

## Open Questions

- Should the blacklist be shared across parallel runs (e.g. via a file)?
- Could a positive reinforcement mechanism (reward safe mutations) be
  combined with the blacklist?
