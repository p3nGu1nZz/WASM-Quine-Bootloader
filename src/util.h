#pragma once

#include "types.h"
#include <string>

// Convert enum values to display strings
std::string stateStr(SystemState s);
std::string eraStr(SystemEra e);

// Generate a short random alphanumeric ID (9 chars)
std::string randomId();

// Return current UTC time as ISO-8601 string (e.g. "2026-01-02T03:04:05.678Z")
std::string nowIso();

// Return a filename-safe UTC timestamp (e.g. "20260102_030405")
std::string nowFileStamp();
