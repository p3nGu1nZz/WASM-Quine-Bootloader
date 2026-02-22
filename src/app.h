#pragma once

#include "types.h"
#include "fsm.h"
#include "logger.h"
#include "wasm/wasm_kernel.h"
#include "wasm/wasm_parser.h"

#include <string>
#include <vector>
#include <cstdint>

// ── App ───────────────────────────────────────────────────────────────────────
//
// Top-level orchestrator.  Drives the BootFsm, coordinates the WasmKernel,
// mutation engine, and AppLogger.  Call update() once per frame.
// ─────────────────────────────────────────────────────────────────────────────

class App {
public:
    App();

    // Drive the state machine.  Returns false when the app should exit.
    bool update();

    // ── Accessors for the renderer ────────────────────────────────────────────
    SystemState  state()               const { return m_fsm.current(); }
    SystemEra    era()                 const { return m_era; }
    int          generation()          const { return m_generation; }
    double       uptimeSec()           const { return m_uptimeMs / 1000.0; }
    int          retryCount()          const { return m_retryCount; }
    int          evolutionAttempts()   const { return m_evolutionAttempts; }
    int          programCounter()      const { return m_programCounter; }
    bool         isPaused()            const { return m_paused; }
    int          focusAddr()           const { return m_focusAddr; }
    int          focusLen()            const { return m_focusLen; }
    bool         isMemoryGrowing()     const { return m_memGrowing; }
    bool         isSystemReading()     const { return m_sysReading; }
    size_t       kernelBytes()         const;

    const std::deque<LogEntry>&               logs()         const { return m_logger.logs(); }
    const std::vector<Instruction>&           instructions() const { return m_instructions; }
    const std::string&                        currentKernel() const { return m_currentKernel; }
    const std::string&                        stableKernel()  const { return m_stableKernel; }
    const std::vector<std::vector<uint8_t>>&  knownInstructions() const { return m_knownInstructions; }
    int                                       knownInstructionCount() const { return (int)m_knownInstructions.size(); }

    void togglePause() { m_paused = !m_paused; }

    // Build and return a telemetry report string.
    std::string exportHistory() const;

private:
    // FSM helpers
    void transitionTo(SystemState s);
    void updateEra();

    // Boot sequence steps (called from update())
    void startBoot();
    void tickBooting();
    void tickLoading();
    void tickExecuting();
    void tickVerifying();
    void tickRepairing();
    void doReboot(bool success);

    // WASM host callbacks
    void onWasmLog(uint32_t ptr, uint32_t len, const uint8_t* mem, uint32_t memSize);
    void onGrowMemory(uint32_t pages);

    void handleBootFailure(const std::string& reason);

    // ── Components ────────────────────────────────────────────────────────────
    BootFsm   m_fsm;
    AppLogger m_logger;
    WasmKernel m_kernel;

    // ── State ─────────────────────────────────────────────────────────────────
    SystemEra m_era    = SystemEra::PRIMORDIAL;
    bool      m_paused = false;

    int    m_generation        = 0;
    double m_uptimeMs          = 0.0;
    int    m_retryCount        = 0;
    int    m_evolutionAttempts = 0;
    int    m_programCounter    = -1;

    std::string m_stableKernel;
    std::string m_currentKernel;
    std::string m_nextKernel;    // deferred on successful quine

    std::vector<Instruction>              m_instructions;
    std::vector<std::vector<uint8_t>>     m_knownInstructions;
    std::vector<uint8_t>                  m_pendingMutation;

    int  m_focusAddr  = 0;
    int  m_focusLen   = 0;
    bool m_memGrowing = false;
    bool m_sysReading = false;

    // Per-tick timing
    uint64_t m_lastFrameTicks  = 0;
    int      m_loadingProgress = 0;
    int      m_instrIndex      = 0;
    bool     m_callExecuted    = false;
    bool     m_quineSuccess    = false;

    uint64_t m_memGrowFlashUntil = 0;

    uint64_t now() const;
};
