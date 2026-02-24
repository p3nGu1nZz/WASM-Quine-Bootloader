;; weight_kernel.wat – example kernel that calls env.record_weight with a
;; 4-byte dummy neural weight matrix located at memory offset 0.  This
;; demonstrates the record_weight host import for neural-feature work.
;;
;; Host imports used:
;;   env.log(ptr i32, len i32)            – write [ptr,ptr+len) from WASM memory
;;   env.grow_memory(pages i32)           – request additional WASM pages
;;   env.record_weight(ptr i32, len i32)  – transmit a serialized weight blob
;;
;; Build (requires wat2wasm from the WebAssembly Binary Toolkit):
;;   wat2wasm weight_kernel.wat -o weight_kernel.wasm
;;
;; The pre-built base64 blob is embedded in test/test_examples.cpp and kept
;; in sync with this source.  Re-run the generation script if you change it.
(module
  (type $t0 (func (param i32 i32)))
  (type $t1 (func (param i32)))
  (import "env" "log"           (func $log           (type $t0)))
  (import "env" "grow_memory"   (func $grow_memory   (type $t1)))
  (import "env" "record_weight" (func $record_weight (type $t0)))
  (memory (export "memory") 1)
  (func (export "run") (param $ptr i32) (param $len i32)
    ;; send 4 bytes at offset 0 as a dummy neural weight matrix
    (call $record_weight (i32.const 0) (i32.const 4))
  )
)
