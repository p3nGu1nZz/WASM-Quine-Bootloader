#include "gui/window.h"
#include "gui/colors.h"
#include "util.h"
#include "wasm/parser.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>

// ─── Gui lifecycle ────────────────────────────────────────────────────────────

void Gui::init(SDL_Window* window, SDL_Renderer* renderer) {
    m_renderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply DPI scaling before loading fonts so the chosen font size is scaled
    // appropriately.  We also expose the raw DPI scale and the final UI scale
    // via members for testing and layout decisions.
    m_dpiScale = computeDpiScale(window);
    // Boost the raw scale slightly to make fonts/buttons large enough for
    // touch screens and maintain the sci-fi look, but avoid runaway sizes
    // on very large monitors.  Empirically the previous boost was too
    // aggressive on Linux, so we scale it back by about 35% (="UI_BOOST"
    // now ≈0.8125).  The final value is still clamped to [1.0, 2.0].
    const float UI_BOOST = 1.25f * 0.65f; // ~0.8125
    m_uiScale = std::max(1.0f, m_dpiScale * UI_BOOST);
    if (m_uiScale > 2.0f) m_uiScale = 2.0f;
    io.FontGlobalScale = m_uiScale;

    // Customize styling for a darker, neon-accented 'cyberpunk' look.
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = { 8.0f * m_uiScale, 6.0f * m_uiScale };
    style.ItemSpacing  = { 8.0f * m_uiScale, 6.0f * m_uiScale };
    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 1.0f;
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 0.0f;
    style.Colors[ImGuiCol_Button]          = {0.15f, 0.00f, 0.25f, 1.0f};
    style.Colors[ImGuiCol_ButtonHovered]   = {0.45f, 0.10f, 0.80f, 1.0f};
    style.Colors[ImGuiCol_ButtonActive]    = {0.60f, 0.20f, 1.00f, 1.0f};
    style.Colors[ImGuiCol_Header]          = {0.00f, 0.40f, 0.90f, 0.8f};
    style.Colors[ImGuiCol_HeaderHovered]   = {0.00f, 0.55f, 1.00f, 0.8f};
    style.Colors[ImGuiCol_HeaderActive]    = {0.00f, 0.70f, 1.00f, 0.8f};
    style.Colors[ImGuiCol_Text]            = {0.80f, 0.80f, 0.85f, 1.0f};
    style.Colors[ImGuiCol_WindowBg]        = {0.02f, 0.02f, 0.05f, 1.0f};

    // Load a monospace font; fall back to ImGui default if none found
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        nullptr
    };
    m_monoFont = nullptr;
    for (int i = 0; fontPaths[i]; i++) {
        m_monoFont = io.Fonts->AddFontFromFileTTF(fontPaths[i], 13.0f * m_uiScale);
        if (m_monoFont) break;
    }
    if (!m_monoFont)
        m_monoFont = io.Fonts->AddFontDefault();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void Gui::shutdown() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

// ─── Frame ───────────────────────────────────────────────────────────────────

