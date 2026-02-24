#pragma once

#include "types.h"
#include "fsm.h"
#include "log.h"
#include "wasm/kernel.h"
#include "wasm/parser.h"
#include "cli.h"
#include "advisor.h"
#include "train.h"
#include <climits>
#include <functional>
#include <map>
#include <unordered_map>

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
    // constructor with optional custom time source (used by unit tests)
    explicit App(const CliOptions& opts,
                 std::function<uint64_t()> nowFn = nullptr);

    // Drive the state machine.  Returns false when the app should exit.
    bool update();

    // ── Accessors for the renderer ────────────────────────────────────────────
    SystemState  state()               const { return m_fsm.current(); }
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

    // Execute a callback with a per-run timeout.  On platforms that support
    // it this will fork a child process and kill it if it exceeds the
    // configured `CliOptions::maxExecMs`.  Returns true if the callback
    // completed successfully before the deadline; false otherwise.
    bool runWithTimeout(const std::function<void()>& fn);

    // Request that the application shut itself down at the next convenient
    // opportunity.  This sets an internal flag so that `update()` will return
    // false soon after the current generation completes.  It is safe to call
    // from a signal handler (invoked via `requestAppExit()` below).
    void requestExit();

    // manually trigger telemetry export for the current generation
    void exportNow();

    // Unique identifier for this run; used to organise exported data.
    const std::string& runId() const { return m_runId; }

    // access CLI options
    const CliOptions& options() const { return m_opts; }

    // expose trainer for tests
    const Trainer& trainer() const { return m_trainer; }

    // expose advisor for GUI or tests
    const Advisor& advisor() const { return m_advisor; }

    // helper used by tests to apply a telemetry entry and optionally
    // persist the model via CLI flag.
    void trainAndMaybeSave(const TelemetryEntry& te);

    // expose reboot helper for tests
    void doReboot(bool success);

    // test helpers
    // simulate a boot failure triggered by the given mutation sequence
    // (calls private handleBootFailure internally)
    void test_simulateFailure(const std::string& reason,
                               const std::vector<uint8_t>& mutation) {
        m_pendingMutation = mutation;
        handleBootFailure(reason);
    }

    // Blacklist management
    bool isBlacklisted(const std::vector<uint8_t>& seq) const;
    void addToBlacklist(const std::vector<uint8_t>& seq);

    // decay all weights by one; entries reaching zero are removed
    void decayBlacklist();

    // telemetry accessors (for tests or GUI)
    int mutationsApplied() const { return m_mutationsApplied; }
    int mutationInsertCount() const { return m_mutationInsert; }
    int mutationDeleteCount() const { return m_mutationDelete; }
    int mutationModifyCount() const { return m_mutationModify; }
    int mutationAddCount() const { return m_mutationAdd; }
    double lastGenDurationMs() const { return m_lastGenDurationMs; }
    int kernelSizeMin() const { return m_kernelSizeMin; }
    int kernelSizeMax() const { return m_kernelSizeMax; }
    const std::string& lastTrapReason() const { return m_lastTrapReason; }

    // Persist blacklist across runs
    ~App();                             // flush blacklist on destruction
    void loadBlacklist();
    void saveBlacklist() const;

private:
    // FSM helpers
    void transitionTo(SystemState s);

    // Boot sequence steps (called from update())
    void startBoot();
    void tickBooting();
    void tickLoading();
    void tickExecuting();
    void tickVerifying();
    void tickRepairing();

    // Export helpers
    void autoExport();

    // WASM host callbacks
    void onWasmLog(uint32_t ptr, uint32_t len, const uint8_t* mem, uint32_t memSize);
    void onGrowMemory(uint32_t pages);

    void handleBootFailure(const std::string& reason);

    // ── Components ────────────────────────────────────────────────────────────
    BootFsm   m_fsm;
    AppLogger m_logger;
    WasmKernel m_kernel;

    // for learning & advice
    Advisor m_advisor;
    Trainer m_trainer;

    // ── State ─────────────────────────────────────────────────────────────────
    // era tracking removed; visual themes not required
    bool      m_paused = false;

    int    m_generation        = 0;
    double m_uptimeMs          = 0.0;
    int    m_retryCount        = 0;
    int    m_evolutionAttempts = 0;
    // telemetry counters
    int    m_mutationsApplied = 0;
    int    m_mutationInsert   = 0;
    int    m_mutationDelete   = 0;
    int    m_mutationModify   = 0;
    int    m_mutationAdd      = 0;
    // heuristic blacklist: sequences that previously caused traps, mapped to decay weight
    // weight >0 means entry is still blacklisted; DECAY mode will decrement after each success
    struct VecHash {
        size_t operator()(const std::vector<uint8_t>& v) const noexcept {
            // FNV-1a 64-bit
            size_t h = 1469598103934665603ULL;
            for (uint8_t b : v) {
                h ^= b;
                h *= 1099511628211ULL;
            }
            return h;
        }
    };
    std::unordered_map<std::vector<uint8_t>, int, VecHash> m_blacklist;
    // profiling / telemetry timing
    uint64_t m_genStartTime   = 0; // steady ticks at generation start
    double   m_lastGenDurationMs = 0.0;
    int      m_kernelSizeMin  = INT_MAX;
    int      m_kernelSizeMax  = 0;
    std::string m_lastTrapReason;
    int    m_programCounter    = -1;

    std::string m_stableKernel;
    std::string m_currentKernel;
    std::string m_nextKernel;    // deferred on successful quine

    // cache the decoded bytes for the current kernel; updated whenever
    // `m_currentKernel` changes.  This avoids repeated base64 decoding in
    // kernelBytes() and other accessors.
    std::vector<uint8_t> m_currentKernelBytes;

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

    // Identifier for this session; created at startup
    std::string m_runId;

    uint64_t m_memGrowFlashUntil = 0;

    // CLI options supplied at startup
    CliOptions m_opts;

    // internal flag used by requestExit() and signal handlers
    bool m_shouldExit = false;

    // optional time source (default uses SDL_GetTicks).  tests inject a fake
    // clock so that run-time limits can be verified deterministically.
    std::function<uint64_t()> m_nowFn;

    // requester to exit after max generation reached
    bool m_shouldExit = false;

    uint64_t now() const;

    // utility used internally whenever the base64 kernel string is updated.
    // decodes into `m_currentKernelBytes` and refreshes `m_instructions`.
    void updateKernelData();
};
