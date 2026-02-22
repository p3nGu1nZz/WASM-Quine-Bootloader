#include "wasm/wasm_parser.h"
#include <sstream>
#include <iomanip>

std::string getOpcodeName(uint8_t byte) {
    switch (byte) {
        case 0x00: return "unreachable";
        case 0x01: return "nop";
        case 0x0B: return "end";
        case 0x1A: return "drop";
        case 0x20: return "local.get";
        case 0x21: return "local.set";
        case 0x22: return "local.tee";
        case 0x41: return "i32.const";
        case 0x10: return "call";
        default: {
            std::ostringstream ss;
            ss << "0x" << std::uppercase << std::hex << std::setw(2)
               << std::setfill('0') << (int)byte;
            return ss.str();
        }
    }
}

LEB128Result decodeLEB128(const uint8_t* bytes, size_t size, int offset) {
    uint32_t result = 0;
    int      shift  = 0;
    int      count  = 0;
    while (true) {
        if (offset + count >= (int)size) break;
        uint8_t byte = bytes[offset + count];
        result |= (uint32_t)(byte & 0x7F) << shift;
        shift += 7;
        count++;
        if ((byte & 0x80) == 0) break;
    }
    if (count == 0 && offset < (int)size) return { bytes[offset], 1 };
    if (count == 0)                       return { 0, 0 };
    return { result, count };
}

std::vector<uint8_t> encodeLEB128(uint32_t value) {
    std::vector<uint8_t> bytes;
    if (value == 0) { bytes.push_back(0); return bytes; }
    while (true) {
        uint8_t byte = (uint8_t)(value & 0x7F);
        value >>= 7;
        if (value == 0) {
            bytes.push_back(byte);
            break;
        } else {
            bytes.push_back(byte | 0x80);
        }
    }
    return bytes;
}

std::vector<Instruction> parseInstructions(const uint8_t* data, size_t len) {
    std::vector<Instruction> instructions;
    int ptr = 0;
    while (ptr < (int)len) {
        uint8_t opcode = data[ptr];
        std::vector<uint8_t> args;
        int instLen = 1;

        if (opcode == 0x41) { // i32.const
            auto leb = decodeLEB128(data, len, ptr + 1);
            instLen += leb.length;
            args.assign(data + ptr + 1, data + ptr + instLen);
        } else if (opcode == 0x20 || opcode == 0x21 ||
                   opcode == 0x22 || opcode == 0x10) {
            auto leb = decodeLEB128(data, len, ptr + 1);
            instLen += leb.length;
            args.assign(data + ptr + 1, data + ptr + instLen);
        } else if (opcode == 0x04) { // if
            instLen = 2; // opcode + blocktype byte
            args.assign(data + ptr + 1, data + ptr + instLen);
        }

        if (ptr + instLen > (int)len) {
            instLen = (int)len - ptr;
            if (instLen > 1)
                args.assign(data + ptr + 1, data + ptr + instLen);
            else
                args.clear();
        }

        instructions.push_back({ opcode, args, instLen, ptr });
        ptr += instLen;
    }
    return instructions;
}

std::vector<Instruction> extractCodeSection(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < 8) return {};

    int  ptr                   = 8;
    int  codeSectionStart      = -1;
    int  codeSectionContentStart = -1;

    while (ptr < (int)bytes.size()) {
        uint8_t id      = bytes[ptr];
        auto    sizeData = decodeLEB128(bytes.data(), bytes.size(), ptr + 1);
        if (id == 10) {
            codeSectionStart       = ptr;
            codeSectionContentStart = ptr + 1 + sizeData.length;
            break;
        }
        ptr = ptr + 1 + sizeData.length + (int)sizeData.value;
    }

    if (codeSectionStart == -1) return {};

    auto numFuncsData    = decodeLEB128(bytes.data(), bytes.size(), codeSectionContentStart);
    int  funcBodySizeOff = codeSectionContentStart + numFuncsData.length;
    auto funcBodySizeData = decodeLEB128(bytes.data(), bytes.size(), funcBodySizeOff);
    int  funcContentStart = funcBodySizeOff + funcBodySizeData.length;

    auto localCountData = decodeLEB128(bytes.data(), bytes.size(), funcContentStart);
    int  instrPtr       = funcContentStart + localCountData.length;

    for (int i = 0; i < (int)localCountData.value; i++) {
        auto countData = decodeLEB128(bytes.data(), bytes.size(), instrPtr);
        instrPtr += countData.length + 1;
    }

    int instructionStart = instrPtr;
    int funcEnd          = funcContentStart + (int)funcBodySizeData.value;
    int endOpIndex       = funcEnd - 1;

    if (instructionStart >= endOpIndex) return {};

    return parseInstructions(bytes.data() + instructionStart,
                             (size_t)(endOpIndex - instructionStart));
}
