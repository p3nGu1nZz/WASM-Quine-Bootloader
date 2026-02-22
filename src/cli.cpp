#include "cli.h"
#include <cstring>
#include <cstdlib>

CliOptions parseCli(int argc, char** argv) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "--gui") == 0) {
            opts.useGui = true;
        } else if (std::strcmp(arg, "--headless") == 0 ||
                   std::strcmp(arg, "--no-gui") == 0 ||
                   std::strcmp(arg, "--nogui") == 0) {
            opts.useGui = false;
        } else if (std::strcmp(arg, "--fullscreen") == 0) {
            opts.fullscreen = true;
        } else if (std::strcmp(arg, "--windowed") == 0) {
            opts.fullscreen = false;
        }
        // unrecognised args are silently ignored; the run.sh script
        // forwards remaining arguments to the executable so they can be
        // consumed by the App if necessary.
    }
    return opts;
}
