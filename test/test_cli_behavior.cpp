#include <catch2/catch_test_macros.hpp>
#include "app.h"
#include "cli.h"
#include <filesystem>

TEST_CASE("App honors max-gen and stops updating", "[app][cli]") {
    CliOptions opts;
    opts.maxGen = 2;
    App a(opts);
    // simulate a couple of reboots using doReboot
    a.doReboot(true);
    REQUIRE(a.generation() == 1);
    REQUIRE(a.update() == true);
    a.doReboot(true);
    REQUIRE(a.generation() == 2);
    // next update should return false due to maxGen
    REQUIRE(a.update() == false);
}

TEST_CASE("autoExport respects telemetry-level", "[export][cli]") {
    namespace fs = std::filesystem;
    CliOptions opts;
    opts.telemetryLevel = TelemetryLevel::BASIC;
    opts.telemetryDir = "test_seq";

    fs::remove_all(opts.telemetryDir);
    App a(opts);
    a.doReboot(true);
    fs::path base = fs::path(opts.telemetryDir) / a.runId();
    REQUIRE(fs::exists(base));
    std::ifstream fin((base / "gen_1.txt").string());
    REQUIRE(fin.good());
    std::string report((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    REQUIRE(report.find("Final Generation") != std::string::npos);
    // BASIC level shouldn't include "CURRENT KERNEL"
    REQUIRE(report.find("CURRENT KERNEL") == std::string::npos);

    // if none, file is absent
    opts.telemetryLevel = TelemetryLevel::NONE;
    App b(opts);
    b.doReboot(true);
    fs::path base2 = fs::path(opts.telemetryDir) / b.runId();
    // base2 may reside in old directory; remove everything before
    fs::remove_all(opts.telemetryDir);
    REQUIRE(!fs::exists(base2 / "gen_1.txt"));
}

TEST_CASE("profile flag outputs log entries", "[cli]") {
    CliOptions opts;
    opts.profile = true;
    App a(opts);
    // simulate two generations so profiling triggers
    a.doReboot(true);
    a.doReboot(true);
    // log buffer should contain at least one PROFILE line
    bool found = false;
    for (auto& e : a.logs()) {
        if (e.message.find("PROFILE") != std::string::npos) { found = true; break; }
    }
    REQUIRE(found);
}

