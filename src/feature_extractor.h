#pragma once

#include "mutation_advisor.h"
#include "wasm/parser.h"
#include <vector>

// Convert a telemetry entry (kernel/mutation) into a fixed-length
// numeric feature vector suitable for feeding into a neural policy.
// Current implementation counts opcode frequencies in the kernel.
class FeatureExtractor {
public:
    // return vector of 256 floats where index=opcode count
    static std::vector<float> extract(const TelemetryEntry& entry);
};
