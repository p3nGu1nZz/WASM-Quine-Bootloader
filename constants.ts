// A valid minimal WASM binary encoded in Base64.
// This module exports a 'run' function and imports 'env.log' AND 'env.grow_memory'.
// It acts as a bridge to allow the host to inject data (the source) and the module to "echo" it back.
//
// WAT Source for reference:
// (module
//   (type $t0 (func (param i32 i32)))
//   (type $t1 (func (param i32)))
//   (import "env" "log" (func $log (type $t0)))
//   (import "env" "grow_memory" (func $grow_memory (type $t1)))
//   (memory (export "memory") 1)
//   (func (export "run") (param $ptr i32) (param $len i32)
//     (call $log (local.get $ptr) (local.get $len))
//     (nop) ;; Added for padding/stability
//   )
// )
//
// Hex: 0061736d01000000010a0260027f7f0060017f00021d0203656e76036c6f67000003656e760b67726f775f6d656d6f72790001030201000503010001071002066d656d6f727902000372756e00020a0b010900200020011000010b
export const KERNEL_GLOB = "AGFzbQEAAAABCgJgAn9/AGABfwACHQIDZW52A2xvZwAAA2Vudgtncm93X21lbW9yeQABAwIBAAUDAQABBxACBm1lbW9yeQIAA3J1bgACCgsBCQAgACABEAABCw==";

export const DEFAULT_BOOT_CONFIG = {
  memorySizePages: 1,
  autoReboot: true,
  rebootDelayMs: 2000,
};