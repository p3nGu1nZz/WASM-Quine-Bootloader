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
