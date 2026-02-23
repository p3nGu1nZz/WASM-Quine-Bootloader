#include "cli.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

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

static TelemetryFormat parseTelemetryFormat(const char* v) {
    if (std::strcmp(v, "json") == 0) return TelemetryFormat::JSON;
    return TelemetryFormat::TEXT;
}

// small helper to extract an option value either from "--opt=val" or
// by consuming the next argv element.  `i` is incremented when the value is
// taken from the following argument so callers don't have to duplicate the
// boilerplate.
static const char* extractValue(const std::string& arg, int& i, int argc, char** argv) {
    auto pos = arg.find('=');
    if (pos != std::string::npos)
        return arg.c_str() + pos + 1;
    if (i + 1 < argc)
        return argv[++i];
    return nullptr;
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
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) {
                opts.telemetryLevel = parseTelemetryLevel(val);
                if ((opts.telemetryLevel == TelemetryLevel::BASIC &&
                     std::strcmp(val, "basic") != 0) &&
                    std::strcmp(val, "none") != 0 &&
                    std::strcmp(val, "full") != 0) {
                    std::cerr << "Warning: unknown telemetry-level '" << val << "'\n";
                    opts.parseError = true;
                }
            }
        } else if (argstr.rfind("--telemetry-dir", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) opts.telemetryDir = val;
        } else if (argstr.rfind("--telemetry-format", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) {
                opts.telemetryFormat = parseTelemetryFormat(val);
                if (opts.telemetryFormat == TelemetryFormat::TEXT &&
                    std::strcmp(val, "text") != 0) {
                    std::cerr << "Warning: unknown telemetry-format '" << val << "'\n";
                    opts.parseError = true;
                }
            }
        } else if (argstr.rfind("--mutation-strategy", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) {
                opts.mutationStrategy = parseMutationStrategy(val);
                if (opts.mutationStrategy == MutationStrategy::RANDOM &&
                    std::strcmp(val, "random") != 0) {
                    std::cerr << "Warning: unknown mutation-strategy '" << val << "'\n";
                    opts.parseError = true;
                }
            }
        } else if (argstr.rfind("--heuristic", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) {
                opts.heuristic = parseHeuristicMode(val);
                if (opts.heuristic == HeuristicMode::NONE &&
                    std::strcmp(val, "none") != 0) {
                    std::cerr << "Warning: unknown heuristic '" << val << "'\n";
                    opts.parseError = true;
                }
            }
        } else if (argstr == "--profile") {
            opts.profile = true;
        } else if (argstr.rfind("--max-gen", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) {
                char* end;
                long v = std::strtol(val, &end, 10);
                if (*end != '\0' || v < 0) {
                    std::cerr << "Warning: invalid max-gen '" << val << "'\n";
                    opts.parseError = true;
                } else {
                    opts.maxGen = static_cast<int>(v);
                }
            }
        } else if (argstr.rfind("--save-model", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) opts.saveModelPath = val;
        } else if (argstr.rfind("--load-model", 0) == 0) {
            const char* val = extractValue(argstr, i, argc, argv);
            if (val) opts.loadModelPath = val;
        } else {
            std::cerr << "Warning: unrecognised option '" << argstr << "'\n";
            opts.parseError = true;
        }
        // unrecognised args are silently ignored; the run.sh script
        // forwards remaining arguments to the executable so they can be
        // consumed by the App if necessary.
    }
    return opts;
}