void Gui::renderFrame(App& app) {
    // Sync auto-scroll flags (only relevant for EVOLUTION scene)
    if (app.programCounter() != m_lastIP) {
        m_scrollInstrs = true;
        m_lastIP       = app.programCounter();
    }
    if (app.logs().size() != m_lastLogSz) {
        m_scrollLogs = true;
        m_lastLogSz  = app.logs().size();
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int winW, winH;
    SDL_GetWindowSize(SDL_GetRenderWindow(m_renderer), &winW, &winH);

    if (m_scene == GuiScene::TRAINING) {
        renderTrainingScene(app, winW, winH);
    } else {
        // automatically switch to training if evolution has been disabled
        if (!app.evolutionEnabled() && app.trainingPhase() != TrainingPhase::COMPLETE) {
            m_scene = GuiScene::TRAINING;
        }
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize({ (float)winW, (float)winH });
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("##Root", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);

        if (m_monoFont) ImGui::PushFont(m_monoFont);

        renderTopBar(app, winW);

        float headerH = ImGui::GetCursorPosY();
        float footerH = 130.0f * m_uiScale;
        float panelH  = (float)winH - headerH - footerH;
        float logW    = (float)winW * 0.40f;
        float instrW  = 240.0f * m_uiScale;
        float kernelW = (float)winW - logW - instrW;

        renderLogPanel(app, logW, panelH);
        ImGui::SameLine();
        renderInstrPanel(app, instrW, panelH);
        ImGui::SameLine();
        renderKernelPanel(app, kernelW, panelH);

        if (m_showAdvisor) {
            ImGui::SameLine();
            renderAdvisorPanel(app, 300.0f * m_uiScale, panelH);
        }

        if (app.instanceCount() > 0) {
            ImGui::SameLine();
            renderInstancesPanel(app, 260.0f * m_uiScale, panelH);
        }

        // weight heatmaps per layer appear just above the memory panel
        renderWeightHeatmaps(app, winW);

        m_heatmap.renderPanel(app, winW);
        renderStatusBar(app);

        if (m_monoFont) ImGui::PopFont();
        ImGui::End();
    }

    // Use a neutral background color
    ImVec4 bg = { 0.01f, 0.03f, 0.10f, 1.0f };
    SDL_SetRenderDrawColorFloat(m_renderer, bg.x, bg.y, bg.z, 1.0f);
    SDL_RenderClear(m_renderer);
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
}

// ─── Training scene ──────────────────────────────────────────────────────────

void Gui::renderTrainingScene(App& app, int winW, int winH) {
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)winW, (float)winH });
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##TrainRoot", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);

    if (m_monoFont) ImGui::PushFont(m_monoFont);

    // ── Header ───────────────────────────────────────────────────────────────
    ImGui::Separator();
    ImGui::TextColored({ 0.11f, 0.83f, 0.93f, 1.0f }, "QUINEOS v2.0.4");
    ImGui::SameLine(0, 30);
    ImGui::TextColored({ 0.78f, 0.50f, 0.98f, 1.0f }, "NEURAL NETWORK RL TRAINING DASHBOARD");
    ImGui::Separator();

    // ── Central panel ────────────────────────────────────────────────────────
    float panelW = std::min((float)winW - 40.0f, 900.0f * m_uiScale);
    float panelX = ((float)winW - panelW) * 0.5f;
    ImGui::SetCursorPosX(panelX);

    ImGui::BeginChild("##TrainPanel", { panelW, (float)winH - 100.0f * m_uiScale }, true);

    // Phase indicator
    const char* phaseStr = "LOADING TELEMETRY";
    ImVec4      phaseCol = { 0.39f, 0.66f, 0.97f, 1.0f };
    if (app.trainingPhase() == TrainingPhase::TRAINING) {
        phaseStr = "TRAINING POLICY";
        phaseCol = { 0.98f, 0.82f, 0.10f, 1.0f };
    } else if (app.trainingPhase() == TrainingPhase::COMPLETE) {
        phaseStr = "TRAINING COMPLETE";
        phaseCol = { 0.29f, 0.87f, 0.38f, 1.0f };
    }
    ImGui::TextColored(phaseCol, "PHASE: %s", phaseStr);
    ImGui::Spacing();

    // ── Side-by-side: telemetry stats | policy architecture ──────────────────
    float halfW = (panelW - 30.0f) * 0.5f;

    ImGui::BeginChild("##TelStats", { halfW, 130.0f * m_uiScale }, true);
    ImGui::TextDisabled("TELEMETRY DATA");
    ImGui::Separator();
    ImGui::Text("Entries loaded : %d", (int)app.advisor().entryCount());
    ImGui::Text("Observations   : %d", app.trainer().observations());
    if (app.advisor().entryCount() > 0) {
        float avgGen = 0.0f;
        for (const auto& e : app.advisor().entries())
            avgGen += static_cast<float>(e.generation);
        avgGen /= static_cast<float>(app.advisor().entryCount());
        ImGui::Text("Avg generation : %.1f", avgGen);
        float score = app.advisor().score({});
        ImGui::Text("Advisor score  : %.3f", score);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##PolicyArch", { halfW, 130.0f * m_uiScale }, true);
    ImGui::TextDisabled("POLICY NETWORK");
    ImGui::Separator();
    const Policy& pol = app.trainer().policy();
    int totalParams = 0;
    for (int l = 0; l < pol.layerCount(); ++l) {
        int w = pol.layerInSize(l) * pol.layerOutSize(l);
        int b = pol.layerOutSize(l);
        totalParams += w + b;
        ImGui::Text("Layer %d: %4d -> %-4d  (Dense)", l, pol.layerInSize(l), pol.layerOutSize(l));
    }
    ImGui::Separator();
    ImGui::Text("Total params   : %d", totalParams);
    if (app.trainer().observations() > 0)
        ImGui::Text("Avg loss (EMA) : %.6f", app.trainer().avgLoss());
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();

    // ── Overall progress bar ──────────────────────────────────────────────────
    float prog = app.trainingProgress();
    char overlay[64];
    std::snprintf(overlay, sizeof(overlay), "%.0f%%  (%d obs)",
                  prog * 100.0f, app.trainer().observations());
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, { 0.11f, 0.83f, 0.93f, 0.85f });
    ImGui::ProgressBar(prog, { -1.0f, 22.0f * m_uiScale }, overlay);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Advisor entry table ───────────────────────────────────────────────────
    ImGui::TextDisabled("RECENT TELEMETRY ENTRIES");
    ImGui::Spacing();

    if (app.advisor().entryCount() == 0) {
        // show the actual telemetry root so users running from different
        // working directories know where data is being searched for.  the
        // path is derived from the executable location (see App::telemetryRoot()).
        std::string telemetry = app.telemetryRootPublic().string();
        if (!telemetry.empty()) {
            // add trailing slash for readability
            if (telemetry.back() != '/' && telemetry.back() != '\\')
                telemetry += '/';
            ImGui::TextDisabled("No prior run sequences found in %s", telemetry.c_str());
        } else {
            ImGui::TextDisabled("No prior run sequences found (telemetry root unknown)");
        }
        ImGui::TextDisabled("Training will begin fresh once evolution starts.");
    } else {
        // Table header
        ImGui::TextDisabled("%-6s  %-20s  %s", "GEN", "TRAP", "KERNEL (first 40 chars)");
        ImGui::Separator();
        ImGui::BeginChild("##EntryScroll", { 0, 160.0f * m_uiScale }, false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        const auto& entries = app.advisor().entries();
        // show at most 50 most-recent entries
        int startIdx = (int)entries.size() > 50 ? (int)entries.size() - 50 : 0;
        for (int i = startIdx; i < (int)entries.size(); ++i) {
            const auto& e = entries[i];
            std::string ker = e.kernelBase64.size() > 40
                              ? e.kernelBase64.substr(0, 40) + "..."
                              : e.kernelBase64;
            std::string trap = e.trapCode.empty() ? "none" : e.trapCode;
            ImVec4 col = e.trapCode.empty()
                         ? ImVec4{ 0.29f, 0.87f, 0.38f, 1.0f }
                         : ImVec4{ 0.96f, 0.26f, 0.21f, 1.0f };
            ImGui::TextColored(col, "%-6d  %-20s  %s",
                               e.generation, trap.c_str(), ker.c_str());
        }
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // if training completes, immediately transition back to evolution
    if (app.trainingDone()) {
        app.enableEvolution();
        m_scene = GuiScene::EVOLUTION;
        // fall through; we still want to draw status bar below
    }

    ImGui::EndChild();

    // ── Status bar ────────────────────────────────────────────────────────────
    ImGui::SetCursorPosY((float)winH - 30.0f * m_uiScale);
    ImGui::Separator();
    ImGui::TextDisabled("WASM-QUINE-BOOTLOADER_SYS v2.4 // STARTUP TRAINING");

    if (m_monoFont) ImGui::PopFont();
    ImGui::End();
}

// ─── Panel implementations ───────────────────────────────────────────────────

void Gui::renderTopBar(App& app, int winW) {
    ImGui::Separator();
    ImGui::Text("QUINEOS v2.0.4");
    ImGui::SameLine(0, 30);
    ImGui::SameLine(0, 20);
    ImGui::Text("GEN: %04d", app.generation());
    ImGui::SameLine(0, 20);
    ImGui::TextColored(colorForState(app.state()), "STATE: %s",
                       stateStr(app.state()).c_str());
    ImGui::SameLine(0, 20);
    ImGui::Text("UPTIME: %.1fs", app.uptimeSec());
    ImGui::SameLine(0, 20);
    if (app.retryCount() > 0)
        ImGui::TextColored({0.96f, 0.26f, 0.21f, 1}, "RETRIES: %d",
                           app.retryCount());

    ImGui::SameLine();
    float btnW = 140.0f * m_uiScale;
    ImGui::SetCursorPosX((float)winW - btnW - 110);
    if (app.isPaused()) {
        ImGui::PushStyleColor(ImGuiCol_Button, {0.4f, 0.3f, 0.0f, 1});
        if (ImGui::Button("RESUME SYSTEM", {btnW, 0})) app.togglePause();
        ImGui::PopStyleColor();
    } else {
        if (ImGui::Button("PAUSE  SYSTEM", {btnW, 0})) app.togglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Advisor", {btnW, 0})) {
        m_showAdvisor = !m_showAdvisor;
    }
    ImGui::SameLine();
    // EXPORT button removed – telemetry is now saved automatically every
    // generation under bin/seq/<runid>/gen_<n>.txt (see App autoExport()).
    ImGui::Separator();
}

void Gui::renderLogPanel(const App& app, float w, float h) {
    ImGui::BeginChild("##LogPanel", { w, h }, true,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("SYSTEM LOG  BUF:%d", (int)app.logs().size());
    // filter input
    ImGui::SameLine();
    ImGui::SetCursorPosX(w - 200.0f * m_uiScale);
    // InputText doesn't accept std::string in this build, so use fixed buffer.
    char buf[256];
    std::strncpy(buf, m_logFilter.c_str(), sizeof(buf));
    if (ImGui::InputText("Filter", buf, sizeof(buf))) {
        m_logFilter = buf;
    }

    ImGui::Separator();
    ImGui::BeginChild("##LogScroll", { 0, 0 }, false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : app.logs()) {
        if (!m_logFilter.empty()) {
            if (log.message.find(m_logFilter) == std::string::npos)
                continue;
        }
        uint64_t t = log.timestamp;
        int ms = (int)(t % 1000);
        int s  = (int)((t / 1000)   % 60);
        int m  = (int)((t / 60000)  % 60);
        int hr = (int)(t / 3600000);
        char ts[20];
        std::snprintf(ts, sizeof ts, "%02d:%02d:%02d.%03d", hr, m, s, ms);
        ImGui::TextDisabled("%s", ts);
        ImGui::SameLine();
        if (log.type == "system")
            ImGui::TextColored(colorForLogType(log.type), "-> %s",
                               log.message.c_str());
        else
            ImGui::TextColored(colorForLogType(log.type), "%s",
                               log.message.c_str());
    }
    if (m_scrollLogs) {
        ImGui::SetScrollHereY(1.0f);
        m_scrollLogs = false;
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5)
        m_scrollLogs = true;
    ImGui::EndChild();
    ImGui::EndChild();
}

void Gui::renderInstrPanel(const App& app, float w, float h) {
    ImGui::BeginChild("##InstrPanel", { w, h }, true,
                      ImGuiWindowFlags_NoScrollbar);
    int ip = app.programCounter();
    ImGui::TextDisabled("INSTRUCTION STACK");
    ImGui::SameLine();
    if (ip >= 0) ImGui::TextColored({0.29f, 0.87f, 0.38f, 1}, "IP:%03d", ip);
    else         ImGui::TextDisabled("IP:WAIT");
    ImGui::Separator();

    ImGui::BeginChild("##InstrScroll", { 0, 0 }, false);
    const auto& instrs = app.instructions();
    for (int i = 0; i < (int)instrs.size(); i++) {
        const auto& inst   = instrs[i];
        bool        active = (i == ip);
        std::string name   = getOpcodeName(inst.opcode);

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
            ImGui::TextUnformatted("-> ");
            ImGui::SameLine();
        } else {
            ImGui::TextDisabled("   ");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 120, 120, 180));
        }

        if (!inst.args.empty()) {
            std::ostringstream ss;
            for (uint8_t a : inst.args)
                ss << " 0x" << std::uppercase << std::hex << (int)a;
            ImGui::Text("%-12s%s", name.c_str(), ss.str().c_str());
        } else {
            ImGui::TextUnformatted(name.c_str());
        }
        ImGui::PopStyleColor();

        if (active && m_scrollInstrs) {
            ImGui::SetScrollHereY(0.5f);
            m_scrollInstrs = false;
        }
    }
    if (instrs.empty())
        ImGui::TextDisabled("Waiting for Kernel...");
    ImGui::EndChild();
    ImGui::EndChild();
}

