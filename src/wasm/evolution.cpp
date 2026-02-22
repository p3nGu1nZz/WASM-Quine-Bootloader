#include "wasm/evolution.h"
#include "base64.h"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <random>

// Thread-local Mersenne Twister for determinism-free mutations
static std::mt19937& rng() {
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

static float randF() {
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng());
}

static int randInt(int n) {
    return (int)std::uniform_int_distribution<int>(0, n - 1)(rng());
}

static std::vector<uint8_t> generateRandomConstDrop() {
    return { 0x41, (uint8_t)(randInt(128)), 0x1A };
}

static std::vector<uint8_t> generateSafeMath() {
    static const uint8_t ops[] = { 0x6A, 0x6B, 0x71, 0x72, 0x73 };
    uint8_t op = ops[randInt(5)];
    return {
        0x41, (uint8_t)(randInt(128)),
        0x41, (uint8_t)(randInt(128)),
        op,
        0x1A
    };
}

static std::vector<uint8_t> generateLocalTee() {
    return { 0x41, (uint8_t)(randInt(255)), 0x22, 0x00, 0x1A };
}

static std::vector<uint8_t> generateIfTrue() {
    return {
        0x41, 0x01,
        0x04, 0x40,
        0x41, (uint8_t)(randInt(64)),
        0x1A,
        0x0B
    };
}

static const std::vector<std::vector<uint8_t>> BASE_SAFE_GENOMES = {
    { 0x20, 0x00, 0x1A },
    { 0x20, 0x01, 0x1A },
    { 0x41, 0x00, 0x1A },
    { 0x41, 0x01, 0x1A },
};

static std::string describeSequence(const std::vector<uint8_t>& seq) {
    if (seq.empty()) return "empty";
    auto instrs = parseInstructions(seq.data(), seq.size());
    std::string desc;
    for (size_t i = 0; i < instrs.size(); i++) {
        if (i) desc += ", ";
        desc += getOpcodeName(instrs[i].opcode);
        if (!instrs[i].args.empty()) {
            std::ostringstream ss;
            ss << " " << (int)instrs[i].args[0];
            desc += ss.str();
        }
    }
    return desc;
}

static std::vector<uint8_t> flatten(const std::vector<Instruction>& instrs) {
    std::vector<uint8_t> out;
    for (const auto& inst : instrs) {
        out.push_back(inst.opcode);
        out.insert(out.end(), inst.args.begin(), inst.args.end());
    }
    return out;
}

static std::vector<uint8_t> getGenome(
    const std::vector<std::vector<uint8_t>>& known)
{
    float r = randF();
    if (known.size() > 2 && r < 0.7f)
        return known[randInt((int)known.size())];

    float s = randF();
    if (s < 0.30f) return generateRandomConstDrop();
    if (s < 0.60f) return generateSafeMath();
    if (s < 0.80f) return generateLocalTee();
    if (s < 0.95f) return generateIfTrue();
    return BASE_SAFE_GENOMES[randInt((int)BASE_SAFE_GENOMES.size())];
}

