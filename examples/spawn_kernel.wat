;; spawn_kernel.wat – example kernel that calls env.spawn with the source
;; pointer/length received in run(), effectively requesting the host to
;; record a copy of this kernel as a new instance.  It then calls env.log
;; with an empty range to signal success.
;;
;; Host imports used:
;;   env.log(ptr i32, len i32)          – write [ptr,ptr+len) from WASM memory
;;   env.grow_memory(pages i32)         – request additional WASM pages
;;   env.spawn(ptr i32, len i32)        – spawn a new kernel from base64 data
;;
;; Build (requires wat2wasm from the WebAssembly Binary Toolkit):
;;   wat2wasm spawn_kernel.wat -o spawn_kernel.wasm
;;
;; The pre-built base64 blob is embedded in test/test_examples.cpp and kept
;; in sync with this source.  Re-run the generation script if you change it.
(module
  (type $t0 (func (param i32 i32)))
  (type $t1 (func (param i32)))
  (import "env" "log"         (func $log         (type $t0)))
  (import "env" "grow_memory" (func $grow_memory (type $t1)))
  (import "env" "spawn"       (func $spawn       (type $t0)))
  (memory (export "memory") 1)
  (func (export "run") (param $ptr i32) (param $len i32)
    ;; forward the source blob straight to spawn so the host records a copy
    (call $spawn (local.get $ptr) (local.get $len))
    ;; log with a zero-length range as a success signal
    (call $log (i32.const 0) (i32.const 0))
  )
)
