#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <optional>

struct Instruction {
    uint8_t              opcode;
    std::vector<uint8_t> args;
    int                  length;
    int                  originalOffset;
};

std::string getOpcodeName(uint8_t byte);

struct LEB128Result {
    uint32_t value;
    int      length;
};

LEB128Result decodeLEB128(const uint8_t* bytes, size_t size, int offset);
std::vector<uint8_t> encodeLEB128(uint32_t value);

std::vector<Instruction> parseInstructions(const uint8_t* data, size_t len);

// Returns instructions from the code section of a WASM binary.
// Returns empty vector if code section not found.
std::vector<Instruction> extractCodeSection(const std::vector<uint8_t>& bytes);
