#pragma once

#include <string>
#include <vector>

// simple record extracted from a telemetry export
struct TelemetryEntry {
    int generation = 0;
    std::string kernelBase64;
    std::string trapCode;
};

// Advisor loads all telemetry exports under a given base
// directory and makes them available for training/advice.
class Advisor {
public:
    explicit Advisor(const std::string& baseDir = "bin/seq");

    // number of entries successfully parsed
    size_t size() const { return m_entries.size(); }

    const std::vector<TelemetryEntry>& entries() const { return m_entries; }

private:
    void scanDirectory(const std::string& runDir);
    void parseFile(const std::string& path);

    std::vector<TelemetryEntry> m_entries;
};
