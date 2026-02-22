#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "util.h"
#include "gui/window.h"
#include "app.h"
#include <filesystem>

TEST_CASE("dpiScaleFromDpi returns 1.0 at 96 dpi", "[dpi]") {
    REQUIRE(dpiScaleFromDpi(96.0f) == 1.0f);
}

TEST_CASE("dpiScaleFromDpi doubles at 192 dpi", "[dpi]") {
    REQUIRE(dpiScaleFromDpi(192.0f) == 2.0f);
}

TEST_CASE("computeDpiScale handles null window gracefully", "[dpi]") {
    REQUIRE(computeDpiScale(nullptr) == 1.0f);
}

TEST_CASE("computeDpiScale increases with window size", "[dpi]") {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* w1 = SDL_CreateWindow("t1", 1400,900, 0);
    SDL_Window* w2 = SDL_CreateWindow("t2", 2800,1800, 0);
    REQUIRE(computeDpiScale(w1) == 1.0f);
    REQUIRE(computeDpiScale(w2) > computeDpiScale(w1));
    SDL_DestroyWindow(w1);
    SDL_DestroyWindow(w2);
    SDL_Quit();
}

TEST_CASE("Gui init applies dpi & ui scales", "[dpi]") {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* w = SDL_CreateWindow("dpi", 1400,900, 0);
    SDL_Renderer* r = SDL_CreateRenderer(w, nullptr);
    Gui gui;
    gui.init(w, r);
    REQUIRE(gui.dpiScale() == computeDpiScale(w));
    REQUIRE(gui.uiScale() >= gui.dpiScale());
    // ImGui global scale should reflect the uiScale value
    ImGuiIO& io = ImGui::GetIO();
    REQUIRE(io.FontGlobalScale == gui.uiScale());
    gui.shutdown();
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}

TEST_CASE("App auto-export creates session files", "[export]") {
    namespace fs = std::filesystem;
    // use a temporary working directory so we don't pollute repo
    fs::path orig = fs::current_path();
    fs::path tmp = orig / "test_run";
    fs::remove_all(tmp);
    fs::create_directories(tmp);
    fs::current_path(tmp);

    App app;
    // simulate one successful reboot/generation
    app.doReboot(true);
    // default path is rooted at executable directory (not current_path)
    fs::path seqdir = sequenceDir(app.runId());
    REQUIRE(fs::exists(seqdir));
    REQUIRE(fs::exists(seqdir / "gen_1.txt"));
    // default telemetry level is BASIC, which does not write kernel blob
    REQUIRE(!fs::exists(seqdir / "kernel_1.b64"));

    // create stray files at root and confirm constructor cleans them up
    std::ofstream stray1("bootloader_old.log");
    std::ofstream stray2("quine_telemetry_gen999.txt");
    stray1 << "x";
    stray2 << "x";
    stray1.close();
    stray2.close();
    App app2;  // construction should purge stray files
    REQUIRE(!fs::exists("bootloader_old.log"));
    REQUIRE(!fs::exists("quine_telemetry_gen999.txt"));

    // cleanup
    fs::current_path(orig);
    fs::remove_all(tmp);
}

TEST_CASE("executableDir returns non-empty string", "[util]") {
    std::string ed = executableDir();
    REQUIRE(!ed.empty());
    // should be absolute path
    REQUIRE(ed[0] == '/');
}
