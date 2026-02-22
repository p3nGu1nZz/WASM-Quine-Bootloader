#pragma once

#include "types.h"
#include <string>
#include <SDL3/SDL.h>

// Convert enum values to display strings
std::string stateStr(SystemState s);

// DPI scaling helpers (scale factor relative to 96 DPI)
float dpiScaleFromDpi(float ddpi);
// compute scale using an SDL_Window handle
float computeDpiScale(SDL_Window* window);

// Generate a short random alphanumeric ID (9 chars)
std::string randomId();

// Return current UTC time as ISO-8601 string (e.g. "2026-01-02T03:04:05.678Z")
std::string nowIso();

// Return a filename-safe UTC timestamp (e.g. "20260102_030405")
std::string nowFileStamp();

// Return directory containing the running executable (Linux-specific).
// Falls back to current_path() if unreadable.
std::string executableDir();
