#include "app.h"
#include "constants.h"
#include "base64.h"
#include "util.h"
#include "exporter.h"
#include "wasm/wasm_evolution.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// ─── App ─────────────────────────────────────────────────────────────────────

App::App() {
    m_stableKernel   = KERNEL_GLOB;
    m_currentKernel  = KERNEL_GLOB;
    m_lastFrameTicks = now();

    // Open buffered log file (flushes every ~1 s; always flushed on exit/signal)
    m_logger.init("bootloader_" + nowFileStamp() + ".log");

    // Parse initial kernel
    auto bytes = base64_decode(m_currentKernel);
    m_instructions = extractCodeSection(bytes);
}

uint64_t App::now() const {
    return static_cast<uint64_t>(SDL_GetTicks());
}

size_t App::kernelBytes() const {
    return base64_decode(m_currentKernel).size();
}

// ─── FSM helpers ─────────────────────────────────────────────────────────────

void App::transitionTo(SystemState s) {
    m_fsm.transition(s);
}

void App::updateEra() {
    if      (m_generation < 5)  m_era = SystemEra::PRIMORDIAL;
    else if (m_generation < 15) m_era = SystemEra::EXPANSION;
    else if (m_generation < 30) m_era = SystemEra::COMPLEXITY;
    else                        m_era = SystemEra::SINGULARITY;
}

// ─── Main update (called every frame) ────────────────────────────────────────

bool App::update() {
    uint64_t t  = now();
    uint64_t dt = t - m_lastFrameTicks;
    m_lastFrameTicks = t;

    if (!m_paused)
        m_uptimeMs += static_cast<double>(dt);

    if (m_memGrowing && t >= m_memGrowFlashUntil)
        m_memGrowing = false;

    if (m_paused) return true;

    switch (m_fsm.current()) {
        case SystemState::IDLE:             startBoot();      break;
        case SystemState::BOOTING:          tickBooting();    break;
        case SystemState::LOADING_KERNEL:   tickLoading();    break;
        case SystemState::EXECUTING:        tickExecuting();  break;
        case SystemState::VERIFYING_QUINE:  tickVerifying();  break;
        case SystemState::REPAIRING:        tickRepairing();  break;
        case SystemState::SYSTEM_HALT:                        break;
    }

    return true;
}

// ─── Boot sequence steps ─────────────────────────────────────────────────────

void App::startBoot() {
    transitionTo(SystemState::BOOTING);
    m_logger.log("--- BOOT SEQUENCE INITIATED ---", "system");
    m_instrIndex     = 0;
    m_callExecuted   = false;
    m_quineSuccess   = false;
    m_programCounter = -1;
    m_focusAddr      = 0;
    m_focusLen       = 0;
    m_sysReading     = false;
    m_kernel.terminate();
}

void App::tickBooting() {
    uint64_t bootSpeed = static_cast<uint64_t>(std::max(50, 400 - m_generation * 5));
    if (m_fsm.elapsedMs() >= bootSpeed) {
        transitionTo(SystemState::LOADING_KERNEL);
        m_loadingProgress = 0;
        int kbytes = static_cast<int>(kernelBytes());
        m_logger.log("Loading Kernel Image: " + std::to_string(kbytes) + " bytes", "info");
    }
}

void App::tickLoading() {
    static const int LOAD_STEP = 8;
    int kbytes = static_cast<int>(kernelBytes());

    if (m_loadingProgress < kbytes) {
        m_focusAddr        = m_loadingProgress;
        m_focusLen         = LOAD_STEP;
        m_loadingProgress += LOAD_STEP;
        return;
    }

    m_focusAddr = 0;
    m_focusLen  = 0;

    m_logger.log("Instantiating Module...", "info");
    try {
        m_kernel.bootDynamic(
            m_currentKernel,
            [this](uint32_t ptr, uint32_t len, const uint8_t* mem, uint32_t msz) {
                onWasmLog(ptr, len, mem, msz);
            },
            [this](uint32_t pages) {
                onGrowMemory(pages);
            }
        );
    } catch (const std::exception& e) {
        handleBootFailure(std::string("Module load failed: ") + e.what());
        return;
    }

    if (!m_kernel.isLoaded()) {
        handleBootFailure("Instance lost during boot");
        return;
    }

    transitionTo(SystemState::EXECUTING);
    m_instrIndex   = 0;
    m_callExecuted = false;
}

void App::tickExecuting() {
    uint64_t stepSpeed   = static_cast<uint64_t>(std::max(80, 200 - m_generation * 2));
    uint64_t elapsed     = m_fsm.elapsedMs();
    int      expectedIdx = static_cast<int>(elapsed / stepSpeed);
    if (m_instrIndex > expectedIdx) return;

    if (m_instructions.empty()) {
        if (!m_callExecuted) {
            m_logger.log("EXEC: Blind Run (Parser unavailable)", "warning");
            try {
                m_kernel.runDynamic(m_currentKernel);
            } catch (const std::exception& e) {
                handleBootFailure(e.what());
            }
            m_callExecuted = true;
        }
        return;
    }

    if (m_instrIndex >= static_cast<int>(m_instructions.size())) {
        if (!m_callExecuted) {
            m_logger.log("Warning: No CALL detected, forcing execution...", "warning");
            try {
                m_kernel.runDynamic(m_currentKernel);
            } catch (const std::exception& e) {
                handleBootFailure(e.what());
            }
        }
        return;
    }

    const auto& inst = m_instructions[m_instrIndex];
    m_programCounter  = m_instrIndex;
    m_focusAddr       = inst.originalOffset;
    m_focusLen        = std::max(1, inst.length);

    if (inst.opcode == 0x10 && !m_callExecuted) { // CALL
        m_sysReading = true;
        try {
            m_kernel.runDynamic(m_currentKernel);
        } catch (const std::exception& e) {
            m_sysReading = false;
            handleBootFailure(e.what());
            return;
        }
        m_callExecuted = true;
        m_sysReading   = false;
    }

    m_instrIndex++;
}

