#pragma once

#include "wasm/parser.h"
#include "../cli.h"  // for MutationStrategy
#include <string>
#include <vector>

enum class EvolutionAction { MODIFY = 0, INSERT = 1, ADD = 2, DELETE = 3 };

struct EvolutionResult {
    std::string     binary;           // base64-encoded evolved binary
    std::vector<uint8_t> mutationSequence; // empty = no mutation tracked
    EvolutionAction actionUsed;
    std::string     description;
};

// Produce an evolved WASM binary from the current base64-encoded kernel.
// knownInstructions: previously seen instruction byte sequences for guided mutation.
// attemptSeed: determines which action to try (cycles through 0-3).
EvolutionResult evolveBinary(
    const std::string&                             currentBase64,
    const std::vector<std::vector<uint8_t>>&       knownInstructions,
    int                                            attemptSeed,
    MutationStrategy                               strategy = MutationStrategy::RANDOM
);
