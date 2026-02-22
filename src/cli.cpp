#include "cli.h"
#include <cstring>
#include <cstdlib>

static TelemetryLevel parseTelemetryLevel(const char* v) {
    if (std::strcmp(v, "none") == 0) return TelemetryLevel::NONE;
    if (std::strcmp(v, "full") == 0) return TelemetryLevel::FULL;
    return TelemetryLevel::BASIC;
}

static MutationStrategy parseMutationStrategy(const char* v) {
    if (std::strcmp(v, "blacklist") == 0) return MutationStrategy::BLACKLIST;
    if (std::strcmp(v, "smart") == 0) return MutationStrategy::SMART;
    return MutationStrategy::RANDOM;
}

static HeuristicMode parseHeuristicMode(const char* v) {
    if (std::strcmp(v, "blacklist") == 0) return HeuristicMode::BLACKLIST;
    if (std::strcmp(v, "decay") == 0) return HeuristicMode::DECAY;
    return HeuristicMode::NONE;
}

CliOptions parseCli(int argc, char** argv) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string argstr = argv[i];
        if (argstr == "--gui") {
            opts.useGui = true;
        } else if (argstr == "--headless" || argstr == "--no-gui" ||
                   argstr == "--nogui") {
            opts.useGui = false;
        } else if (argstr == "--fullscreen") {
            opts.fullscreen = true;
        } else if (argstr == "--windowed") {
            opts.fullscreen = false;
        } else if (argstr.rfind("--telemetry-level", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.telemetryLevel = parseTelemetryLevel(val);
        } else if (argstr.rfind("--telemetry-dir", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.telemetryDir = val;
        } else if (argstr.rfind("--mutation-strategy", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.mutationStrategy = parseMutationStrategy(val);
        } else if (argstr.rfind("--heuristic", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.heuristic = parseHeuristicMode(val);
        } else if (argstr == "--profile") {
            opts.profile = true;
        } else if (argstr.rfind("--max-gen", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.maxGen = std::atoi(val);
        } else if (argstr.rfind("--save-model", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.saveModelPath = val;
        } else if (argstr.rfind("--load-model", 0) == 0) {
            const char* val = nullptr;
            auto pos = argstr.find('=');
            if (pos != std::string::npos) val = argstr.c_str() + pos + 1;
            else if (i + 1 < argc) val = argv[++i];
            if (val) opts.loadModelPath = val;
        }
        // unrecognised args are silently ignored; the run.sh script
        // forwards remaining arguments to the executable so they can be
        // consumed by the App if necessary.
    }
    return opts;
}
