#include <catch2/catch_test_macros.hpp>
#include "wasm/evolution.h"
#include "constants.h"
#include "cli.h"
#include "base64.h"
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

TEST_CASE("evolveBinary handles very large sequences gracefully", "[evolution][error]") {
    std::string base = KERNEL_GLOB;
    // large mutation sequence; behavior may be throw or successful but bounded
    std::vector<uint8_t> longSeq(33000, 0x01);
    int seed = 3;
    bool threw = false;
    try {
        auto res = evolveBinary(base, {longSeq}, seed, MutationStrategy::RANDOM);
        auto decoded = base64_decode(res.binary);
        REQUIRE(decoded.size() <= 35000); // should not grow unbounded
        // still valid magic header
        REQUIRE(decoded[0] == 0x00);
    } catch (const std::exception&) {
        threw = true;
    }
    // success if we reach here without crashing (either threw or returned)
}

TEST_CASE("evolveBinary produces valid magic header", "[evolution]") {
    auto res = evolveBinary(KERNEL_GLOB, {}, 42, MutationStrategy::RANDOM);
    auto decoded = base64_decode(res.binary);
    REQUIRE(decoded.size() >= 4);
    REQUIRE(decoded[0] == 0x00);
    REQUIRE(decoded[1] == 0x61);
    REQUIRE(decoded[2] == 0x73);
    REQUIRE(decoded[3] == 0x6D);
}

TEST_CASE("evolveBinary rejects missing code section", "[evolution][error]") {
    // base32 of minimal header only (00 61 73 6D 01 00 00 00)
    std::string bad = "AGFzbQEAAAAB"; // harness may treat as invalid
    REQUIRE_THROWS_AS(evolveBinary(bad, {}, 0, MutationStrategy::RANDOM), std::runtime_error);
}

TEST_CASE("evolveBinary handles truncated function body", "[evolution][error]") {
    // take valid kernel but truncate bytes
    auto bytes = base64_decode(KERNEL_GLOB);
    if (bytes.size() > 20) {
        bytes.resize(20);
        std::string truncated = base64_encode(bytes);
        bool threw = false;
        try {
            evolveBinary(truncated, {}, 0, MutationStrategy::RANDOM);
        } catch (const std::exception&) {
            threw = true;
        }
        REQUIRE(threw);
    }
}

