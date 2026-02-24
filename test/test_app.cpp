#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include <filesystem>
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

TEST_CASE("App loads trainer state from CLI option", "[app][trainer]") {
    // prepare a trainer state file
    Trainer t;
    TelemetryEntry e; e.generation = 1;
    t.observe(e);
    std::string tmp = "app_trainer.tmp";
    REQUIRE(t.save(tmp));

    CliOptions opts;
    opts.loadModelPath = tmp;
    App a(opts);
    // the app's trainer should have the same forward value as t
    auto orig = t.policy().forward(std::vector<float>(256,0));
    auto loaded = a.trainer().policy().forward(std::vector<float>(256,0));
    REQUIRE(loaded.size() == orig.size());
    REQUIRE(loaded[0] == Approx(orig[0]));

    std::remove(tmp.c_str());
}

TEST_CASE("App saves trainer state to CLI option", "[app][trainer]") {
    CliOptions opts;
    opts.saveModelPath = "trainer_save.tmp";
    App a(opts);
    // manually invoke the training/write logic that the app would execute
    TelemetryEntry te;
    te.generation = a.generation();
    te.kernelBase64 = a.currentKernel();
    te.trapCode = a.lastTrapReason();
    a.trainAndMaybeSave(te);
    REQUIRE(std::filesystem::exists("trainer_save.tmp"));
    Trainer t;
    REQUIRE(t.load("trainer_save.tmp"));
    std::remove("trainer_save.tmp");
}

