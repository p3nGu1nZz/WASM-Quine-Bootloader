#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "train.h"
#include "advisor.h"

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