void App::tickVerifying() {
    if (m_fsm.elapsedMs() >= static_cast<uint64_t>(DEFAULT_BOOT_CONFIG.rebootDelayMs))
        doReboot(true);
}

void App::tickRepairing() {
    if (m_fsm.elapsedMs() >= 1500)
        doReboot(false);
}

// ─── WASM callbacks ───────────────────────────────────────────────────────────

void App::onWasmLog(uint32_t ptr, uint32_t len,
                    const uint8_t* mem, uint32_t memSize)
{
    if (ptr + len > memSize) {
        handleBootFailure("WASM log out of bounds");
        return;
    }

    std::string output(reinterpret_cast<const char*>(mem + ptr), len);

    m_logger.log("STDOUT: Received " + std::to_string(len) +
                 " bytes from 0x" + [&]{
                     std::ostringstream ss;
                     ss << std::uppercase << std::hex << std::setw(4)
                        << std::setfill('0') << ptr;
                     return ss.str();
                 }(), "info");

    if (output == m_currentKernel) {
        m_logger.log("VERIFICATION: MEMORY INTEGRITY CONFIRMED", "success");
        m_logger.log("EXEC: QUINE SUCCESS -> INITIATING REBOOT...", "system");

        m_stableKernel = m_currentKernel;
        m_retryCount   = 0;
        m_logger.addHistory({ m_generation, nowIso(), (int)kernelBytes(),
                               "EXECUTE", "Verification Success", true });

        // Evolve
        try {
            auto evo     = evolveBinary(m_currentKernel, m_knownInstructions,
                                        m_generation + 1);
            auto evolved = base64_decode(evo.binary);
            if (evolved.size() < 8 || evolved[0] != 0x00 || evolved[1] != 0x61 ||
                evolved[2] != 0x73 || evolved[3] != 0x6D)
                throw std::runtime_error("Invalid WASM magic after evolution");

            m_nextKernel      = evo.binary;
            m_pendingMutation = evo.mutationSequence;
            m_evolutionAttempts++;
            m_logger.log("EVOLUTION: " + evo.description, "mutation");
            m_logger.addHistory({ m_generation, nowIso(), (int)kernelBytes(),
                                   "EVOLVE", evo.description, true });
        } catch (const std::exception& e) {
            m_logger.log(std::string("EVOLUTION REJECTED: ") + e.what(), "warning");
            m_nextKernel.clear();
            m_pendingMutation.clear();
        }

        transitionTo(SystemState::VERIFYING_QUINE);
    } else {
        handleBootFailure("Output checksum mismatch (Self-Replication Failed)");
    }
}

void App::onGrowMemory(uint32_t /*pages*/) {
    // We only need the flash effect; the page count itself is not tracked.
    m_memGrowing        = true;
    m_memGrowFlashUntil = now() + 800;
}

// ─── Failure / Repair ────────────────────────────────────────────────────────

void App::handleBootFailure(const std::string& reason) {
    m_logger.log("CRITICAL: " + reason, "error");
    m_logger.addHistory({ m_generation, nowIso(), (int)kernelBytes(),
                          "REPAIR", reason, false });

    m_retryCount++;

    try {
        auto evo      = evolveBinary(m_stableKernel, m_knownInstructions, m_retryCount);
        m_currentKernel  = evo.binary;
        m_nextKernel.clear();
        m_pendingMutation = evo.mutationSequence;
        m_logger.log("ADAPTATION: " + evo.description, "mutation");
        auto bytes = base64_decode(m_currentKernel);
        m_instructions = extractCodeSection(bytes);
    } catch (...) {
        m_currentKernel = m_stableKernel;
        m_pendingMutation.clear();
        m_logger.log("ADAPTATION: Fallback to base stable kernel", "system");
        auto bytes = base64_decode(m_currentKernel);
        m_instructions = extractCodeSection(bytes);
    }

    transitionTo(SystemState::REPAIRING);
    m_programCounter = -1;
    m_focusAddr      = 0;
    m_focusLen       = 0;
    m_sysReading     = false;
}

void App::doReboot(bool success) {
    m_kernel.terminate();
    m_programCounter = -1;
    m_focusAddr      = 0;
    m_focusLen       = 0;
    m_sysReading     = false;

    if (success) {
        m_generation++;
        updateEra();

        if (!m_nextKernel.empty()) {
            m_currentKernel = m_nextKernel;
            m_nextKernel.clear();
            auto bytes = base64_decode(m_currentKernel);
            m_instructions = extractCodeSection(bytes);
        }

        if (!m_pendingMutation.empty()) {
            bool isNop = (m_pendingMutation.size() == 1 && m_pendingMutation[0] == 0x01);
            if (!isNop) {
                bool exists = false;
                for (const auto& seq : m_knownInstructions)
                    if (seq == m_pendingMutation) { exists = true; break; }
                if (!exists)
                    m_knownInstructions.push_back(m_pendingMutation);
            }
            m_pendingMutation.clear();
        }
    } else {
        m_pendingMutation.clear();
    }

    transitionTo(SystemState::IDLE);
}

// ─── Export ──────────────────────────────────────────────────────────────────

std::string App::exportHistory() const {
    ExportData d;
    d.generation    = m_generation;
    d.era           = m_era;
    d.currentKernel = m_currentKernel;
    d.instructions  = m_instructions;
    d.logs          = m_logger.logs();
    d.history       = m_logger.history();
    return buildReport(d);
}
