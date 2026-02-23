#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
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

    // any sequence should yield a score between 0 and 1
    std::vector<uint8_t> seq{1,2,3};
    float sc = adv.score(seq);
    REQUIRE(sc >= 0.0f);
    REQUIRE(sc <= 1.0f);

    // construct a simple advisor with known generations to check formula
    fs::path root2 = fs::temp_directory_path() / "advtest2";
    fs::remove_all(root2);
    // topology requires a run subdirectory
    fs::create_directories(root2 / "runX");
    writeExport(root2 / "runX" / "gen_a.txt", 0, "A");
    writeExport(root2 / "runX" / "gen_b.txt", 20, "B");
    Advisor adv2(root2.string());
    float sc2 = adv2.score(seq);
    // average = 10 -> score = avg/(avg+10) = 0.5
    REQUIRE(sc2 == Approx(0.5f));
    fs::remove_all(root2);

    // empty advisor returns top score
    Advisor emptyAdv(root.string() + "/nonexistent");
    REQUIRE(emptyAdv.size() == 0);
    REQUIRE(emptyAdv.score(seq) == Approx(1.0f));

    fs::remove_all(root);
}
