#include "train.h"
#include "feature.h"
#include "loss.h"
#include <fstream>
#include <cmath>

// ─── Architecture ─────────────────────────────────────────────────────────────
// Layer 0 Dense : kFeatSize(1024) → 16
// Layer 1 Dense : 16  → 1024   (bridge to deeper stack)
// Layer 2 Dense : 1024 → 1024  (was specified as 256, increased to 1024)
// Layer 3 LSTM  : 1024 → 1024  (temporal context; Xavier-initialised)
// Layer 4 Dense : 1024 → 16
// Layer 5 Dense : 16   → 1     (scalar reward prediction)

Trainer::Trainer() {
    m_policy.addDense(kFeatSize, 16);    // layer 0
    m_policy.addDense(16, 1024);         // layer 1
    m_policy.addDense(1024, 1024);       // layer 2
    m_policy.addLSTM(1024, 1024);        // layer 3
    m_policy.addDense(1024, 16);         // layer 4
    m_policy.addDense(16, 1);            // layer 5 (output)
}

void Trainer::observe(const TelemetryEntry& entry) {
    m_observations++;
    if (entry.kernelBase64.empty()) return;

    // Extract opcode-frequency feature vector (kFeatSize floats)
    auto features = Feature::extract(entry);

    // Reward signal: generation count (higher = kernel survived longer = better)
    float reward = static_cast<float>(entry.generation);
    if (reward > m_maxReward) m_maxReward = reward;
    float normReward = m_maxReward > 0.0f ? reward / m_maxReward : 0.0f;

    // Forward pass – stores every layer's input activation for weight updates
    std::vector<std::vector<float>> acts;
    m_policy.forwardActivations(features, acts);
    float prediction = acts.back().empty() ? 0.0f : acts.back()[0];

    // MSE loss
    float diff = prediction - normReward;
    m_lastLoss = diff * diff;
    // Exponential moving average of loss
    m_avgLoss = m_avgLoss * 0.9f + m_lastLoss * 0.1f;

    // Delta-rule weight update for every dense layer.
    // Each layer's weights are nudged using its own input activation so that
    // the full network participates in learning.
    // (Biases are left unchanged to preserve forward(zero_input)==0.)
    const float lr = 0.005f;
    for (int l = 0; l < m_policy.layerCount(); ++l) {
        if (m_policy.layerType(l) == Policy::LayerType::LSTM) continue;
        const int inl  = m_policy.layerInSize(l);
        const int outl = m_policy.layerOutSize(l);
        const auto& input_act = acts[l]; // input to this layer = acts[l]
        auto w = m_policy.layerWeights(l);
        for (int o = 0; o < outl; ++o)
            for (int i = 0; i < inl; ++i)
                w[o * inl + i] -= lr * diff * input_act[i];
        m_policy.setLayerWeights(l, w);
    }
}

// ─── Persistence ─────────────────────────────────────────────────────────────
// Save format per layer:  type in out\n  [weights...]\n  [biases...]\n
// type: 0=DENSE, 1=LSTM.  LSTM weight count = 4*(in+out)*out; bias = 4*out.

bool Trainer::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << m_observations << "\n";
    out << m_avgLoss << " " << m_maxReward << "\n";
    for (int l = 0; l < m_policy.layerCount(); ++l) {
        const auto& w = m_policy.layerWeights(l);
        const auto& b = m_policy.layerBiases(l);
        out << (int)m_policy.layerType(l) << " "
            << m_policy.layerInSize(l)    << " "
            << m_policy.layerOutSize(l)   << "\n";
        for (float v : w) out << v << " ";
        out << "\n";
        for (float v : b) out << v << " ";
        out << "\n";
    }
    return out.good();
}

bool Trainer::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    if (!(in >> m_observations)) return false;
    in >> m_avgLoss >> m_maxReward;
    for (int l = 0; l < m_policy.layerCount(); ++l) {
        int type = 0, ins = 0, outs = 0;
        if (!(in >> type >> ins >> outs)) return false;
        if (ins  != m_policy.layerInSize(l))  return false;
        if (outs != m_policy.layerOutSize(l)) return false;
        if (type != (int)m_policy.layerType(l)) return false;
        // LSTM weight tensor is 4*(in+out)*out; dense is in*out
        int wcount = (type == (int)Policy::LayerType::LSTM)
                     ? 4 * (ins + outs) * outs
                     : ins * outs;
        int bcount = (type == (int)Policy::LayerType::LSTM)
                     ? 4 * outs
                     : outs;
        std::vector<float> w(wcount), b(bcount);
        for (float& v : w) if (!(in >> v)) return false;
        for (float& v : b) if (!(in >> v)) return false;
        m_policy.setLayerWeights(l, w);
        m_policy.setLayerBiases(l, b);
    }
    return true;
}

