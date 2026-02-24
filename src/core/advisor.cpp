#include "advisor.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>

namespace fs = std::filesystem;

Advisor::Advisor(const std::string& baseDir) {
    try {
        for (auto& run : fs::directory_iterator(baseDir)) {
            if (run.is_directory()) {
                scanDirectory(run.path().string());
            }
        }
    } catch (...) {
        // if directory doesn't exist or cannot be read, silently ignore
    }
}

void Advisor::scanDirectory(const std::string& runDir) {
    try {
        for (auto& entry : fs::directory_iterator(runDir)) {
            if (!entry.is_regular_file()) continue;
            const auto& name = entry.path().filename().string();
            if (name.rfind("gen_", 0) == 0 && name.find(".txt") != std::string::npos) {
                parseFile(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Advisor: unable to scan directory '" << runDir
                  << "': " << e.what() << "\n";
    }
}

void Advisor::parseFile(const std::string& path) {
    std::ifstream fin(path);
    if (!fin) {
        std::cerr << "Advisor: failed to open telemetry file '" << path << "'\n";
        return;
    }

    TelemetryEntry te;
    std::string line;
    bool inKernelSection = false;

    while (std::getline(fin, line)) {
        if (line.rfind("Final Generation:", 0) == 0) {
            std::istringstream iss(line.substr(17));
            iss >> te.generation;
        } else if (line.rfind("Traps:", 0) == 0) {
            te.trapCode = line.substr(6);
            // trim leading space
            if (!te.trapCode.empty() && te.trapCode[0] == ' ') te.trapCode.erase(0,1);
        } else if (line == "CURRENT KERNEL (BASE64):") {
            inKernelSection = true;
        } else if (inKernelSection) {
            if (line.empty()) break;
            te.kernelBase64 = line;
            inKernelSection = false;
        }
    }

    if (te.generation || !te.kernelBase64.empty()) {
        m_entries.push_back(std::move(te));
    }
}

float Advisor::score(const std::vector<uint8_t>& /*seq*/) const {
    // simple heuristic: use average generation number mapped into (0,1]
    if (m_entries.empty()) return 1.0f;
    float total = 0.0f;
    for (auto& e : m_entries) total += static_cast<float>(e.generation);
    float avg = total / m_entries.size();
    float s = avg <= 0.0f ? 0.1f : avg / (avg + 10.0f);
    if (s < 0.0f) s = 0.0f;
    if (s > 1.0f) s = 1.0f;
    return s;
}

bool Advisor::dump(const std::string& path) const {
    try {
        std::ofstream f(path);
        if (!f) return false;
        for (const auto& e : m_entries) {
            f << "gen=" << e.generation << " trap=" << e.trapCode << " kernel=" << e.kernelBase64 << "\n";
        }
        return true;
    } catch (...) {
        return false;
    }
}
