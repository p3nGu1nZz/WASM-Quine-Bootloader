#include "app.h"
#include "constants.h"
#include "base64.h"
#include "util.h"
#include "wasm_evolution.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// ─── App ─────────────────────────────────────────────────────────────────────

App::App() {
    m_stableKernel  = KERNEL_GLOB;
    m_currentKernel = KERNEL_GLOB;
    m_lastFrameTicks = now();

    // Parse initial kernel
    auto bytes = base64_decode(m_currentKernel);
    m_instructions = extractCodeSection(bytes);
}

uint64_t App::now() const {
    return SDL_GetTicks();
}

size_t App::kernelBytes() const {
    return base64_decode(m_currentKernel).size();
}

void App::setState(SystemState s) {
    m_state          = s;
    m_stateEnteredAt = now();
}

void App::updateEra() {
    if      (m_generation < 5)  m_era = SystemEra::PRIMORDIAL;
    else if (m_generation < 15) m_era = SystemEra::EXPANSION;
    else if (m_generation < 30) m_era = SystemEra::COMPLEXITY;
    else                        m_era = SystemEra::SINGULARITY;
}

void App::addLog(const std::string& msg, const std::string& type) {
    // Deduplicate within 100 ms
    if (!m_logs.empty()) {
        const auto& last = m_logs.back();
        if (last.message == msg && (now() - last.timestamp) < 100)
            return;
    }
    m_logs.push_back({ randomId(), now(), msg, type });
    if (m_logs.size() > 1000)
        m_logs.pop_front();
}

// ─── Main update (called every frame) ────────────────────────────────────────

bool App::update() {
    uint64_t t   = now();
    uint64_t dt  = t - m_lastFrameTicks;
    m_lastFrameTicks = t;

    if (!m_paused)
        m_uptimeMs += (double)dt;

    // Memory grow flash timeout
    if (m_memGrowing && t >= m_memGrowFlashUntil)
        m_memGrowing = false;

    if (m_paused) return true;

    uint64_t elapsed = t - m_stateEnteredAt;

    switch (m_state) {
        case SystemState::IDLE:
            startBoot();
            break;

        case SystemState::BOOTING: {
            uint64_t bootSpeed = (uint64_t)std::max(50, 400 - m_generation * 5);
            if (elapsed >= bootSpeed) {
                setState(SystemState::LOADING_KERNEL);
                m_loadingProgress = 0;
                int kbytes = (int)kernelBytes();
                addLog("Loading Kernel Image: " + std::to_string(kbytes) + " bytes", "info");
            }
            break;
        }

        case SystemState::LOADING_KERNEL:
            tickLoading();
            break;

        case SystemState::EXECUTING:
            tickExecuting();
            break;

        case SystemState::VERIFYING_QUINE:
            if (elapsed >= (uint64_t)DEFAULT_BOOT_CONFIG.rebootDelayMs)
                doReboot(true);
            break;

        case SystemState::REPAIRING:
            if (elapsed >= 1500)
                doReboot(false);
            break;

        case SystemState::SYSTEM_HALT:
            break;
    }

    return true;
}

// ─── Boot sequence steps ─────────────────────────────────────────────────────

void App::startBoot() {
    setState(SystemState::BOOTING);
    addLog("--- BOOT SEQUENCE INITIATED ---", "system");
    m_instrIndex    = 0;
    m_callExecuted  = false;
    m_quineSuccess  = false;
    m_programCounter = -1;
    m_focusAddr = 0;
    m_focusLen  = 0;
    m_sysReading = false;
    m_kernel.terminate();
}

void App::tickLoading() {
    static const int LOAD_STEP = 8;
    int kbytes = (int)kernelBytes();

    if (m_loadingProgress < kbytes) {
        m_focusAddr = m_loadingProgress;
        m_focusLen  = LOAD_STEP;
        m_loadingProgress += LOAD_STEP;
        return; // one step per frame at the loading stage
    }

    // Loading finished – instantiate WASM module
    m_focusAddr = 0;
    m_focusLen  = 0;

    addLog("Instantiating Module...", "info");
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

    setState(SystemState::EXECUTING);
    m_instrIndex   = 0;
    m_callExecuted = false;
}

