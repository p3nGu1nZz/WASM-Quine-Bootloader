#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "wasm/parser.h"
#include "wasm/kernel.h"
#include "base64.h"
#include <vector>
#include <string>

TEST_CASE("LEB128 encode/decode round trip", "[wasm]") {
    for (uint32_t v : {0u, 1u, 127u, 128u, 255u, 256u, 0xFFFFFFFFu}) {
        auto enc = encodeLEB128(v);
        auto dec = decodeLEB128(enc.data(), enc.size(), 0);
        REQUIRE(dec.value == v);
        REQUIRE(dec.length == (int)enc.size());
    }
}

TEST_CASE("parseInstructions handles simple sequences", "[wasm]") {
    // bytes: i32.const 42, drop, end
    std::vector<uint8_t> data = {0x41, 0x2A, 0x1A, 0x0B};
    auto instrs = parseInstructions(data.data(), data.size());
    REQUIRE(instrs.size() == 3);
    REQUIRE(instrs[0].opcode == 0x41);
    REQUIRE(instrs[0].args.size() == 1);
    REQUIRE(instrs[1].opcode == 0x1A);
    REQUIRE(instrs[2].opcode == 0x0B);
}

TEST_CASE("extractCodeSection returns empty on tiny binaries", "[wasm]") {
    std::vector<uint8_t> bytes(7, 0); // smaller than header
    auto out = extractCodeSection(bytes);
    REQUIRE(out.empty());
}

TEST_CASE("WasmKernel error paths", "[wasm]") {
    WasmKernel wk;
    // run before boot should throw
    REQUIRE_THROWS_AS(wk.runDynamic(""), std::runtime_error);
    // boot with invalid base64 -> decode returns empty which should cause parse error
    REQUIRE_THROWS_AS(wk.bootDynamic("!!!notbase64!!!", {}, {}), std::exception);
    wk.terminate();
}
