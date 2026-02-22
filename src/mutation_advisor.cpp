#include "mutation_advisor.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;

MutationAdvisor::MutationAdvisor(const std::string& baseDir) {
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

void MutationAdvisor::scanDirectory(const std::string& runDir) {
    for (auto& entry : fs::directory_iterator(runDir)) {
        if (!entry.is_regular_file()) continue;
        const auto& name = entry.path().filename().string();
        if (name.rfind("gen_", 0) == 0 && name.find(".txt") != std::string::npos) {
            parseFile(entry.path().string());
        }
    }
}

void MutationAdvisor::parseFile(const std::string& path) {
    std::ifstream fin(path);
    if (!fin) return;

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
