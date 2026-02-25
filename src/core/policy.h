#pragma once

#include <vector>

// Simple feed-forward neural network policy.  Layers are defined by
// their weight matrices and biases.  This is *not* a production ML
// library; it only provides the minimal operations we need for on‑device
// learning.

class Policy {
public:
    Policy() = default;

    // add a dense layer with given input/output sizes; weights are zeroed
    void addDense(int inSize, int outSize);

    // run a forward pass on `input` and return the output vector
    std::vector<float> forward(const std::vector<float>& input) const;

    // simple ReLU activation applied in-place
    static void relu(std::vector<float>& v);

    // helpers for testing: set/get layer weights and biases
    void setLayerWeights(int idx, const std::vector<float>& w);
    void setLayerBiases(int idx, const std::vector<float>& b);

    // read-only accessors for architecture inspection / training
    int layerCount()             const { return (int)m_layers.size(); }
    int layerInSize(int i)       const { return (i >= 0 && i < (int)m_layers.size()) ? m_layers[i].in  : 0; }
    int layerOutSize(int i)      const { return (i >= 0 && i < (int)m_layers.size()) ? m_layers[i].out : 0; }
    const std::vector<float>& layerWeights(int i) const {
        static const std::vector<float> empty;
        return (i >= 0 && i < (int)m_layers.size()) ? m_layers[i].weights : empty;
    }
    const std::vector<float>& layerBiases(int i) const {
        static const std::vector<float> empty;
        return (i >= 0 && i < (int)m_layers.size()) ? m_layers[i].biases : empty;
    }

private:
    // each layer: weights (outSize x inSize), biases (outSize)
    struct Layer { std::vector<float> weights; std::vector<float> biases; int in, out; };
    std::vector<Layer> m_layers;
};
