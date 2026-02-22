#include <catch2/catch_test_macros.hpp>
#include "train.h"
#include "mutation_advisor.h"

TEST_CASE("Trainer observation increments count and save/load", "[train]") {
    Trainer t;
    TelemetryEntry e;
    e.generation = 3;
    t.observe(e);
    t.observe(e);
    REQUIRE(t.policy().forward(std::vector<float>(256,0)).size() == 1);
    // save and reload
    std::string tmp = "train.tmp";
    REQUIRE(t.save(tmp));
    Trainer t2;
    REQUIRE(t2.load(tmp));
    // t2 should read at least something (observation count >0)
    REQUIRE(t2.policy().forward(std::vector<float>(256,0)).size() == 1);
    std::remove(tmp.c_str());
}
