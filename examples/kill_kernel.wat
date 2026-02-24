;; kill_kernel.wat – example kernel that calls env.kill_instance(0) to
;; request the host remove instance 0.  This demonstrates the kill_instance
;; host import available to every kernel.
;;
;; Host imports used:
;;   env.log(ptr i32, len i32)          – write [ptr,ptr+len) from WASM memory
;;   env.grow_memory(pages i32)         – request additional WASM pages
;;   env.kill_instance(idx i32)         – ask the host to remove instance idx
;;
;; Build (requires wat2wasm from the WebAssembly Binary Toolkit):
;;   wat2wasm kill_kernel.wat -o kill_kernel.wasm
;;
;; The pre-built base64 blob is embedded in test/test_examples.cpp and kept
;; in sync with this source.  Re-run the generation script if you change it.
(module
  (type $t0 (func (param i32 i32)))
  (type $t1 (func (param i32)))
  (import "env" "log"           (func $log           (type $t0)))
  (import "env" "grow_memory"   (func $grow_memory   (type $t1)))
  (import "env" "kill_instance" (func $kill_instance (type $t1)))
  (memory (export "memory") 1)
  (func (export "run") (param $ptr i32) (param $len i32)
    ;; request removal of instance 0
    (call $kill_instance (i32.const 0))
  )
)
