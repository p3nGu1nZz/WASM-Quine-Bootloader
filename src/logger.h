#pragma once

#include "types.h"
#include <string>
#include <deque>
#include <vector>

// ── AppLogger ─────────────────────────────────────────────────────────────────
//
// Manages the live log ring-buffer and the immutable history ledger.
// ─────────────────────────────────────────────────────────────────────────────

class AppLogger {
public:
    static constexpr size_t MAX_LOG_ENTRIES = 1000;

    // Append a new log entry.  Entries are deduplicated within 100 ms.
    // type: "info" | "success" | "warning" | "error" | "system" | "mutation"
    void log(const std::string& msg, const std::string& type = "info");

    // Append a permanent history record.
    void addHistory(const HistoryEntry& entry);

    const std::deque<LogEntry>&     logs()    const { return m_logs; }
    const std::vector<HistoryEntry>& history() const { return m_history; }

private:
    std::deque<LogEntry>      m_logs;
    std::vector<HistoryEntry> m_history;
};
