#include <catch2/catch_test_macros.hpp>
#include "app.h"

TEST_CASE("Blacklist helper methods behave correctly", "[app]") {
    CliOptions opts;
    opts.heuristic = HeuristicMode::BLACKLIST;
    App a(opts);
    std::vector<uint8_t> seq = {0x01, 0x02, 0x03};
    REQUIRE(!a.isBlacklisted(seq));
    a.addToBlacklist(seq);
    REQUIRE(a.isBlacklisted(seq));
    // adding again should not duplicate or change weight
    a.addToBlacklist(seq);
    REQUIRE(a.isBlacklisted(seq));
}

TEST_CASE("handleBootFailure records trap reason and blacklists mutation", "[app]") {
    CliOptions opts;
    opts.heuristic = HeuristicMode::BLACKLIST;
    App a(opts);
    std::vector<uint8_t> seq = {0x10, 0x20};
    a.test_simulateFailure("trap XYZ", seq);
    REQUIRE(a.lastTrapReason() == "trap XYZ");
    REQUIRE(a.isBlacklisted(seq));
}

TEST_CASE("exportHistory includes trap reason after failure", "[export]") {
    CliOptions opts;
    opts.heuristic = HeuristicMode::BLACKLIST;
    App a(opts);
    a.test_simulateFailure("oops", {0x55});
    std::string report = a.exportHistory();
    REQUIRE(report.find("Traps: oops") != std::string::npos);
}

TEST_CASE("heuristic decay reduces blacklist over generations", "[app][heuristic]") {
    CliOptions opts;
    opts.heuristic = HeuristicMode::DECAY;
    App a(opts);
    std::vector<uint8_t> seq = {0xAA, 0xBB};
    a.addToBlacklist(seq);
    REQUIRE(a.isBlacklisted(seq));
    // three successful reboots should remove weight=3 entry
    a.doReboot(true);
    REQUIRE(a.isBlacklisted(seq));
    a.doReboot(true);
    REQUIRE(a.isBlacklisted(seq));
    a.doReboot(true);
    REQUIRE(!a.isBlacklisted(seq));
}

