#pragma once

#include "types.h"
#include "wasm_kernel.h"
#include "wasm_parser.h"

#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <cstdint>

class App {
public:
    App();

    // Call once per frame; drives the state machine and returns false when done
    bool update();

    // Accessors for the renderer
    SystemState              state()             const { return m_state; }
    SystemEra                era()               const { return m_era; }
    int                      generation()        const { return m_generation; }
    double                   uptimeSec()         const { return m_uptimeMs / 1000.0; }
    int                      retryCount()        const { return m_retryCount; }
    int                      evolutionAttempts() const { return m_evolutionAttempts; }
    int                      programCounter()    const { return m_programCounter; }
    bool                     isPaused()          const { return m_paused; }
    const std::deque<LogEntry>&       logs()      const { return m_logs; }
    const std::vector<Instruction>&   instructions() const { return m_instructions; }
    const std::string&       currentKernel()     const { return m_currentKernel; }
    const std::string&       stableKernel()      const { return m_stableKernel; }
    const std::vector<std::vector<uint8_t>>& knownInstructions() const { return m_knownInstructions; }
    int                      knownInstructionCount() const { return (int)m_knownInstructions.size(); }
    size_t                   kernelBytes()       const;
    int                      focusAddr()         const { return m_focusAddr; }
    int                      focusLen()          const { return m_focusLen; }
    bool                     isMemoryGrowing()   const { return m_memGrowing; }
    bool                     isSystemReading()   const { return m_sysReading; }

    void togglePause() { m_paused = !m_paused; }

    // Export history as a text string (caller may write to file or clipboard)
    std::string exportHistory() const;

private:
    // State machine
    void setState(SystemState s);
    void updateEra();
    void addLog(const std::string& msg, const std::string& type = "info");

    // Boot sequence steps
    void startBoot();
    void tickLoading();
    void tickExecuting();
    void tickVerifying();
    void tickRepairing();
    void doReboot(bool success);

    // WASM callbacks (called from WasmKernel during execution)
    void onWasmLog(uint32_t ptr, uint32_t len, const uint8_t* mem, uint32_t memSize);
    void onGrowMemory(uint32_t pages);

    void handleBootFailure(const std::string& reason);

    // Data
    SystemState m_state      = SystemState::IDLE;
    SystemEra   m_era        = SystemEra::PRIMORDIAL;
    bool        m_paused     = false;

    int    m_generation         = 0;
    double m_uptimeMs           = 0.0;
    int    m_retryCount         = 0;
    int    m_evolutionAttempts  = 0;
    int    m_programCounter     = -1;

    std::string m_stableKernel;
    std::string m_currentKernel;
    std::string m_nextKernel;    // deferred update

    std::vector<Instruction>        m_instructions;
    std::vector<std::vector<uint8_t>> m_knownInstructions;
    std::vector<uint8_t>            m_pendingMutation;

    std::deque<LogEntry>   m_logs;
    std::vector<HistoryEntry> m_history;

    int  m_focusAddr   = 0;
    int  m_focusLen    = 0;
    bool m_memGrowing  = false;
    bool m_sysReading  = false;

    WasmKernel m_kernel;

    // Timing helpers
    uint64_t m_stateEnteredAt   = 0; // SDL_GetTicks() at last state change
    uint64_t m_lastFrameTicks   = 0;
    int      m_loadingProgress  = 0;
    int      m_instrIndex       = 0;
    bool     m_callExecuted     = false;
    bool     m_quineSuccess     = false; // set inside onWasmLog

    // Memory grow flash timer
    uint64_t m_memGrowFlashUntil = 0;

    uint64_t now() const;
};
