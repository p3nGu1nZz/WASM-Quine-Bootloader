#include <catch2/catch_test_macros.hpp>
#include "feature.h"
#include "constants.h"
#include "base64.h"

TEST_CASE("Feature extractor produces opcode histogram", "[feature]") {
    TelemetryEntry e;
    // invalid wasm produces no instructions
    e.kernelBase64 = "AQ==";
    auto vec = Feature::extract(e);
    REQUIRE(vec.size() == (size_t)kFeatSize);
    for (auto v : vec) REQUIRE(v == 0.0f);

    // realistic kernel should yield some non-zero counts
    e.kernelBase64 = KERNEL_GLOB;
    vec = Feature::extract(e);
    bool saw_nonzero = false;
    for (auto v : vec) if (v > 0.0f) saw_nonzero = true;
    REQUIRE(saw_nonzero);
    
    // our only check is that some opcode was seen, the exact opcodes
    // present may vary between builds so we avoid tying the test to one.
}
