#include "gui.h"
#include "util.h"
#include "wasm_parser.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <random>
#include <cmath>
#include <vector>

// Lightweight per-frame random int in [0, n)
static int guiRandInt(int n) {
    static thread_local std::mt19937 rng(std::random_device{}());
    return std::uniform_int_distribution<int>(0, n - 1)(rng);
}

// ─── Color helpers ────────────────────────────────────────────────────────────

static ImVec4 colorForLogType(const std::string& t) {
    if (t == "success") return { 0.29f, 0.87f, 0.38f, 1.0f };
    if (t == "warning") return { 0.98f, 0.82f, 0.10f, 1.0f };
    if (t == "error")   return { 0.96f, 0.26f, 0.21f, 1.0f };
    if (t == "system")  return { 0.11f, 0.83f, 0.93f, 1.0f };
    if (t == "mutation")return { 0.78f, 0.50f, 0.98f, 1.0f };
    return { 0.63f, 0.63f, 0.63f, 1.0f }; // info
}

static ImVec4 colorForState(SystemState s) {
    switch (s) {
        case SystemState::BOOTING:         return { 0.98f, 0.82f, 0.10f, 1.0f };
        case SystemState::LOADING_KERNEL:  return { 0.39f, 0.66f, 0.97f, 1.0f };
        case SystemState::EXECUTING:       return { 0.29f, 0.87f, 0.38f, 1.0f };
        case SystemState::VERIFYING_QUINE: return { 0.78f, 0.50f, 0.98f, 1.0f };
        case SystemState::SYSTEM_HALT:     return { 0.96f, 0.26f, 0.21f, 1.0f };
        case SystemState::REPAIRING:       return { 0.99f, 0.55f, 0.19f, 1.0f };
        default:                           return { 0.44f, 0.44f, 0.44f, 1.0f };
    }
}

static ImVec4 colorForEra(SystemEra e) {
    switch (e) {
        case SystemEra::EXPANSION:   return { 0.24f, 0.87f, 0.56f, 1.0f };
        case SystemEra::COMPLEXITY:  return { 0.78f, 0.50f, 0.98f, 1.0f };
        case SystemEra::SINGULARITY: return { 0.96f, 0.26f, 0.21f, 1.0f };
        default:                     return { 0.50f, 0.50f, 0.50f, 1.0f };
    }
}

static ImVec4 bgColorForEra(SystemEra e) {
    switch (e) {
        case SystemEra::EXPANSION:   return { 0.01f, 0.10f, 0.06f, 1.0f };
        case SystemEra::COMPLEXITY:  return { 0.06f, 0.02f, 0.12f, 1.0f };
        case SystemEra::SINGULARITY: return { 0.10f, 0.02f, 0.02f, 1.0f };
        default:                     return { 0.01f, 0.03f, 0.10f, 1.0f };
    }
}

// ─── Memory heatmap state ────────────────────────────────────────────────────

static std::vector<float> s_heatMap;

static void drawMemoryHeatmap(const App& app, ImDrawList* dl,
                               ImVec2 pos, ImVec2 size) {
    int kernelSz = (int)app.kernelBytes();
    if (kernelSz == 0) return;

    const int BLOCK = kernelSz < 256 ? 8 : kernelSz < 1024 ? 5 : 3;
    const int GAP   = 1;
    const int STEP  = BLOCK + GAP;
    const int BPB   = kernelSz < 256 ? 1 : kernelSz < 1024 ? 4 : 16;
    const int COLS  = std::max(1, (int)(size.x / STEP));
    const int BLOCKS = (kernelSz + BPB - 1) / BPB;

    if ((int)s_heatMap.size() != BLOCKS)
        s_heatMap.assign(BLOCKS, 0.0f);

    ImVec4 theme, activeC;
    switch (app.era()) {
        case SystemEra::EXPANSION:
            theme   = { 0.02f, 0.31f, 0.24f, 1 };
            activeC = { 0.20f, 0.83f, 0.60f, 1 };
            break;
        case SystemEra::COMPLEXITY:
            theme   = { 0.23f, 0.03f, 0.39f, 1 };
            activeC = { 0.75f, 0.52f, 0.99f, 1 };
            break;
        case SystemEra::SINGULARITY:
            theme   = { 0.27f, 0.04f, 0.04f, 1 };
            activeC = { 0.93f, 0.27f, 0.27f, 1 };
            break;
        default:
            theme   = { 0.07f, 0.16f, 0.23f, 1 };
            activeC = { 0.13f, 0.83f, 0.93f, 1 };
            break;
    }

    bool isActive = (app.state() == SystemState::LOADING_KERNEL ||
                     app.state() == SystemState::EXECUTING);

    for (int i = 0; i < BLOCKS; i++) {
        int   col = i % COLS;
        int   row = i / COLS;
        float x   = pos.x + col * STEP;
        float y   = pos.y + row * STEP;
        if (y + BLOCK > pos.y + size.y) break;

        int  bStart  = i * BPB;
        int  bEnd    = bStart + BPB;
        bool focused = isActive &&
            (bStart < app.focusAddr() + app.focusLen()) &&
            (bEnd   > app.focusAddr());

        if (focused)
            s_heatMap[i] = 1.0f;
        else if (app.isSystemReading() && (guiRandInt(100) > 98))
            s_heatMap[i] = std::min(1.0f, s_heatMap[i] + 0.5f);

        s_heatMap[i] *= 0.85f;
        if (s_heatMap[i] < 0.005f) s_heatMap[i] = 0.0f;

        float heat = s_heatMap[i];

        ImU32 baseCol = IM_COL32(
            (int)(theme.x * 255), (int)(theme.y * 255),
            (int)(theme.z * 255), (int)(0.3f * 255));
        dl->AddRectFilled({ x, y }, { x + BLOCK, y + BLOCK }, baseCol);

        if (heat > 0.01f) {
            float  sz  = BLOCK * (1.0f + heat * 0.6f);
            float  off = (sz - BLOCK) / 2.0f;
            ImVec4 c   = heat > 0.5f
                ? ImVec4(1, 1, 1, heat)
                : ImVec4(activeC.x, activeC.y, activeC.z, heat);
            dl->AddRectFilled(
                { x - off, y - off }, { x - off + sz, y - off + sz },
                IM_COL32((int)(c.x*255),(int)(c.y*255),(int)(c.z*255),(int)(c.w*255)));
        }

        if (app.isMemoryGrowing() && (guiRandInt(100) > 98))
            dl->AddRectFilled({ x, y }, { x + BLOCK, y + BLOCK },
                              IM_COL32(255, 255, 255, 100));
    }
}

