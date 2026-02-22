#pragma once

#include "types.h"
#include "wasm/parser.h"
#include <string>
#include <vector>
#include <deque>

// ── Exporter ──────────────────────────────────────────────────────────────────
//
// Produces a human-readable telemetry report from the current simulation state.
// ─────────────────────────────────────────────────────────────────────────────

struct ExportData {
    int                        generation;
    SystemEra                  era;
    std::string                currentKernel;   // base64
    std::vector<Instruction>   instructions;
    std::deque<LogEntry>       logs;
    std::vector<HistoryEntry>  history;
};

// Build a full text report (hex dump, disassembly, history) from the given data.
std::string buildReport(const ExportData& data);