void App::tickExecuting() {
    uint64_t stepSpeed = (uint64_t)std::max(80, 200 - m_generation * 2);

    // Add a per-step delay based on time since state entered for each instruction
    uint64_t elapsed = now() - m_stateEnteredAt;
    int expectedIdx  = (int)(elapsed / stepSpeed);
    if (m_instrIndex > expectedIdx) return; // wait

    if (m_instructions.empty()) {
        // Fallback blind execution
        if (!m_callExecuted) {
            addLog("EXEC: Blind Run (Parser unavailable)", "warning");
            try {
                m_kernel.runDynamic(m_currentKernel);
            } catch (const std::exception& e) {
                handleBootFailure(e.what());
            }
            m_callExecuted = true;
        }
        return;
    }

    if (m_instrIndex >= (int)m_instructions.size()) {
        // Finished stepping; if call was never executed force it
        if (!m_callExecuted) {
            addLog("Warning: No CALL detected, forcing execution...", "warning");
            try {
                m_kernel.runDynamic(m_currentKernel);
            } catch (const std::exception& e) {
                handleBootFailure(e.what());
            }
        }
        return; // wait for quine success/failure callback or timeout
    }

    const auto& inst = m_instructions[m_instrIndex];
    m_programCounter = m_instrIndex;
    m_focusAddr      = inst.originalOffset;
    m_focusLen       = std::max(1, inst.length);

    if (inst.opcode == 0x10 && !m_callExecuted) { // CALL
        m_sysReading   = true;
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

// ─── WASM callbacks ───────────────────────────────────────────────────────────

void App::onWasmLog(uint32_t ptr, uint32_t len,
                     const uint8_t* mem, uint32_t memSize)
{
    if (ptr + len > memSize) {
        handleBootFailure("WASM log out of bounds");
        return;
    }

    std::string output(reinterpret_cast<const char*>(mem + ptr), len);

    addLog("STDOUT: Received " + std::to_string(len) +
           " bytes from 0x" + [&]{
               std::ostringstream ss;
               ss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << ptr;
               return ss.str();
           }(), "info");

    if (output == m_currentKernel) {
        addLog("VERIFICATION: MEMORY INTEGRITY CONFIRMED", "success");
        addLog("EXEC: QUINE SUCCESS -> INITIATING REBOOT...", "system");

        m_stableKernel = m_currentKernel;
        m_retryCount   = 0;
        m_history.push_back({ m_generation, nowIso(), (int)kernelBytes(),
                               "EXECUTE", "Verification Success", true });

        // Evolve
        try {
            auto evo = evolveBinary(m_currentKernel, m_knownInstructions,
                                    m_generation + 1);
            // Validate the evolved binary (try parsing)
            auto evolved = base64_decode(evo.binary);
            if (evolved.size() < 8 || evolved[0] != 0x00 || evolved[1] != 0x61 ||
                evolved[2] != 0x73 || evolved[3] != 0x6D) {
                throw std::runtime_error("Invalid WASM magic after evolution");
            }
            m_nextKernel       = evo.binary;
            m_pendingMutation  = evo.mutationSequence;
            m_evolutionAttempts++;
            addLog("EVOLUTION: " + evo.description, "mutation");
            m_history.push_back({ m_generation, nowIso(), (int)kernelBytes(),
                                   "EVOLVE", evo.description, true });
        } catch (const std::exception& e) {
            addLog(std::string("EVOLUTION REJECTED: ") + e.what(), "warning");
            m_nextKernel.clear();
            m_pendingMutation.clear();
        }

        setState(SystemState::VERIFYING_QUINE);
    } else {
        handleBootFailure("Output checksum mismatch (Self-Replication Failed)");
    }
}

void App::onGrowMemory(uint32_t pages) {
    m_memGrowing       = true;
    m_memGrowFlashUntil = now() + 800;
}

// ─── Failure / Repair ────────────────────────────────────────────────────────

void App::handleBootFailure(const std::string& reason) {
    addLog("CRITICAL: " + reason, "error");
    m_history.push_back({ m_generation, nowIso(), (int)kernelBytes(),
                          "REPAIR", reason, false });

    int nextRetry = m_retryCount + 1;
    m_retryCount  = nextRetry;

    try {
        auto evo = evolveBinary(m_stableKernel, m_knownInstructions, nextRetry);
        m_currentKernel  = evo.binary;
        m_nextKernel.clear();
        m_pendingMutation = evo.mutationSequence;
        addLog("ADAPTATION: " + evo.description, "mutation");
        // Re-parse instructions for the new kernel
        auto bytes = base64_decode(m_currentKernel);
        m_instructions = extractCodeSection(bytes);
    } catch (...) {
        m_currentKernel = m_stableKernel;
        m_pendingMutation.clear();
        addLog("ADAPTATION: Fallback to base stable kernel", "system");
        auto bytes = base64_decode(m_currentKernel);
        m_instructions = extractCodeSection(bytes);
    }

    setState(SystemState::REPAIRING);
    m_programCounter = -1;
    m_focusAddr = 0;
    m_focusLen  = 0;
    m_sysReading = false;
}

void App::doReboot(bool success) {
    m_kernel.terminate();
    m_programCounter = -1;
    m_focusAddr = 0;
    m_focusLen  = 0;
    m_sysReading = false;

    if (success) {
        m_generation++;
        updateEra();

        // Apply deferred kernel update
        if (!m_nextKernel.empty()) {
            m_currentKernel = m_nextKernel;
            m_nextKernel.clear();
            // Re-parse instructions
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

    setState(SystemState::IDLE);
}

// ─── Export ──────────────────────────────────────────────────────────────────

std::string App::exportHistory() const {
    // Disassembly
    std::string disasm = "No instructions available.";
    if (!m_instructions.empty()) {
        std::ostringstream ss;
        for (int i = 0; i < (int)m_instructions.size(); i++) {
            const auto& inst = m_instructions[i];
            std::string name = getOpcodeName(inst.opcode);
            std::string args;
            for (uint8_t a : inst.args) {
                if (!args.empty()) args += ' ';
                std::ostringstream hex;
                hex << "0x" << std::uppercase << std::hex << (int)a;
                args += hex.str();
            }
            ss << std::setw(3) << std::setfill('0') << i << " | 0x"
               << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
               << inst.originalOffset << " | "
               << std::setw(12) << std::setfill(' ') << std::left << name
               << " " << args << '\n';
        }
        disasm = ss.str();
    }

    // Hex dump
    std::string hexDump;
    auto raw = base64_decode(m_currentKernel);
    std::ostringstream hd;
    for (size_t i = 0; i < raw.size(); i += 16) {
        hd << "0x" << std::uppercase << std::hex << std::setw(4)
           << std::setfill('0') << i << "  ";
        std::string ascii;
        for (size_t j = 0; j < 16; j++) {
            if (i + j < raw.size()) {
                hd << std::uppercase << std::hex << std::setw(2)
                   << std::setfill('0') << (int)raw[i + j] << ' ';
                char c = (char)raw[i + j];
                ascii += (c >= 32 && c <= 126) ? c : '.';
            } else {
                hd << "   ";
                ascii += ' ';
            }
        }
        hd << " |" << ascii << "|\n";
    }
    hexDump = hd.str();

    // History log
    std::ostringstream hist;
    for (const auto& h : m_history) {
        std::string ts = h.timestamp.size() > 11 ? h.timestamp.substr(11, 12) : h.timestamp;
        std::ostringstream row;
        row << "[GEN " << std::setw(4) << std::setfill('0') << h.generation << "] "
            << ts << " | " << std::setw(10) << std::setfill(' ') << std::left << h.action
            << " | " << (h.success ? "OK  " : "FAIL") << " | " << h.details << '\n';
        hist << row.str();
    }

    std::ostringstream out;
    out << "WASM QUINE BOOTLOADER - SYSTEM HISTORY EXPORT\n"
        << "Generated: " << nowIso() << '\n'
        << "Final Generation: " << m_generation << '\n'
        << "Kernel Size: " << raw.size() << " bytes\n"
        << "System Era: ";
    switch (m_era) {
        case SystemEra::PRIMORDIAL:  out << "PRIMORDIAL";  break;
        case SystemEra::EXPANSION:   out << "EXPANSION";   break;
        case SystemEra::COMPLEXITY:  out << "COMPLEXITY";  break;
        case SystemEra::SINGULARITY: out << "SINGULARITY"; break;
    }
    out << "\n\n"
        << "CURRENT KERNEL (BASE64):\n"
        << std::string(80, '-') << '\n'
        << m_currentKernel << '\n'
        << std::string(80, '-') << "\n\n"
        << "HEX DUMP:\n"
        << std::string(80, '-') << '\n'
        << hexDump
        << std::string(80, '-') << "\n\n"
        << "DISASSEMBLY:\n"
        << std::string(80, '-') << '\n'
        << "IDX | ADDR   | OPCODE       ARGS\n"
        << std::string(80, '-') << '\n'
        << disasm
        << std::string(80, '-') << "\n\n"
        << "HISTORY LOG:\n"
        << std::string(80, '-') << '\n'
        << hist.str()
        << std::string(80, '-') << '\n'
        << "END OF REPORT\n";

    return out.str();
}
