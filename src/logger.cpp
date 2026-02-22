#include "logger.h"
#include "util.h"

#include <SDL3/SDL.h>

void AppLogger::log(const std::string& msg, const std::string& type) {
    uint64_t t = static_cast<uint64_t>(SDL_GetTicks());

    // Deduplicate within 100 ms
    if (!m_logs.empty()) {
        const auto& last = m_logs.back();
        if (last.message == msg && (t - last.timestamp) < 100)
            return;
    }

    m_logs.push_back({ randomId(), t, msg, type });
    if (m_logs.size() > MAX_LOG_ENTRIES)
        m_logs.pop_front();
}

void AppLogger::addHistory(const HistoryEntry& entry) {
    m_history.push_back(entry);
}
