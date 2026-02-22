#pragma once

#include "app.h"

#include <SDL3/SDL.h>

// Forward-declare ImGui types to avoid pulling in imgui.h in headers
struct ImFont;
struct ImVec4;

// ── Gui ───────────────────────────────────────────────────────────────────────
//
// Owns the Dear ImGui SDL3/Renderer backend lifecycle and renders every
// frame of the WASM Quine Bootloader UI.  Call init() once after the SDL3
// window and renderer have been created, then renderFrame() each loop
// iteration, and shutdown() before destroying the renderer.
// ─────────────────────────────────────────────────────────────────────────────

class Gui {
public:
    // Initialize ImGui context and SDL3/Renderer backends.
    void init(SDL_Window* window, SDL_Renderer* renderer);

    // Process one full frame: NewFrame → render all panels → Present.
    void renderFrame(App& app);

    // Shut down ImGui backends and destroy the ImGui context.
    void shutdown();

private:
    SDL_Renderer* m_renderer    = nullptr;
    ImFont*       m_monoFont    = nullptr;

    // Per-frame scroll / sync state
    bool   m_scrollLogs   = true;
    bool   m_scrollInstrs = true;
    int    m_lastIP       = -1;
    size_t m_lastLogSz    = 0;

    // ── Panel helpers ─────────────────────────────────────────────────────────
    void renderTopBar(App& app, int winW);
    void renderLogPanel(const App& app, float w, float h);
    void renderInstrPanel(const App& app, float w, float h);
    void renderKernelPanel(const App& app, float w, float h);
    void renderMemoryVisualizer(const App& app, int winW);
    void renderStatusBar(const App& app);
};
