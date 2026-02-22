#include <catch2/catch_test_macros.hpp>
#include "wasm/evolution.h"
#include "constants.h"
#include "cli.h"
#include <algorithm>

// Note: we rely on KERNEL_GLOB being a valid base64-encoded WASM module
// defined in constants.h.

TEST_CASE("SMART mutation strategy prefers known instructions", "[evolution]") {
    std::vector<std::vector<uint8_t>> known = {{0x05}, {0x06}, {0x07}};
    int smartCount = 0, randCount = 0;

    // run a bunch of iterations to gather statistics
    for (int i = 0; i < 200; i++) {
        auto res = evolveBinary(KERNEL_GLOB, known, i, MutationStrategy::SMART);
        if (std::find(known.begin(), known.end(), res.mutationSequence) != known.end())
            smartCount++;
        auto res2 = evolveBinary(KERNEL_GLOB, known, i, MutationStrategy::RANDOM);
        if (std::find(known.begin(), known.end(), res2.mutationSequence) != known.end())
            randCount++;
    }

    REQUIRE(smartCount > randCount);
}