void Gui::renderKernelPanel(const App& app, float w, float h) {
    ImGui::BeginChild("##KernelPanel", { w, h }, true,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("KERNEL SOURCE (BASE64)");
    ImGui::SameLine();
    if (ImGui::SmallButton("COPY"))
        ImGui::SetClipboardText(app.currentKernel().c_str());
    ImGui::Separator();

    ImGui::TextColored({0.13f, 0.83f, 0.93f, 1}, "%zuB",  app.kernelBytes());
    ImGui::SameLine(70);
    ImGui::TextColored({0.78f, 0.5f,  0.98f, 1}, "+%d OPS", app.evolutionAttempts());
    ImGui::SameLine(160);
    ImGui::TextColored({0.29f, 0.87f, 0.38f, 1}, "%d PAT", app.knownInstructionCount());
    ImGui::Separator();

    ImGui::BeginChild("##KernelScroll", { 0, 0 }, false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    {
        const std::string& cur    = app.currentKernel();
        const std::string& stable = app.stableKernel();
        const auto&        instrs = app.instructions();

        int codeStartByte = 0;
        if (!instrs.empty() && instrs[0].originalOffset > 0)
            codeStartByte = instrs[0].originalOffset;
        int codeStartChar = (int)(codeStartByte * 4 / 3);

        ImDrawList* dl     = ImGui::GetWindowDrawList();
        ImVec2      cursor = ImGui::GetCursorScreenPos();
        float       charW  = ImGui::CalcTextSize("A").x;
        float       lineH  = ImGui::GetTextLineHeightWithSpacing();
        float       startX = cursor.x;
        float       maxX   = cursor.x + w - 20;
        float x = startX, y = cursor.y;

        for (int i = 0; i < (int)cur.size(); i++) {
            ImVec4 col;
            if      (i < codeStartChar)         col = { 0.31f, 0.44f, 0.70f, 0.6f };
            else if (i >= (int)stable.size())   col = { 0.29f, 0.87f, 0.38f, 1.0f };
            else if (cur[i] != stable[i])       col = { 0.98f, 0.82f, 0.10f, 1.0f };
            else                                col = { 0.44f, 0.44f, 0.44f, 0.8f };

            char ch[2] = { cur[i], 0 };
            dl->AddText({ x, y },
                IM_COL32((int)(col.x*255),(int)(col.y*255),
                         (int)(col.z*255),(int)(col.w*255)),
                ch);
            x += charW;
            if (x + charW > maxX) { x = startX; y += lineH; }
        }

        ImGui::Dummy({ w - 20, y - cursor.y + lineH + 4 });
        ImGui::TextDisabled("--- END OF FILE ---");
    }
    ImGui::EndChild();
    ImGui::EndChild();
}

void Gui::renderAdvisorPanel(const App& app, float w, float h) {
    ImGui::BeginChild("##AdvisorPanel", { w, h }, true);
    ImGui::TextDisabled("ADVISOR STATE");
    ImGui::Separator();
    ImGui::Text("Entries: %zu", app.advisor().entryCount());
    if (ImGui::Button("Dump state")) {
        std::string path = "advisor_dump.txt";
        if (app.advisor().dump(path)) {
            m_lastDumpPath = path;
        } else {
            m_lastDumpPath = "<error>";
        }
    }
    if (!m_lastDumpPath.empty()) {
        ImGui::Text("Last dump: %s", m_lastDumpPath.c_str());
    }
    ImGui::EndChild();
}

void Gui::renderInstancesPanel(App& app, float w, float h) {
    ImGui::BeginChild("##InstancesPanel", { w, h }, true);
    ImGui::TextDisabled("INSTANCES");
    ImGui::Separator();
    for (int i = 0; i < app.instanceCount(); i++) {
        const std::string& inst = app.instances()[i];
        ImGui::Text("%d: %s", i, inst.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Kill##") + std::to_string(i)).c_str())) {
            app.killInstance(i);
        }
    }
    ImGui::EndChild();
}

// Render heatmaps of each policy layer's weight matrix.  Each layer is shown
// as a small grid of colored cells (red for positive weights, blue for
// negative) stacked vertically.  This panel is drawn in the evolution scene
// above the heap memory heatmap.
void Gui::renderWeightHeatmaps(const App& app, int winW) {
    const Policy& pol = app.trainer().policy();
    int layers = pol.layerCount();
    if (layers == 0) return;

    ImGui::Separator();
    ImGui::TextDisabled("NN WEIGHT HEATMAPS");
    float visW = (float)winW - 20.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int l = 0; l < layers; ++l) {
        int in = pol.layerInSize(l);
        int out = pol.layerOutSize(l);
        const auto& w = pol.layerWeights(l);
        if (in <= 0 || out <= 0) continue;

        float cell = std::min(4.0f, visW / (float)in);
        float layerH = cell * out;
        ImGui::Text("Layer %d (%dx%d)", l, out, in);
        ImGui::Dummy({cell * in, layerH});
        ImVec2 p = ImGui::GetCursorScreenPos();
        for (int i = 0; i < out; ++i) {
            for (int j = 0; j < in; ++j) {
                float val = w[i * in + j];
                float tn = std::tanh(val);
                ImU32 col;
                if (tn >= 0) {
                    col = IM_COL32((int)(tn * 255), 0, 0, 255);
                } else {
                    col = IM_COL32(0, 0, (int)(-tn * 255), 255);
                }
                dl->AddRectFilled({p.x + j * cell, p.y + i * cell},
                                  {p.x + (j + 1) * cell, p.y + (i + 1) * cell},
                                  col);
            }
        }
        ImGui::SetCursorScreenPos({p.x, p.y + layerH + 5.0f});
    }
}

void Gui::renderStatusBar(const App& app) {
    ImGui::Separator();
    ImGui::TextDisabled("WASM-QUINE-BOOTLOADER_SYS v2.4 // STATUS: %s",
                        app.isPaused() ? "PAUSED" : "RUNNING");
    if (app.instanceCount() > 0) {
        ImGui::SameLine(0,20);
        ImGui::Text("Instances: %d", app.instanceCount());
    }
}
