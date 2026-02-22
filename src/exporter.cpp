#include "exporter.h"
#include "base64.h"
#include "util.h"
#include "wasm/parser.h"

#include <sstream>
#include <iomanip>

std::string buildReport(const ExportData& d) {
    // ── Disassembly ────────────────────────────────────────────────────────────
    std::string disasm = "No instructions available.";
    if (!d.instructions.empty()) {
        std::ostringstream ss;
        for (int i = 0; i < (int)d.instructions.size(); i++) {
            const auto& inst = d.instructions[i];
            std::string name = getOpcodeName(inst.opcode);
            std::string args;
            for (uint8_t a : inst.args) {
                if (!args.empty()) args += ' ';
                std::ostringstream hex;
                hex << "0x" << std::uppercase << std::hex << (int)a;
                args += hex.str();
            }
            ss << std::setw(3) << std::setfill('0') << i
               << " | 0x" << std::uppercase << std::hex
               << std::setw(4) << std::setfill('0') << inst.originalOffset
               << " | " << std::setw(12) << std::setfill(' ') << std::left
               << name << " " << args << '\n';
        }
        disasm = ss.str();
    }

    // ── Hex dump ──────────────────────────────────────────────────────────────
    auto raw = base64_decode(d.currentKernel);
    std::ostringstream hd;
    for (size_t i = 0; i < raw.size(); i += 16) {
        hd << "0x" << std::uppercase << std::hex
           << std::setw(4) << std::setfill('0') << i << "  ";
        std::string ascii;
        for (size_t j = 0; j < 16; j++) {
            if (i + j < raw.size()) {
                hd << std::uppercase << std::hex
                   << std::setw(2) << std::setfill('0') << (int)raw[i + j] << ' ';
                char c = (char)raw[i + j];
                ascii += (c >= 32 && c <= 126) ? c : '.';
            } else {
                hd << "   ";
                ascii += ' ';
            }
        }
        hd << " |" << ascii << "|\n";
    }

    // ── History log ───────────────────────────────────────────────────────────
    std::ostringstream hist;
    for (const auto& h : d.history) {
        std::string ts = h.timestamp.size() > 11 ? h.timestamp.substr(11, 12) : h.timestamp;
        hist << "[GEN " << std::setw(4) << std::setfill('0') << h.generation << "] "
             << ts << " | " << std::setw(10) << std::setfill(' ') << std::left << h.action
             << " | " << (h.success ? "OK  " : "FAIL") << " | " << h.details << '\n';
    }

    // ── Era name ──────────────────────────────────────────────────────────────
    std::string eraName = eraStr(d.era);

    // ── Assemble report ───────────────────────────────────────────────────────
    std::ostringstream out;
    out << "WASM QUINE BOOTLOADER - SYSTEM HISTORY EXPORT\n"
        << "Generated: " << nowIso() << '\n'
        << "Final Generation: " << d.generation << '\n'
        << "Kernel Size: " << raw.size() << " bytes\n"
        << "System Era: " << eraName << "\n\n"
        << "CURRENT KERNEL (BASE64):\n"
        << std::string(80, '-') << '\n'
        << d.currentKernel << '\n'
        << std::string(80, '-') << "\n\n"
        << "HEX DUMP:\n"
        << std::string(80, '-') << '\n'
        << hd.str()
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
