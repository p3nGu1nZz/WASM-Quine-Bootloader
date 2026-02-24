#pragma once

#include "types.h"
#include <string>

// A valid minimal WASM binary encoded in Base64.
// This module exports a 'run' function and imports 'env.log' AND 'env.grow_memory'.
// It acts as a bridge: host writes source to WASM memory, module echoes it back via log.
//
// WAT Source:
// (module
//   (type $t0 (func (param i32 i32)))
//   (type $t1 (func (param i32)))
//   (import "env" "log" (func $log (type $t0)))
//   (import "env" "grow_memory" (func $grow_memory (type $t1)))
//   (memory (export "memory") 1)
//   (func (export "run") (param $ptr i32) (param $len i32)
//     (call $log (local.get $ptr) (local.get $len))
//     (nop)
//   )
// )
//
// Hex: 0061736d01000000010a0260027f7f0060017f00021d0203656e76036c6f67000003656e760b67726f775f6d656d6f72790001030201000503010001071002066d656d6f727902000372756e00020a0b010900200020011000010b
static const std::string KERNEL_GLOB =
    "AGFzbQEAAAABCgJgAn9/AGABfwACHQIDZW52A2xvZwAAA2Vudgtncm93X21lbW9yeQABAwIBAAUDAQAB"
    "BxACBm1lbW9yeQIAA3J1bgACCgsBCQAgACABEAABCw==";

// ── Example kernels (see examples/ directory for WAT sources) ──────────────
//
// SPAWN_KERNEL_GLOB
// Imports env.log and env.spawn.  run(ptr,len) echoes the source via log
// then calls spawn(ptr,len) to request a sibling instance.
//
// WAT: examples/spawn_kernel.wat
static const std::string SPAWN_KERNEL_GLOB =
    "AGFzbQEAAAABBgFgAn9/AAIXAgNlbnYDbG9nAAADZW52BXNwYXduAAADAgEABQMBAAEHEAIGbWVtb3J5"
    "AgADcnVuAAIKEAEOACAAIAEQACAAIAEQAQs=";

// KILL_KERNEL_GLOB
// Imports env.log and env.kill_instance.  run(ptr,len) logs then calls
// kill_instance(0) to ask the bootloader to remove instance 0.
//
// WAT: examples/kill_kernel.wat
static const std::string KILL_KERNEL_GLOB =
    "AGFzbQEAAAABCgJgAn9/AGABfwACHwIDZW52A2xvZwAAA2Vudg1raWxsX2luc3RhbmNlAAEDAgEABQMB"
    "AAEHEAIGbWVtb3J5AgADcnVuAAIKDgEMACAAIAEQAEEAEAEL";

// WEIGHT_KERNEL_GLOB
// Imports env.log and env.record_weight.  run(ptr,len) logs then sends the
// same memory slice as a dummy neural-matrix blob via record_weight.
//
// WAT: examples/weight_kernel.wat
static const std::string WEIGHT_KERNEL_GLOB =
    "AGFzbQEAAAABBgFgAn9/AAIfAgNlbnYDbG9nAAADZW52DXJlY29yZF93ZWlnaHQAAAMCAQAFAwEAAQcQ"
    "AgZtZW1vcnkCAANydW4AAgoQAQ4AIAAgARAAIAAgARABCw==";

static const BootConfig DEFAULT_BOOT_CONFIG = {
    /* memorySizePages */ 1,
    /* autoReboot      */ true,
    /* rebootDelayMs   */ 2000,
};
