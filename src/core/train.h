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

    // training statistics
    int   observations() const { return m_observations; }
    float avgLoss()      const { return m_avgLoss; }
    float lastLoss()     const { return m_lastLoss; }

    // testing hooks
    bool test_lastUsedSequence() const { return m_lastUsedSequence; }

private:
    // true if last observe() call processed a non-empty opcode sequence
    bool m_lastUsedSequence = false;
    Policy m_policy;
    int   m_observations = 0;
    float m_avgLoss      = 0.0f;
    float m_lastLoss     = 0.0f;
    float m_maxReward    = 1.0f; // tracks max reward seen for normalisation
};
