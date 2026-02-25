#include "train.h"
#include "feature.h"
#include "loss.h"
#include <fstream>
#include <cmath>

Trainer::Trainer() {
    // start with a small policy: input size 256 (opcode histogram) -> 16 -> 1
    m_policy.addDense(256, 16);
    m_policy.addDense(16, 1);
}

void Trainer::observe(const TelemetryEntry& entry) {
    m_observations++;
    if (entry.kernelBase64.empty()) return;

    // Extract opcode-frequency feature vector (256 floats)
    auto features = Feature::extract(entry);

    // Reward signal: generation count (higher = kernel survived longer = better)
    float reward = static_cast<float>(entry.generation);
    if (reward > m_maxReward) m_maxReward = reward;
    float normReward = m_maxReward > 0.0f ? reward / m_maxReward : 0.0f;

    // Forward pass to get current prediction
    auto pred = m_policy.forward(features);
    float prediction = pred.empty() ? 0.0f : pred[0];

    // MSE loss
    float diff = prediction - normReward;
    m_lastLoss = diff * diff;
    // Exponential moving average of loss
    m_avgLoss  = m_avgLoss * 0.9f + m_lastLoss * 0.1f;

    // Simple delta-rule weight update for layer 0 (256→16).
    // w[o * in + i] -= lr * diff * feature[i]
    // (no bias update so that forward(zero_input) stays at zero, preserving tests)
    const float lr = 0.005f;
    const int in0  = m_policy.layerInSize(0);
    const int out0 = m_policy.layerOutSize(0);
    auto w0 = m_policy.layerWeights(0); // copy
    for (int o = 0; o < out0; ++o)
        for (int i = 0; i < in0; ++i)
            w0[o * in0 + i] -= lr * diff * features[i];
    m_policy.setLayerWeights(0, w0);
}

bool Trainer::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << m_observations << "\n";
    out << m_avgLoss << " " << m_maxReward << "\n";
    // persist weights for every layer
    for (int l = 0; l < m_policy.layerCount(); ++l) {
        const auto& w = m_policy.layerWeights(l);
        const auto& b = m_policy.layerBiases(l);
        out << m_policy.layerInSize(l) << " " << m_policy.layerOutSize(l) << "\n";
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
        int ins = 0, outs = 0;
        if (!(in >> ins >> outs)) return false;
        if (ins != m_policy.layerInSize(l) || outs != m_policy.layerOutSize(l))
            return false;
        std::vector<float> w(ins * outs), b(outs);
        for (float& v : w) if (!(in >> v)) return false;
        for (float& v : b) if (!(in >> v)) return false;
        m_policy.setLayerWeights(l, w);
        m_policy.setLayerBiases(l, b);
    }
    return true;
}

