#pragma once

#include "types.h"
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <filesystem>

// Convert enum values to display strings
std::string stateStr(SystemState s);

// DPI scaling helpers (scale factor relative to 96 DPI)
float dpiScaleFromDpi(float ddpi);
// compute scale using an SDL_Window handle
float computeDpiScale(SDL_Window* window);

// Generate a short random alphanumeric ID (9 chars)
std::string randomId();

// Decode a base64 string with an internal cache to avoid repeated work.
const std::vector<uint8_t>& decodeBase64Cached(const std::string& b64);

// Return current UTC time as ISO-8601 string (e.g. "2026-01-02T03:04:05.678Z")
std::string nowIso();

// Return a filename-safe UTC timestamp (e.g. "20260102_030405")
std::string nowFileStamp();

// Return directory containing the running executable (Linux-specific).
// Falls back to current_path() if unreadable.
std::string executableDir();

// Given a run identifier, return the sequence export directory where
// the telemetry for that run should be written.  The result is
// "<exe_dir>/bin/seq/<runId>".
std::filesystem::path sequenceDir(const std::string& runId);
