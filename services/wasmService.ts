import { KERNEL_GLOB } from '../constants';

export class WasmKernel {
  private instance: WebAssembly.Instance | null = null;
  private memory: WebAssembly.Memory | null = null;
  
  // Converts Base64 to Uint8Array
  private decodeGlob(glob: string): Uint8Array {
    const binaryString = atob(glob);
    const bytes = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
      bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes;
  }

  public isLoaded(): boolean {
      return this.instance !== null && this.memory !== null;
  }

  // Boot the kernel with a dynamic binary
  public async bootDynamic(
    glob: string,
    logCallback: (ptr: number, len: number, memory: WebAssembly.Memory) => void,
    growCallback?: (pages: number) => void
  ): Promise<void> {
    this.terminate(); // Ensure clean slate before booting
    
    const wasmBytes = this.decodeGlob(glob);

    const importObject = {
      env: {
        log: (ptr: number, len: number) => {
          if (this.memory) {
            logCallback(ptr, len, this.memory);
          }
        },
        grow_memory: (pages: number) => {
          if (this.memory) {
            try {
              const oldSize = this.memory.buffer.byteLength;
              this.memory.grow(pages);
              console.log(`[WASM System] Memory grew by ${pages} page(s). New size: ${this.memory.buffer.byteLength} bytes.`);
              if (growCallback) {
                growCallback(pages);
              }
            } catch (e) {
              console.error("[WASM System] grow_memory failed:", e);
            }
          }
        }
      },
    };

    try {
      const wasmModule = await WebAssembly.instantiate(wasmBytes, importObject);
      this.instance = wasmModule.instance;
      this.memory = this.instance.exports.memory as WebAssembly.Memory;
    } catch (error) {
      console.error("WASM Boot Failure:", error);
      this.terminate();
      throw error; // Re-throw for handling in App
    }
  }

  // Legacy boot for backward compatibility if needed, but we use bootDynamic now
  public async boot(
    logCallback: (ptr: number, len: number, memory: WebAssembly.Memory) => void
  ): Promise<void> {
    return this.bootDynamic(KERNEL_GLOB, logCallback);
  }

  public runDynamic(sourceGlob: string): void {
    if (!this.instance || !this.memory) {
      throw new Error("Kernel Panic: No instance found. Boot first.");
    }

    const runFunc = this.instance.exports.run as (ptr: number, len: number) => void;
    
    // 1. Inject the provided Source Glob into Memory
    const encoder = new TextEncoder();
    const encodedSource = encoder.encode(sourceGlob);
    
    const memoryBuffer = new Uint8Array(this.memory.buffer);
    // Ensure memory is large enough (should be, it's 1 page / 64KB, glob is tiny)
    memoryBuffer.set(encodedSource, 0);

    // 2. Execute
    runFunc(0, encodedSource.length);
  }
  
  public run(): void {
      this.runDynamic(KERNEL_GLOB);
  }

  public terminate(): void {
    this.instance = null;
    this.memory = null;
  }
}

export const wasmKernel = new WasmKernel();