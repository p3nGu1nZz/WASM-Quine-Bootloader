#include <catch2/catch_test_macros.hpp>
#include "app.h"

TEST_CASE("Blacklist helper methods behave correctly", "[app]") {
    App a;
    std::vector<uint8_t> seq = {0x01, 0x02, 0x03};
    REQUIRE(!a.isBlacklisted(seq));
    a.addToBlacklist(seq);
    REQUIRE(a.isBlacklisted(seq));
    // adding again should not duplicate
    a.addToBlacklist(seq);
    REQUIRE(a.isBlacklisted(seq));
}

TEST_CASE("handleBootFailure records trap reason and blacklists mutation", "[app]") {
    App a;
    std::vector<uint8_t> seq = {0x10, 0x20};
    // simulate pending mutation that led to failure
    // use private member via friend or reflection? we can use public accessors or hack
    // we'll cast and modify for test
    a.test_simulateFailure("trap XYZ", seq);
    REQUIRE(a.lastTrapReason() == "trap XYZ");
    REQUIRE(a.isBlacklisted(seq));
}

TEST_CASE("exportHistory includes trap reason after failure", "[export]") {
    App a;
    a.test_simulateFailure("oops", {0x55});
    std::string report = a.exportHistory();
    REQUIRE(report.find("Traps: oops") != std::string::npos);
}

