#include <catch2/catch_test_macros.hpp>
#include "cli.h"

TEST_CASE("CLI defaults to GUI and fullscreen") {
    const char* argv[] = {"bootloader"};
    CliOptions opts = parseCli(1, const_cast<char**>(argv));
    REQUIRE(opts.useGui == true);
    REQUIRE(opts.fullscreen == true);
}

TEST_CASE("CLI --headless disables GUI") {
    const char* argv[] = {"bootloader", "--headless"};
    CliOptions opts = parseCli(2, const_cast<char**>(argv));
    REQUIRE(opts.useGui == false);
}

TEST_CASE("CLI --no-gui is also headless") {
    const char* argv[] = {"bootloader", "--no-gui"};
    CliOptions opts = parseCli(2, const_cast<char**>(argv));
    REQUIRE(opts.useGui == false);
}

TEST_CASE("CLI --windowed disables fullscreen") {
    const char* argv[] = {"bootloader", "--windowed"};
    CliOptions opts = parseCli(2, const_cast<char**>(argv));
    REQUIRE(opts.fullscreen == false);
}

TEST_CASE("CLI explicit --fullscreen overrides --windowed") {
    const char* argv[] = {"bootloader", "--windowed", "--fullscreen"};
    CliOptions opts = parseCli(3, const_cast<char**>(argv));
    REQUIRE(opts.fullscreen == true);
}

TEST_CASE("CLI parses --telemetry-level value") {
    const char* argv[] = {"bootloader", "--telemetry-level=none"};
    CliOptions opts = parseCli(2, const_cast<char**>(argv));
    REQUIRE(opts.telemetryLevel == TelemetryLevel::NONE);
}

TEST_CASE("CLI parses --max-gen and --profile") {
    const char* argv[] = {"bootloader", "--max-gen", "5", "--profile"};
    CliOptions opts = parseCli(4, const_cast<char**>(argv));
    REQUIRE(opts.maxGen == 5);
    REQUIRE(opts.profile == true);
}

TEST_CASE("CLI accepts mutation-strategy and heuristic") {
    const char* argv[] = {"bootloader", "--mutation-strategy=blacklist", "--heuristic", "blacklist"};
    CliOptions opts = parseCli(4, const_cast<char**>(argv));
    REQUIRE(opts.mutationStrategy == MutationStrategy::BLACKLIST);
    REQUIRE(opts.heuristic == HeuristicMode::BLACKLIST);
}

TEST_CASE("CLI recognizes heuristic=decay", "[cli]") {
    const char* argv[] = {"bootloader", "--heuristic=decay"};
    CliOptions opts = parseCli(2, const_cast<char**>(argv));
    REQUIRE(opts.heuristic == HeuristicMode::DECAY);
}

TEST_CASE("CLI ignores unknown options but still records others") {
    const char* argv[] = {"bootloader", "--foo", "--windowed", "--bar"};
    CliOptions opts = parseCli(4, const_cast<char**>(argv));
    REQUIRE(opts.fullscreen == false);
    REQUIRE(opts.parseError == true);
}

TEST_CASE("CLI parseError is set on invalid numeric and enum values") {
    const char* argv1[] = {"bootloader", "--max-gen=xyz"};
    CliOptions o1 = parseCli(2, const_cast<char**>(argv1));
    REQUIRE(o1.parseError == true);

    const char* argv2[] = {"bootloader", "--telemetry-level=bad"};
    CliOptions o2 = parseCli(2, const_cast<char**>(argv2));
    REQUIRE(o2.parseError == true);
}

TEST_CASE("CLI telemetry-dir is accepted but not validated") {
    const char* argv[] = {"bootloader", "--telemetry-dir", "///not/a/real/path"};
    CliOptions o = parseCli(3, const_cast<char**>(argv));
    REQUIRE(o.telemetryDir == "///not/a/real/path");
    REQUIRE(!o.parseError);
}

TEST_CASE("CLI parses save-model and load-model paths") {
    const char* argv[] = {"bootloader", "--save-model", "model.dat", "--load-model=prev.bin"};
    CliOptions opts = parseCli(4, const_cast<char**>(argv));
    REQUIRE(opts.saveModelPath == "model.dat");
    REQUIRE(opts.loadModelPath == "prev.bin");
}
