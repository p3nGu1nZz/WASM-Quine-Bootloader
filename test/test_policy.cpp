#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "policy.h"

TEST_CASE("Policy forward pass and layer addition", "[policy]") {
    Policy p;
    p.addDense(3, 2); // one layer 3->2
    // configure weights so output = [in0+in1, in1+in2]
    std::vector<float> weights = {
        1.0f, 1.0f, 0.0f, // first output neuron
        0.0f, 1.0f, 1.0f  // second output neuron
    };
    std::vector<float> biases = {0.0f, 0.0f};
    p.setLayerWeights(0, weights);
    p.setLayerBiases(0, biases);

    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    auto out = p.forward(input);
    REQUIRE(out.size() == 2);
    REQUIRE(out[0] == Approx(3.0f)); // 1+2
    REQUIRE(out[1] == Approx(5.0f)); // 2+3
}
