#pragma once

#include <string>

// Simple command-line option parsing used by the bootloader executable.
// Defaults are chosen so that the GUI is enabled and starts fullscreen,
// which matches the new "GUI by default" requirement.  A timeout value
// (seconds) may be supplied to automatically exit the main loop.

struct CliOptions {
    bool useGui      = true;   // false would eventually enable headless mode
    bool fullscreen  = true;   // only meaningful when useGui == true
};

// Parse arguments from main() argc/argv; unrecognised options are ignored
// (they will be passed through to the bootloader instance when exec'ing).
CliOptions parseCli(int argc, char** argv);
