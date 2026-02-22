#pragma once

#include "policy.h"
#include "advisor.h"

// Trainer applies online updates to a policy network given telemetry data.
class Trainer {
public:
    Trainer();

    // observe one telemetry entry and adjust weights accordingly
    void observe(const TelemetryEntry& entry);

    // save/load model state (weights) to disk
    bool save(const std::string& path) const;
    bool load(const std::string& path);

    // expose access to underlying policy for inspection/tests
    const Policy& policy() const { return m_policy; }

private:
    Policy m_policy;
    int m_observations = 0;
};
