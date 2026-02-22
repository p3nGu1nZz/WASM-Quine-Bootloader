#pragma once

#include "app.h"
#include "gui/heatmap.h"

#include <SDL3/SDL.h>

// Forward-declare ImGui types to avoid pulling in imgui.h in the header
struct ImFont;

// ── Gui ───────────────────────────────────────────────────────────────────────
//
// Owns the Dear ImGui SDL3/Renderer backend lifecycle and orchestrates all
// per-frame rendering.  Created once after the SDL3 window + renderer are up.
//
//   gui.init(window, renderer);
//   while (running) { gui.renderFrame(app); }
//   gui.shutdown();
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
    SDL_Renderer* m_renderer  = nullptr;
    ImFont*       m_monoFont  = nullptr;

    // DPI scale computed during init; exposed for tests and potential layout
    // decisions.  Default is 1.0 for compatibility.
    float m_dpiScale = 1.0f;
    // Additional multiplier applied to UI elements (fonts, buttons) to ensure
    // touch-friendly, easy-to-read controls.  This is typically the DPI scale
    // boosted by a constant factor, with a minimum of 1.0.
    float m_uiScale  = 1.0f;
public:
    // Query the raw DPI-based scale.
    float dpiScale() const { return m_dpiScale; }
    // Query the effective UI scale used for fonts and widget dimensions.
    float uiScale() const { return m_uiScale; }

private:
    // Sub-module: memory heatmap (owns its own heat-decay state)
    GuiHeatmap m_heatmap;

    // Per-frame scroll / auto-scroll state
    bool   m_scrollLogs   = true;
    bool   m_scrollInstrs = true;
    int    m_lastIP       = -1;
    size_t m_lastLogSz    = 0;

    // ── Panel helpers (each renders one independent UI region) ────────────────
    void renderTopBar(App& app, int winW);
    void renderLogPanel(const App& app, float w, float h);
    void renderInstrPanel(const App& app, float w, float h);
    void renderKernelPanel(const App& app, float w, float h);
    void renderStatusBar(const App& app);
};
