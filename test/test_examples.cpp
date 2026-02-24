#include <catch2/catch_test_macros.hpp>

#include "wasm/kernel.h"
#include "base64.h"

// ─── Pre-computed WASM blobs (base64) ────────────────────────────────────────
// Each constant was generated from the corresponding WAT file in examples/.
// To regenerate after editing a WAT:
//   wat2wasm examples/<name>.wat -o /tmp/<name>.wasm
//   python3 -c "import base64,sys; print(base64.b64encode(open('/tmp/<name>.wasm','rb').read()).decode())"
//
// WAT: examples/spawn_kernel.wat
// Imports: env.log, env.grow_memory, env.spawn
// Exported: run(ptr i32, len i32)
// Behaviour: calls spawn(ptr,len) then log(0,0)
static const char* SPAWN_KERNEL_B64 =
    "AGFzbQEAAAABCgJgAn9/AGABfwACKQMDZW52A2xvZwAAA2Vudgtncm93X21lbW9yeQABA2VudgVzcGF3"
    "bgAAAwIBAAUDAQABBxACBm1lbW9yeQIAA3J1bgADChABDgAgACABEAJBAEEAEAAL";

// WAT: examples/kill_kernel.wat
// Imports: env.log, env.grow_memory, env.kill_instance
// Exported: run(ptr i32, len i32)
// Behaviour: calls kill_instance(0)
static const char* KILL_KERNEL_B64 =
    "AGFzbQEAAAABCgJgAn9/AGABfwACMQMDZW52A2xvZwAAA2Vudgtncm93X21lbW9yeQABA2Vudg1raWxs"
    "X2luc3RhbmNlAAEDAgEABQMBAAEHEAIGbWVtb3J5AgADcnVuAAMKCAEGAEEAEAIL";

// WAT: examples/weight_kernel.wat
// Imports: env.log, env.grow_memory, env.record_weight
// Exported: run(ptr i32, len i32)
// Behaviour: calls record_weight(0, 4)
static const char* WEIGHT_KERNEL_B64 =
    "AGFzbQEAAAABCgJgAn9/AGABfwACMQMDZW52A2xvZwAAA2Vudgtncm93X21lbW9yeQABA2Vudg1yZWNv"
    "cmRfd2VpZ2h0AAADAgEABQMBAAEHEAIGbWVtb3J5AgADcnVuAAMKCgEIAEEAQQQQAgs=";

// ─── Tests ────────────────────────────────────────────────────────────────────

TEST_CASE("spawn_kernel: boots and fires spawn callback", "[examples][spawn]") {
    // Verify that the blob decodes to a non-empty binary
    auto bytes = base64_decode(SPAWN_KERNEL_B64);
    REQUIRE(!bytes.empty());

    bool spawnFired = false;
    WasmKernel wk;
    REQUIRE_NOTHROW(wk.bootDynamic(
        SPAWN_KERNEL_B64,
        /*logCb=*/   {},
        /*growCb=*/  {},
        /*spawnCb=*/ [&](uint32_t, uint32_t) { spawnFired = true; }
    ));
    REQUIRE(wk.isLoaded());

    // run() receives the kernel's own base64 as the "source" written to WASM
    // memory; the kernel forwards it to spawn().
    REQUIRE_NOTHROW(wk.runDynamic(SPAWN_KERNEL_B64));
    REQUIRE(spawnFired);
}

TEST_CASE("kill_kernel: boots and fires kill callback", "[examples][kill]") {
    auto bytes = base64_decode(KILL_KERNEL_B64);
    REQUIRE(!bytes.empty());

    bool killFired = false;
    WasmKernel wk;
    REQUIRE_NOTHROW(wk.bootDynamic(
        KILL_KERNEL_B64,
        /*logCb=*/    {},
        /*growCb=*/   {},
        /*spawnCb=*/  {},
        /*weightCb=*/ {},
        /*killCb=*/   [&](int32_t idx) { if (idx == 0) killFired = true; }
    ));
    REQUIRE(wk.isLoaded());

    REQUIRE_NOTHROW(wk.runDynamic(""));
    REQUIRE(killFired);
}

TEST_CASE("weight_kernel: boots and fires record_weight callback", "[examples][weight]") {
    auto bytes = base64_decode(WEIGHT_KERNEL_B64);
    REQUIRE(!bytes.empty());

    bool weightFired = false;
    uint32_t capturedLen = 0;
    WasmKernel wk;
    REQUIRE_NOTHROW(wk.bootDynamic(
        WEIGHT_KERNEL_B64,
        /*logCb=*/    {},
        /*growCb=*/   {},
        /*spawnCb=*/  {},
        /*weightCb=*/ [&](uint32_t, uint32_t len) {
            weightFired  = true;
            capturedLen  = len;
        }
    ));
    REQUIRE(wk.isLoaded());

    REQUIRE_NOTHROW(wk.runDynamic(""));
    REQUIRE(weightFired);
    REQUIRE(capturedLen == 4u);
}