// ─── Gui ─────────────────────────────────────────────────────────────────────

void Gui::init(SDL_Window* window, SDL_Renderer* renderer) {
    m_renderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 1.0f;
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 0.0f;

    // Load a monospace font; fall back to ImGui default if none found
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        nullptr
    };
    m_monoFont = nullptr;
    for (int i = 0; fontPaths[i]; i++) {
        m_monoFont = io.Fonts->AddFontFromFileTTF(fontPaths[i], 13.0f);
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

void Gui::renderFrame(App& app) {
    // ── sync scroll trackers ──────────────────────────────────────────────────
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
    float footerH = 130.0f;
    float panelH  = (float)winH - headerH - footerH;
    float logW    = (float)winW * 0.40f;
    float instrW  = 240.0f;
    float kernelW = (float)winW - logW - instrW;

    renderLogPanel(app, logW, panelH);
    ImGui::SameLine();
    renderInstrPanel(app, instrW, panelH);
    ImGui::SameLine();
    renderKernelPanel(app, kernelW, panelH);

    renderMemoryVisualizer(app, winW);
    renderStatusBar(app);

    if (m_monoFont) ImGui::PopFont();
    ImGui::End();

    // Composite frame
    auto bg = bgColorForEra(app.era());
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
    ImGui::TextColored(colorForEra(app.era()), "ERA: %s", eraStr(app.era()).c_str());
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
    float btnW = 140.0f;
    ImGui::SetCursorPosX((float)winW - btnW - 110);
    if (app.isPaused()) {
        ImGui::PushStyleColor(ImGuiCol_Button, {0.4f, 0.3f, 0.0f, 1});
        if (ImGui::Button("RESUME SYSTEM", {btnW, 0})) app.togglePause();
        ImGui::PopStyleColor();
    } else {
        if (ImGui::Button("PAUSE  SYSTEM", {btnW, 0})) app.togglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("EXPORT", {60, 0})) {
        std::string report = app.exportHistory();
        std::string fname  = "quine_telemetry_gen" +
                             std::to_string(app.generation()) + ".txt";
        std::ofstream f(fname);
        if (f) f << report;
    }
    ImGui::Separator();
}

void Gui::renderLogPanel(const App& app, float w, float h) {
    ImGui::BeginChild("##LogPanel", { w, h }, true,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("SYSTEM LOG  BUF:%d", (int)app.logs().size());
    ImGui::Separator();
    ImGui::BeginChild("##LogScroll", { 0, 0 }, false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : app.logs()) {
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

void Gui::renderMemoryVisualizer(const App& app, int winW) {
    ImGui::Separator();
    std::ostringstream hdr;
    hdr << "SYSTEM_MEMORY_MAP // HEAP_VISUALIZER  PTR:0x"
        << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
        << app.focusAddr();
    if (app.isSystemReading())
        hdr << "  [READ]";
    else if (app.state() == SystemState::LOADING_KERNEL ||
             app.state() == SystemState::EXECUTING)
        hdr << "  [WRITE]";
    hdr << "  SIZE:" << std::dec << app.kernelBytes() << "B";
    ImGui::TextDisabled("%s", hdr.str().c_str());

    ImVec2 visPos = ImGui::GetCursorScreenPos();
    float  visW   = (float)winW - 20;
    float  visH   = 80.0f;
    ImGui::Dummy({ visW, visH });
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(visPos, { visPos.x + visW, visPos.y + visH },
                      IM_COL32(5, 8, 18, 240));
    drawMemoryHeatmap(app, dl, visPos, { visW, visH });
}

void Gui::renderStatusBar(const App& app) {
    ImGui::Separator();
    ImGui::TextDisabled(
        "WASM-QUINE-BOOTLOADER_SYS v2.4 // PHASE: %s // STATUS: %s",
        eraStr(app.era()).c_str(),
        app.isPaused() ? "PAUSED" : "RUNNING");
}
