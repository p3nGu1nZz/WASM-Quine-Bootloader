#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

// Forward-declare wasm3 types to avoid pulling in headers here
struct M3Environment;
struct M3Runtime;
struct M3Module;
struct M3Function;
typedef M3Environment* IM3Environment;
typedef M3Runtime*     IM3Runtime;
typedef M3Module*      IM3Module;
typedef M3Function*    IM3Function;

// Per-runtime user data (defined in wasm_kernel.cpp)
struct KernelUserData;

// Callback types matching the original TypeScript interface
using LogCallback       = std::function<void(uint32_t ptr, uint32_t len,
                                              const uint8_t* mem, uint32_t memSize)>;
using GrowMemCallback   = std::function<void(uint32_t pages)>;
// request from WASM to spawn a new kernel; passes pointer/length of base64 string
using SpawnCallback     = std::function<void(uint32_t ptr, uint32_t len)>;

class WasmKernel {
public:
    WasmKernel();
    ~WasmKernel();

    bool isLoaded() const;

    // Boot with a base64-encoded WASM binary
    void bootDynamic(const std::string& glob,
                     LogCallback        logCb,
                     GrowMemCallback    growCb = {},
                     SpawnCallback      spawnCb = {});

    // Execute the exported 'run' function with source written to WASM memory
    void runDynamic(const std::string& sourceGlob);

    // Provide read-only access to linear memory; returns pointer and size.
    const uint8_t* rawMemory(uint32_t* size) const;

    // Release all wasm3 resources
    void terminate();

private:
    IM3Environment m_env     = nullptr;
    IM3Runtime     m_runtime = nullptr;
    IM3Module      m_module  = nullptr;
    IM3Function    m_runFunc = nullptr;

    std::vector<uint8_t> m_wasmBytes; // keep alive for wasm3

    LogCallback     m_logCb;
    GrowMemCallback m_growCb;

    KernelUserData* m_userData = nullptr; // owned; deleted in terminate()
};
