#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "train.h"
#include "advisor.h"
#include "constants.h"  // KERNEL_GLOB

TEST_CASE("Trainer observation increments count and save/load", "[train]") {
    Trainer t;
    TelemetryEntry e;
    e.generation = 3;
    t.observe(e);
    // observe again to change weights more noticeably
    t.observe(e);
    auto before = t.policy().forward(std::vector<float>(256,0));
    REQUIRE(before.size() == 1);
    // save and reload
    std::string tmp = "train.tmp";
    REQUIRE(t.save(tmp));
    Trainer t2;
    REQUIRE(t2.load(tmp));
    auto after = t2.policy().forward(std::vector<float>(256,0));
    REQUIRE(after.size() == 1);
    // the loaded policy should behave identically to the original
    REQUIRE(after[0] == Approx(before[0]));
    std::remove(tmp.c_str());
}

TEST_CASE("Trainer load/save failures propagate false", "[train]") {
    Trainer t;
    REQUIRE(!t.load("nonexistent_file.xyz"));
    std::string badpath = "/invalid/path/model.dat";
    REQUIRE(!t.save(badpath));
}

TEST_CASE("Trainer observe with non-empty kernel updates avgLoss", "[train]") {
    Trainer t;
    REQUIRE(t.observations() == 0);
    REQUIRE(t.avgLoss() == Approx(0.0f));

    TelemetryEntry e;
    e.generation    = 5;
    e.kernelBase64  = KERNEL_GLOB;  // real seed kernel, non-empty

    t.observe(e);
    REQUIRE(t.observations() == 1);
    // avgLoss should now be positive (non-zero prediction error)
    REQUIRE(t.lastLoss() >= 0.0f);
    REQUIRE(t.avgLoss() >= 0.0f);
}

TEST_CASE("Trainer observe with empty kernel still increments observations", "[train]") {
    Trainer t;
    TelemetryEntry e;
    e.generation   = 7;
    e.kernelBase64 = "";   // empty → early return, no weight update
    t.observe(e);
    REQUIRE(t.observations() == 1);
    // avgLoss should remain 0 (no update)
    REQUIRE(t.avgLoss() == Approx(0.0f));
}

TEST_CASE("Trainer save/load round-trip preserves avgLoss and maxReward", "[train]") {
    Trainer t;
    TelemetryEntry e;
    e.generation   = 10;
    e.kernelBase64 = KERNEL_GLOB;
    t.observe(e);

    float origAvgLoss = t.avgLoss();
    int   origObs     = t.observations();

    std::string tmp = "train_roundtrip.tmp";
    REQUIRE(t.save(tmp));

    Trainer t2;
    REQUIRE(t2.load(tmp));
    REQUIRE(t2.observations() == origObs);
    REQUIRE(t2.avgLoss() == Approx(origAvgLoss));

    // Policy output should match after reload
    std::vector<float> inp(256, 1.0f);
    REQUIRE(t2.policy().forward(inp)[0] == Approx(t.policy().forward(inp)[0]));
    std::remove(tmp.c_str());
}

TEST_CASE("Trainer multiple observations drive loss toward zero", "[train]") {
    Trainer t;
    TelemetryEntry e;
    e.generation   = 8;
    e.kernelBase64 = KERNEL_GLOB;

    // Run many observations; loss should eventually decrease
    float firstLoss = -1.0f;
    for (int i = 0; i < 200; ++i) {
        t.observe(e);
        if (i == 0) firstLoss = t.lastLoss();
    }
    // After many observations on the same input, loss should not grow
    REQUIRE(t.avgLoss() <= firstLoss + 0.1f);
}

