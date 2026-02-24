#include "gui/window.h"
#include "gui/colors.h"
#include "util.h"
#include "wasm/parser.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

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
    // on very large monitors.  The scale is clamped to 2.0 to keep text
    // legible without becoming overwhelming.
    const float UI_BOOST = 1.25f;
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
    // Sync auto-scroll flags
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
        // show small advisor panel on right side
        ImGui::SameLine();
        renderAdvisorPanel(app, 300.0f * m_uiScale, panelH);
    }

    m_heatmap.renderPanel(app, winW);
    renderStatusBar(app);

    if (m_monoFont) ImGui::PopFont();
    ImGui::End();

    // Use a neutral background color (era theming removed)
    ImVec4 bg = { 0.01f, 0.03f, 0.10f, 1.0f };
    SDL_SetRenderDrawColorFloat(m_renderer, bg.x, bg.y, bg.z, 1.0f);
    SDL_RenderClear(m_renderer);
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
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

void Gui::renderStatusBar(const App& app) {
    ImGui::Separator();
    ImGui::TextDisabled("WASM-QUINE-BOOTLOADER_SYS v2.4 // STATUS: %s",
                        app.isPaused() ? "PAUSED" : "RUNNING");
    if (app.instanceCount() > 0) {
        ImGui::SameLine(0,20);
        ImGui::Text("Instances: %d", app.instanceCount());
    }
}
