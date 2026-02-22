#include <catch2/catch_test_macros.hpp>
#include "advisor.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static void writeExport(const fs::path& path, int gen, const std::string& kernel,
                        const std::string& trap = "") {
    std::ofstream o(path);
    o << "WASM QUINE BOOTLOADER - SYSTEM HISTORY EXPORT\n";
    o << "Final Generation: " << gen << "\n";
    if (!trap.empty()) o << "Traps: " << trap << "\n";
    o << "CURRENT KERNEL (BASE64):\n";
    o << kernel << "\n";
}

TEST_CASE("Advisor loads entries from telemetry files", "[advisor]") {
    fs::path root = fs::temp_directory_path() / "advtest";
    fs::remove_all(root);
    fs::create_directories(root / "runA");
    writeExport(root / "runA" / "gen_1.txt", 1, "AAA", "oops");
    writeExport(root / "runA" / "gen_2.txt", 2, "BBB");
    // another run subdirectory
    fs::create_directories(root / "runB");
    writeExport(root / "runB" / "gen_5.txt", 5, "CCC", "trap");

    Advisor adv(root.string());
    REQUIRE(adv.size() == 3);
    auto& entries = adv.entries();
    // verify that expected generations exist somewhere in the list
    bool saw1=false, saw2=false, saw5=false;
    for (auto& e : entries) {
        if (e.generation == 1) {
            saw1 = true;
            REQUIRE(e.kernelBase64 == "AAA");
            REQUIRE(e.trapCode == "oops");
        }
        if (e.generation == 2) {
            saw2 = true;
            REQUIRE(e.kernelBase64 == "BBB");
        }
        if (e.generation == 5) {
            saw5 = true;
            REQUIRE(e.kernelBase64 == "CCC");
            REQUIRE(e.trapCode == "trap");
        }
    }
    REQUIRE(saw1);
    REQUIRE(saw2);
    REQUIRE(saw5);

    fs::remove_all(root);
}
