#include "policy.h"
#include <algorithm>

void Policy::addDense(int inSize, int outSize) {
    Layer layer;
    layer.in = inSize;
    layer.out = outSize;
    layer.weights.assign(inSize * outSize, 0.0f);
    layer.biases.assign(outSize, 0.0f);
    m_layers.push_back(std::move(layer));
}

std::vector<float> Policy::forward(const std::vector<float>& input) const {
    std::vector<float> current = input;
    for (const auto& layer : m_layers) {
        std::vector<float> next(layer.out, 0.0f);
        for (int o = 0; o < layer.out; ++o) {
            float sum = layer.biases[o];
            for (int i = 0; i < layer.in; ++i) {
                sum += layer.weights[o * layer.in + i] * current[i];
            }
            next[o] = sum;
        }
        relu(next);
        current.swap(next);
    }
    return current;
}

void Policy::relu(std::vector<float>& v) {
    for (auto &x : v) if (x < 0.0f) x = 0.0f;
}

// helpers for tests
void Policy::setLayerWeights(int idx, const std::vector<float>& w) {
    if (idx < 0 || idx >= (int)m_layers.size()) return;
    auto &layer = m_layers[idx];
    if ((int)w.size() == layer.in * layer.out)
        layer.weights = w;
}

void Policy::setLayerBiases(int idx, const std::vector<float>& b) {
    if (idx < 0 || idx >= (int)m_layers.size()) return;
    auto &layer = m_layers[idx];
    if ((int)b.size() == layer.out)
        layer.biases = b;
}
