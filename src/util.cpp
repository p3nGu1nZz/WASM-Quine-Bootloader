#include "util.h"

#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <ctime>

std::string stateStr(SystemState s) {
    switch (s) {
        case SystemState::IDLE:            return "IDLE";
        case SystemState::BOOTING:         return "BOOTING";
        case SystemState::LOADING_KERNEL:  return "LOADING_KERNEL";
        case SystemState::EXECUTING:       return "EXECUTING";
        case SystemState::VERIFYING_QUINE: return "VERIFYING_QUINE";
        case SystemState::SYSTEM_HALT:     return "SYSTEM_HALT";
        case SystemState::REPAIRING:       return "REPAIRING";
    }
    return "UNKNOWN";
}

// -----------------------------------------------------------------------------
// DPI scaling utilities
// -----------------------------------------------------------------------------

// simple helper calculating scale factor relative to 96 dpi (unused now)
float dpiScaleFromDpi(float ddpi) {
    if (ddpi <= 0.0f) return 1.0f;
    return ddpi / 96.0f;
}

// compute a font/UI scale based purely on window size
float computeDpiScale(SDL_Window* window) {
    if (!window) return 1.0f;
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);
    if (w <= 0 || h <= 0) return 1.0f;
    // baseline resolution (chosen empirically)
    const float baseW = 1400.0f;
    const float baseH = 900.0f;
    float sx = (float)w / baseW;
    float sy = (float)h / baseH;
    float scale = std::max(sx, sy);
    return scale < 1.0f ? 1.0f : scale;
}


std::string randomId() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 35);
    const char* ch = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string id(9, ' ');
    for (char& c : id) c = ch[dist(rng)];
    return id;
}

std::string nowIso() {
    using namespace std::chrono;
    auto tp = system_clock::now();
    auto tt = system_clock::to_time_t(tp);
    auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S", &utc);
    std::ostringstream ss;
    ss << buf << '.' << std::setw(3) << std::setfill('0') << ms.count() << 'Z';
    return ss.str();
}

std::string nowFileStamp() {
    using namespace std::chrono;
    auto tp = system_clock::now();
    auto tt = system_clock::to_time_t(tp);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    char buf[20];
    std::strftime(buf, sizeof buf, "%Y%m%d_%H%M%S", &utc);
    return buf;
}
