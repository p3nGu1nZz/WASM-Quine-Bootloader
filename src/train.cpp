#include "train.h"
#include "feature.h"
#include "loss.h"
#include <fstream>

Trainer::Trainer() {
    // start with a small policy: input size 256 (opcode histogram) -> 16 -> 1
    m_policy.addDense(256, 16);
    m_policy.addDense(16, 1);
}

void Trainer::observe(const TelemetryEntry& entry) {
    // simple stub: increment observation count
    m_observations++;
    // full implementation would compute features/loss and adjust policy
    (void)entry;
}

bool Trainer::save(const std::string& path) const {
    // trivial implementation: write observation count
    std::ofstream out(path);
    if (!out) return false;
    out << m_observations;
    return true;
}

bool Trainer::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    in >> m_observations;
    return true;
}