EvolutionResult evolveBinary(
    const std::string&                       currentBase64,
    const std::vector<std::vector<uint8_t>>& knownInstructions,
    int                                      attemptSeed)
{
    std::vector<uint8_t> bytes = base64_decode(currentBase64);

    // 1. Locate code section
    int ptr                    = 8;
    int codeSectionStart       = -1;
    int codeSectionContentStart = -1;

    while (ptr < (int)bytes.size()) {
        uint8_t id       = bytes[ptr];
        auto    sizeData = decodeLEB128(bytes.data(), bytes.size(), ptr + 1);
        if (id == 10) {
            codeSectionStart        = ptr;
            codeSectionContentStart = ptr + 1 + sizeData.length;
            break;
        }
        ptr = ptr + 1 + sizeData.length + (int)sizeData.value;
    }
    if (codeSectionStart == -1)
        throw std::runtime_error("Code section missing");

    // 2. Locate function body
    auto numFuncsData      = decodeLEB128(bytes.data(), bytes.size(), codeSectionContentStart);
    int  funcBodySizeOff   = codeSectionContentStart + numFuncsData.length;
    auto funcBodySizeData  = decodeLEB128(bytes.data(), bytes.size(), funcBodySizeOff);
    int  funcContentStart  = funcBodySizeOff + funcBodySizeData.length;

    auto localCountData = decodeLEB128(bytes.data(), bytes.size(), funcContentStart);
    int  instrPtr       = funcContentStart + localCountData.length;

    for (int i = 0; i < (int)localCountData.value; i++) {
        auto countData = decodeLEB128(bytes.data(), bytes.size(), instrPtr);
        instrPtr += countData.length + 1;
    }

    int instructionStart = instrPtr;
    int funcEnd          = funcContentStart + (int)funcBodySizeData.value;
    int endOpIndex       = funcEnd - 1;

    // 3. Parse instructions
    std::vector<uint8_t> instrBytes(bytes.begin() + instructionStart,
                                     bytes.begin() + endOpIndex);
    auto parsedInstructions = parseInstructions(instrBytes.data(), instrBytes.size());

    // 4. Evolution logic
    int action = attemptSeed % 4;
    std::vector<uint8_t> mutationSequence;
    std::vector<uint8_t> newInstructionsBytes;
    std::string          description;

    switch (action) {
        case (int)EvolutionAction::MODIFY:
        case (int)EvolutionAction::INSERT: {
            auto seq = getGenome(knownInstructions);
            mutationSequence = seq;
            int  idx    = (int)(randF() * (float)(parsedInstructions.size() + 1));
            auto before = flatten(std::vector<Instruction>(
                parsedInstructions.begin(), parsedInstructions.begin() + idx));
            auto after  = flatten(std::vector<Instruction>(
                parsedInstructions.begin() + idx, parsedInstructions.end()));
            newInstructionsBytes = before;
            newInstructionsBytes.insert(newInstructionsBytes.end(), seq.begin(), seq.end());
            newInstructionsBytes.insert(newInstructionsBytes.end(), after.begin(), after.end());
            description = std::string(action == 0 ? "Modified" : "Inserted") +
                          ": [" + describeSequence(seq) + "] at " + std::to_string(idx);
            break;
        }
        case (int)EvolutionAction::DELETE: {
            if (!parsedInstructions.empty()) {
                int targetIdx  = -1;
                int deleteCount = 0;

                // Priority 1: delete NOPs
                for (int i = 0; i < (int)parsedInstructions.size(); i++) {
                    if (parsedInstructions[i].opcode == 0x01) {
                        targetIdx   = i;
                        deleteCount = 1;
                        description = "Deleted NOP at index " + std::to_string(i);
                        break;
                    }
                }

                // Priority 2: delete safe-math [Const,Const,Op,Drop]
                if (targetIdx == -1) {
                    for (int i = 0; i < (int)parsedInstructions.size() - 3; i++) {
                        static const uint8_t mathOps[] = {0x6A,0x6B,0x6C,0x71,0x72,0x73};
                        auto& p0 = parsedInstructions[i];
                        auto& p1 = parsedInstructions[i+1];
                        auto& p2 = parsedInstructions[i+2];
                        auto& p3 = parsedInstructions[i+3];
                        bool isMath = (p0.opcode == 0x41 && p1.opcode == 0x41 &&
                            std::find(std::begin(mathOps), std::end(mathOps), p2.opcode) != std::end(mathOps) &&
                            p3.opcode == 0x1A);
                        if (isMath && randF() < 0.6f) {
                            targetIdx   = i;
                            deleteCount = 4;
                            description = "Pruned math sequence [" + getOpcodeName(p2.opcode) + "]";
                            break;
                        }
                    }
                }

                // Priority 3: delete if-true blocks
                if (targetIdx == -1) {
                    for (int i = 0; i < (int)parsedInstructions.size() - 4; i++) {
                        auto& p0 = parsedInstructions[i];
                        auto& p1 = parsedInstructions[i+1];
                        auto& p4 = parsedInstructions[i+4];
                        if (p0.opcode == 0x41 && !p0.args.empty() && p0.args[0] == 1 &&
                            p1.opcode == 0x04 && p4.opcode == 0x0B && randF() < 0.5f) {
                            targetIdx   = i;
                            deleteCount = 5;
                            description = "Pruned control flow block";
                            break;
                        }
                    }
                }

                // Priority 4: delete producer-consumer pairs
                if (targetIdx == -1) {
                    for (int i = 0; i < (int)parsedInstructions.size() - 1; i++) {
                        auto& p0 = parsedInstructions[i];
                        auto& p1 = parsedInstructions[i+1];
                        if ((p0.opcode == 0x41 || p0.opcode == 0x20 || p0.opcode == 0x22)
                            && p1.opcode == 0x1A) {
                            targetIdx   = i;
                            deleteCount = 2;
                            description = "Deleted balanced pair [" +
                                          getOpcodeName(p0.opcode) + ", drop]";
                            break;
                        }
                    }
                }

                if (targetIdx != -1) {
                    parsedInstructions.erase(
                        parsedInstructions.begin() + targetIdx,
                        parsedInstructions.begin() + targetIdx + deleteCount);
                } else {
                    description = "No safe deletion targets found (Skipped)";
                }
                mutationSequence.clear();
            } else {
                description = "Instruction set empty";
            }
            newInstructionsBytes = flatten(parsedInstructions);
            break;
        }
        case (int)EvolutionAction::ADD: {
            auto seq = getGenome(knownInstructions);
            mutationSequence = seq;
            newInstructionsBytes = flatten(parsedInstructions);
            newInstructionsBytes.insert(newInstructionsBytes.end(), seq.begin(), seq.end());
            description = "Appended [" + describeSequence(seq) + "]";
            break;
        }
    }

    if (newInstructionsBytes.empty() && !parsedInstructions.empty())
        newInstructionsBytes = flatten(parsedInstructions);

    // 5. Reconstruct binary
    std::vector<uint8_t> preInstructions(bytes.begin() + funcContentStart,
                                          bytes.begin() + instructionStart);
    std::vector<uint8_t> postInstructions(bytes.begin() + endOpIndex, bytes.end());

    uint32_t newFuncBodyLen = (uint32_t)(preInstructions.size() +
                                          newInstructionsBytes.size() +
                                          postInstructions.size());

    if (newFuncBodyLen > 32768)
        throw std::runtime_error("Evolution Limit: 32KB");

    auto newFuncBodySizeEnc = encodeLEB128(newFuncBodyLen);

    std::vector<uint8_t> preFuncSize(bytes.begin() + codeSectionContentStart,
                                      bytes.begin() + funcBodySizeOff);

    uint32_t newSectionContentLen = (uint32_t)(preFuncSize.size() +
                                                newFuncBodySizeEnc.size() +
                                                newFuncBodyLen);
    auto newSectionSizeEnc = encodeLEB128(newSectionContentLen);

    std::vector<uint8_t> preCode(bytes.begin(), bytes.begin() + codeSectionStart + 1);

    std::vector<uint8_t> newBytes;
    newBytes.reserve(preCode.size() + newSectionSizeEnc.size() + preFuncSize.size() +
                     newFuncBodySizeEnc.size() + preInstructions.size() +
                     newInstructionsBytes.size() + postInstructions.size());

    auto append = [&](const std::vector<uint8_t>& v) {
        newBytes.insert(newBytes.end(), v.begin(), v.end());
    };

    append(preCode);
    append(newSectionSizeEnc);
    append(preFuncSize);
    append(newFuncBodySizeEnc);
    append(preInstructions);
    append(newInstructionsBytes);
    append(postInstructions);

    return {
        base64_encode(newBytes),
        mutationSequence,
        (EvolutionAction)action,
        description
    };
}
