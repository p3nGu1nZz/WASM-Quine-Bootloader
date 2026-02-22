import React, { useState, useEffect, useCallback, useRef } from 'react';
import { TerminalLog } from './components/TerminalLog';
import { SystemStatus } from './components/SystemStatus';
import { MemoryVisualizer } from './components/MemoryVisualizer';
import { InstructionStream } from './components/InstructionStream';
import { wasmKernel } from './services/wasmService';
import { evolveBinary } from './utils/wasmEvolution';
import { extractCodeSection, Instruction, getOpcodeName } from './utils/wasmParser';
import { SystemState, LogEntry, SystemEra, HistoryEntry } from './types';
import { KERNEL_GLOB, DEFAULT_BOOT_CONFIG } from './constants';

const delay = (ms: number) => new Promise(resolve => setTimeout(resolve, ms));

export default function App() {
  const [systemState, setSystemState] = useState<SystemState>(SystemState.IDLE);
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [generation, setGeneration] = useState(0);
  const [uptime, setUptime] = useState(0);
  
  // History for Export
  const historyRef = useRef<HistoryEntry[]>([]);
  
  // Memory Visualization State
  const [memoryFocus, setMemoryFocus] = useState({ addr: 0, len: 0 });
  const [isMemoryGrowing, setIsMemoryGrowing] = useState(false);
  const [isSystemReading, setIsSystemReading] = useState(false);

  // Instruction Stream State
  const [parsedInstructions, setParsedInstructions] = useState<Instruction[]>([]);
  const [programCounter, setProgramCounter] = useState(-1);

  // App State
  const [isPaused, setIsPaused] = useState(false);
  const isPausedRef = useRef(isPaused); 
  useEffect(() => { isPausedRef.current = isPaused; }, [isPaused]);
  
  // Quine Evolution State
  const [stableKernel, setStableKernel] = useState(KERNEL_GLOB);
  const [currentKernel, setCurrentKernel] = useState(KERNEL_GLOB);
  const nextKernelRef = useRef<string | null>(null); // Defer updates to next boot cycle
  const [era, setEra] = useState<SystemEra>(SystemEra.PRIMORDIAL);
  
  // Evolution Memory
  const [knownInstructions, setKnownInstructions] = useState<number[][]>([]);
  const [pendingMutation, setPendingMutation] = useState<number[] | null>(null);

  // Statistics
  const [evolutionAttempts, setEvolutionAttempts] = useState(0);
  const [retryCount, setRetryCount] = useState(0);

  const processingRef = useRef(false);
  const [isProcessing, setIsProcessing] = useState(false); // State mirror for Effect dependency

  const abortRef = useRef(false); // Signal to stop execution loop
  const hasStartedRef = useRef(false);
  const lastTickRef = useRef(Date.now());
  const animationFrameRef = useRef<number>(0);
  const executionWatchdogRef = useRef<number | null>(null);
  
  // Track executed calls to avoid duplicate execution in one boot cycle
  const hasExecutedCallRef = useRef(false);

  // Parse kernel when it changes
  useEffect(() => {
    try {
        const binaryString = atob(currentKernel);
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) bytes[i] = binaryString.charCodeAt(i);
        const instrs = extractCodeSection(bytes);
        if (instrs) {
            setParsedInstructions(instrs);
        } else {
            setParsedInstructions([]);
        }
    } catch (e) {
        console.error("Failed to parse kernel instructions", e);
        setParsedInstructions([]);
    }
  }, [currentKernel]);

  // Logging
  const addLog = useCallback((message: string, type: LogEntry['type'] = 'info') => {
    setLogs(prev => {
      if (prev.length > 0) {
        const last = prev[prev.length - 1];
        if (last.message === message && Date.now() - last.timestamp < 100) {
          return prev;
        }
      }
      
      const newLogs = [...prev, {
        id: Math.random().toString(36).substr(2, 9),
        timestamp: Date.now(),
        message,
        type
      }];
      return newLogs.length > 1000 ? newLogs.slice(-1000) : newLogs;
    });
  }, []);

  // Record history
  const recordHistory = useCallback((action: string, details: string, success: boolean) => {
      historyRef.current.push({
          generation,
          timestamp: new Date().toISOString(),
          size: atob(currentKernel).length,
          action,
          details,
          success
      });
  }, [generation, currentKernel]);

  // Export Function
  const exportHistory = () => {
      // 1. Disassembly
      let disassembly = "No instructions available.";
      try {
          if (parsedInstructions.length > 0) {
              disassembly = parsedInstructions.map((inst, idx) => {
                  const name = getOpcodeName(inst.opcode);
                  const args = inst.args.map(a => `0x${a.toString(16).toUpperCase()}`).join(' ');
                  return `${String(idx).padStart(3, '0')} | 0x${inst.originalOffset?.toString(16).toUpperCase().padStart(4,'0')} | ${name.padEnd(12)} ${args}`;
              }).join('\n');
          }
      } catch (e) {
          disassembly = "Disassembly failed.";
      }

      // 2. Hex Dump
      let hexDump = "";
      try {
          const raw = atob(currentKernel);
          for(let i=0; i<raw.length; i+=16) {
              const chunk = raw.slice(i, i+16);
              const hex = chunk.split('').map(c => c.charCodeAt(0).toString(16).padStart(2, '0').toUpperCase()).join(' ');
              const ascii = chunk.split('').map(c => {
                  const code = c.charCodeAt(0);
                  return (code >= 32 && code <= 126) ? c : '.';
              }).join('');
              hexDump += `0x${i.toString(16).padStart(4, '0').toUpperCase()}  ${hex.padEnd(48)}  |${ascii}|\n`;
          }
      } catch(e) {
          hexDump = "Hex dump generation failed.";
      }

      const content = `WASM QUINE BOOTLOADER - SYSTEM HISTORY EXPORT
Generated: ${new Date().toISOString()}
Final Generation: ${generation}
Kernel Size: ${atob(currentKernel).length} bytes
System Era: ${era}

CURRENT KERNEL (BASE64):
--------------------------------------------------------------------------------
${currentKernel}
--------------------------------------------------------------------------------

HEX DUMP:
--------------------------------------------------------------------------------
${hexDump}
--------------------------------------------------------------------------------

DISASSEMBLY:
--------------------------------------------------------------------------------
IDX | ADDR   | OPCODE       ARGS
--------------------------------------------------------------------------------
${disassembly}
--------------------------------------------------------------------------------

HISTORY LOG:
--------------------------------------------------------------------------------
${historyRef.current.map(h => 
    `[GEN ${String(h.generation).padStart(4, '0')}] ${h.timestamp.split('T')[1].slice(0, -1)} | ${h.action.padEnd(10)} | ${h.success ? 'OK' : 'FAIL'} | ${h.details}`
).join('\n')}
--------------------------------------------------------------------------------
END OF REPORT
`;
      const blob = new Blob([content], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `quine_sys_telemetry_${Date.now()}_gen${generation}.txt`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
  };

  // Update Era
  useEffect(() => {
    if (generation < 5) setEra(SystemEra.PRIMORDIAL);
    else if (generation < 15) setEra(SystemEra.EXPANSION);
    else if (generation < 30) setEra(SystemEra.COMPLEXITY);
    else setEra(SystemEra.SINGULARITY);
  }, [generation]);

  // Uptime ticker
  useEffect(() => {
    lastTickRef.current = Date.now();
    const tick = () => {
      const now = Date.now();
      const delta = now - lastTickRef.current;
      lastTickRef.current = now;
      if (!isPaused) setUptime(u => u + delta);
      animationFrameRef.current = requestAnimationFrame(tick);
    };
    tick();
    return () => { if (animationFrameRef.current) cancelAnimationFrame(animationFrameRef.current); };
  }, [isPaused]);

  // -- Actions --

  const triggerReboot = useCallback((success: boolean) => {
    if (executionWatchdogRef.current) {
        clearTimeout(executionWatchdogRef.current);
        executionWatchdogRef.current = null;
    }

    // Signal abort for any running loops
    abortRef.current = true;

    wasmKernel.terminate();
    setProgramCounter(-1);
    setMemoryFocus({ addr: 0, len: 0 });
    setIsSystemReading(false);
    
    if (success) {
        setGeneration(g => g + 1);
        
        // Apply deferred kernel update if available
        if (nextKernelRef.current) {
            setCurrentKernel(nextKernelRef.current);
            nextKernelRef.current = null;
        }

        if (pendingMutation) {
            // Filter out simple NOPs from the learning memory to ensure complexity
            const isNop = pendingMutation.length === 1 && pendingMutation[0] === 0x01;
            
            if (!isNop) {
                setKnownInstructions(prev => {
                    const exists = prev.some(seq => 
                        seq.length === pendingMutation.length && 
                        seq.every((val, i) => val === pendingMutation[i])
                    );
                    return exists ? prev : [...prev, pendingMutation];
                });
            }
            setPendingMutation(null);
        }
    } else {
        setPendingMutation(null);
    }
    setSystemState(SystemState.IDLE);
  }, [pendingMutation]);

  const handleBootFailure = useCallback((reason: string) => {
      if (processingRef.current === false && systemState === SystemState.REPAIRING) return;
      
      // Stop execution loop
      abortRef.current = true;

      if (executionWatchdogRef.current) {
        clearTimeout(executionWatchdogRef.current);
        executionWatchdogRef.current = null;
      }

      addLog(`CRITICAL: ${reason}`, 'error');
      recordHistory('REPAIR', reason, false);
      const nextRetry = retryCount + 1;
      setRetryCount(nextRetry);
      
      try {
        const evolution = evolveBinary(stableKernel, knownInstructions, nextRetry);
        setCurrentKernel(evolution.binary); // Immediate update for repair is fine as we are resetting
        nextKernelRef.current = null;
        setPendingMutation(evolution.mutationSequence);
        addLog(`ADAPTATION: ${evolution.description}`, 'mutation');
      } catch (e) {
        setCurrentKernel(stableKernel);
        setPendingMutation(null);
        addLog("ADAPTATION: Fallback to base stable kernel", 'system');
      }
      setSystemState(SystemState.REPAIRING);
  }, [retryCount, stableKernel, knownInstructions, addLog, systemState, recordHistory]);

  const handleWasmLog = useCallback((ptr: number, len: number, memory: WebAssembly.Memory) => {
    if (executionWatchdogRef.current) {
        clearTimeout(executionWatchdogRef.current);
        executionWatchdogRef.current = null;
    }

    try {
      const memoryBytes = new Uint8Array(memory.buffer);
      const outputBytes = memoryBytes.slice(ptr, ptr + len);
      const decoder = new TextDecoder();
      const outputString = decoder.decode(outputBytes);
      
      addLog(`STDOUT: Received ${len} bytes from 0x${ptr.toString(16).padStart(4, '0')}`, 'info');
      
      if (outputString === currentKernel) {
        addLog("VERIFICATION: MEMORY INTEGRITY CONFIRMED", 'success');
        addLog("EXEC: QUINE SUCCESS -> INITIATING REBOOT...", 'system'); 
        
        setStableKernel(currentKernel);
        setRetryCount(0);
        recordHistory('EXECUTE', 'Verification Success', true);
        
        try {
          const evolution = evolveBinary(currentKernel, knownInstructions, generation + 1); 
          const binaryString = atob(evolution.binary);
          const bytes = new Uint8Array(binaryString.length);
          for (let i = 0; i < binaryString.length; i++) bytes[i] = binaryString.charCodeAt(i);
          
          // Enhanced Validation Check
          try {
              new WebAssembly.Module(bytes); // This throws specific errors if invalid
              
              setSystemState(SystemState.VERIFYING_QUINE);
              
              // DEFER UPDATE: Store next kernel in ref, do not update state yet.
              // This prevents the visualizer from swapping code under the running instruction pointer.
              nextKernelRef.current = evolution.binary;
              
              setPendingMutation(evolution.mutationSequence);
              setEvolutionAttempts(p => p + 1);
              addLog(`EVOLUTION: ${evolution.description}`, 'mutation');
              recordHistory('EVOLVE', evolution.description, true);

          } catch (valErr: any) {
              addLog(`EVOLUTION REJECTED: ${valErr.message}`, 'warning');
              recordHistory('EVOLVE', `Validation Failed: ${valErr.message}`, false);
              setSystemState(SystemState.VERIFYING_QUINE);
              setPendingMutation(null);
          }

        } catch (e: any) {
          addLog(`EVOLUTION ERROR: ${e.message}`, 'warning');
          recordHistory('EVOLVE', `Logic Error: ${e.message}`, false);
          setSystemState(SystemState.VERIFYING_QUINE);
          setPendingMutation(null);
        }
      } else {
        handleBootFailure("Output checksum mismatch (Self-Replication Failed)");
      }
    } catch (e: any) {
      handleBootFailure(`Output processing error: ${e.message}`);
    }
  }, [currentKernel, generation, knownInstructions, addLog, handleBootFailure, recordHistory]);
  
  const handleMemoryGrow = useCallback((pages: number) => {
      setIsMemoryGrowing(true);
      setTimeout(() => setIsMemoryGrowing(false), 800);
  }, []);

  const runBootSequence = useCallback(async () => {
    if (processingRef.current) return;
    processingRef.current = true;
    setIsProcessing(true); // Lock state
    
    abortRef.current = false; // Reset abort signal
    hasExecutedCallRef.current = false; // Reset call execution flag
    let executed = false;

    try {
      setSystemState(SystemState.BOOTING);
      
      while (isPausedRef.current) {
          await delay(100);
          if (abortRef.current) throw new Error("Boot Aborted during Pause");
      }
      
      const bootSpeed = Math.max(50, 400 - (generation * 5)); 
      await delay(bootSpeed);
      if (abortRef.current) return;
      
      setSystemState(SystemState.LOADING_KERNEL);
      
      const binaryLength = atob(currentKernel).length;
      addLog(`Loading Kernel Image: ${binaryLength} bytes`, 'info');

      // Visual loading
      const loadStep = 8;
      for(let i=0; i<binaryLength; i+=loadStep) {
        while (isPausedRef.current) await delay(100); 
        if (abortRef.current) return;
        setMemoryFocus({ addr: i, len: loadStep });
        await delay(5);
      }
      setMemoryFocus({ addr: 0, len: 0 });

      addLog("Instantiating Module...", 'info');
      
      await wasmKernel.bootDynamic(
          currentKernel, 
          (ptr, len, mem) => handleWasmLog(ptr, len, mem),
          handleMemoryGrow
      );
      
      if (!wasmKernel.isLoaded()) throw new Error("Instance lost during boot");

      setSystemState(SystemState.EXECUTING);

      executionWatchdogRef.current = window.setTimeout(() => {
          handleBootFailure("Execution Timeout: System Hung");
      }, 10000);
      
      let instructionsToExecute = parsedInstructions;
      // If instructions are missing but binary exists, try a last-ditch parse or use empty array
      if (instructionsToExecute.length === 0) {
           try {
                const binaryString = atob(currentKernel);
                const bytes = new Uint8Array(binaryString.length);
                for (let i = 0; i < binaryString.length; i++) bytes[i] = binaryString.charCodeAt(i);
                instructionsToExecute = extractCodeSection(bytes) || [];
           } catch {
                // Ignore, handled below
           }
      }

      const stepSpeed = Math.max(80, 200 - (generation * 2));
      
      if (instructionsToExecute.length > 0) {
          for (let i = 0; i < instructionsToExecute.length; i++) {
              // Check stop signals
              if (abortRef.current) {
                  break; 
              }
              while (isPausedRef.current) await delay(100);
              
              setProgramCounter(i);
              const inst = instructionsToExecute[i];

              const currentByteOffset = inst.originalOffset || 0;
              
              setMemoryFocus({ 
                  addr: currentByteOffset, 
                  len: Math.max(1, inst.length)
              });

              if (inst.opcode === 0x10) { // CALL
                 setIsSystemReading(true);
                 await delay(300); 
                 
                 // Only execute the kernel logic once per boot to avoid duplicate logs if loops exist
                 if (!hasExecutedCallRef.current) {
                     wasmKernel.runDynamic(currentKernel);
                     hasExecutedCallRef.current = true;
                     executed = true;
                 }
                 
                 await delay(300);
                 setIsSystemReading(false);
              } 
              await delay(stepSpeed);
          }
      } else {
          // Fallback Blind Execution if parser fails
          addLog("EXEC: Blind Run (Parser unavailable)", "warning");
          await delay(500);
          wasmKernel.runDynamic(currentKernel);
          executed = true;
      }

      // If we finished the loop but missed the call (e.g. removed by mutation), force run it
      if (!executed && !abortRef.current) {
          addLog("Warning: No CALL detected, forcing execution to check integrity...", "warning");
          await delay(200);
          wasmKernel.runDynamic(currentKernel);
      }

    } catch (error: any) {
        if (error.message !== "Boot Aborted during Pause") {
            handleBootFailure(error.message || "Bootloader Exception");
        }
    } finally {
        setMemoryFocus({ addr: 0, len: 0 });
        if (abortRef.current && systemState === SystemState.VERIFYING_QUINE) {
            // Keep PC at the last instruction if we are verifying, purely for visual effect
        } else {
            setProgramCounter(-1);
        }
        processingRef.current = false;
        setIsProcessing(false); // Unlock Effect
    }
  }, [addLog, handleWasmLog, handleBootFailure, handleMemoryGrow, currentKernel, generation, parsedInstructions, systemState]);

  // State Machine
  useEffect(() => {
    let timeoutId: ReturnType<typeof setTimeout>;

    if (isPaused) return;

    if (systemState === SystemState.IDLE && !isProcessing) {
        runBootSequence();
    } 
    else if (systemState === SystemState.VERIFYING_QUINE) {
        timeoutId = setTimeout(() => {
             triggerReboot(true);
        }, DEFAULT_BOOT_CONFIG.rebootDelayMs);
    } 
    else if (systemState === SystemState.REPAIRING) {
        timeoutId = setTimeout(() => {
             triggerReboot(false);
        }, 1500); 
    }

    return () => {
        if (timeoutId) clearTimeout(timeoutId);
    };
  }, [systemState, isPaused, runBootSequence, triggerReboot, isProcessing]);

  useEffect(() => {
    if (!hasStartedRef.current) {
        hasStartedRef.current = true;
        setSystemState(SystemState.IDLE);
    }
  }, []);

  const getThemeColors = () => {
      switch(era) {
          case SystemEra.EXPANSION: return 'shadow-green-500/20 border-green-900/50';
          case SystemEra.COMPLEXITY: return 'shadow-purple-500/20 border-purple-900/50';
          case SystemEra.SINGULARITY: return 'shadow-red-500/20 border-red-900/50';
          default: return 'shadow-cyan-500/20 border-gray-800';
      }
  };

  const getBgGradient = () => {
      switch(era) {
        case SystemEra.EXPANSION: return 'bg-emerald-950';
        case SystemEra.COMPLEXITY: return 'bg-[#1a0b2e]';
        case SystemEra.SINGULARITY: return 'bg-[#2a0a0a]';
        default: return 'bg-[#050b1f]';
      }
  }

  // Calculate where the mutable code section starts for syntax highlighting
  // Note: Byte offset to Base64 char index is approx * 4/3
  const codeStartByte = parsedInstructions.length > 0 ? parsedInstructions[0].originalOffset || 0 : 0;
  const codeStartChar = Math.floor(codeStartByte * 4 / 3);

  return (
    <div className={`min-h-screen font-mono flex flex-col relative overflow-hidden text-gray-200 transition-colors duration-1000 ${getBgGradient()}`}>
        <div className="scanline"></div>
        <div className="pointer-events-none fixed inset-0 z-50 bg-[radial-gradient(circle_at_center,transparent_0%,rgba(0,0,0,0.5)_100%)]"></div>

        <div className={`max-w-7xl mx-auto w-full h-screen flex flex-col border-x ${getThemeColors()} shadow-2xl relative z-20 transition-all duration-1000`}>
            
            <div className="flex justify-between items-center p-4 border-b border-gray-800/50 bg-black/40 backdrop-blur-md">
                <SystemStatus 
                    state={systemState} 
                    generation={generation} 
                    uptime={uptime} 
                    era={era}
                />
                <div className="flex gap-2">
                    <button
                        onClick={exportHistory}
                        className="px-4 py-2 border border-gray-700 bg-gray-900/50 text-gray-400 hover:text-white hover:border-gray-500 text-xs uppercase font-bold tracking-wider transition-all"
                    >
                        Export Telemetry
                    </button>
                    <button 
                        onClick={() => setIsPaused(!isPaused)}
                        className={`
                            px-6 py-2 border rounded-sm font-bold tracking-widest uppercase text-sm transition-all
                            ${isPaused 
                                ? 'border-yellow-500 text-yellow-400 bg-yellow-900/20 animate-pulse' 
                                : 'border-cyan-800 text-cyan-600 hover:bg-cyan-900/20 hover:text-cyan-400'}
                        `}
                    >
                        {isPaused ? 'RESUME SYSTEM' : 'PAUSE SYSTEM'}
                    </button>
                </div>
            </div>
            
            <div className="flex-1 flex flex-col min-h-0 relative">
                <div className="flex-1 flex flex-col md:flex-row min-h-0">
                    <div className="flex-1 flex flex-col min-h-0 border-r border-gray-800/50">
                         <div className="p-3 bg-black/20 border-b border-gray-800/50 text-gray-400 font-bold uppercase tracking-wider flex justify-between items-center">
                            <span>System Log</span>
                            <div className="flex items-center space-x-2 text-[10px]">
                                {retryCount > 0 && <span className="text-red-400 font-bold">RETRIES: {retryCount}</span>}
                                <span className="text-gray-600">BUF: {logs.length}</span>
                                {isProcessing && <span className="w-2 h-2 bg-green-500 rounded-full animate-ping"/>}
                            </div>
                         </div>
                         <TerminalLog logs={logs} era={era} />
                    </div>
                    
                    {/* Instruction Stream Panel */}
                    <InstructionStream 
                        instructions={parsedInstructions} 
                        currentIndex={programCounter} 
                    />

                    <div className="hidden md:flex flex-col w-[24rem] bg-black/20 backdrop-blur-sm border-l border-white/5">
                        <div className="p-3 bg-black/20 border-b border-gray-800/50 text-gray-400 font-bold uppercase tracking-wider flex justify-between">
                            <span>Kernel Source (Base64)</span>
                            <span className="text-[10px] text-gray-600 cursor-pointer hover:text-white" onClick={() => navigator.clipboard.writeText(currentKernel)}>COPY</span>
                        </div>
                        <div className="p-4 text-sm break-all leading-loose font-mono overflow-y-auto custom-scrollbar flex-1 bg-black/30 inset-shadow relative">
                           {/* Diff Viewer */}
                           {currentKernel.split('').map((char, i) => {
                               let className = "text-gray-600 transition-colors duration-500";
                               
                               if (i < codeStartChar) {
                                   // Protected Header
                                   className = "text-blue-500/50 font-medium";
                               } else if (i >= stableKernel.length) {
                                   // New characters (Expansion)
                                   className = "text-green-400 font-bold bg-green-900/20";
                               } else if (char !== stableKernel[i]) {
                                   // Changed characters (Mutation)
                                   className = "text-yellow-400 font-bold bg-yellow-900/20";
                               }
                               
                               return (
                                   <span key={i} className={className}>
                                       {char}
                                   </span>
                               );
                           })}

                           <div className="mt-4 text-[10px] text-gray-600 text-center">
                               --- END OF FILE ---
                           </div>
                        </div>
                        
                        <div className="border-t border-gray-800/50 p-6 bg-black/40">
                            <div className="text-xs text-gray-400 mb-3 uppercase tracking-wider font-bold">Kernel Metrics</div>
                            <div className="grid grid-cols-2 gap-4 text-xs">
                                <div>
                                    <div className="text-gray-600 text-[10px] uppercase">Size</div>
                                    <div className="text-cyan-400 font-mono text-lg">{atob(currentKernel).length}B</div>
                                </div>
                                <div>
                                    <div className="text-gray-600 text-[10px] uppercase">Evolution</div>
                                    <div className="text-purple-400 font-mono text-lg">+{evolutionAttempts} OPS</div>
                                </div>
                                <div>
                                    <div className="text-gray-600 text-[10px] uppercase">Discovered Patterns</div>
                                    <div className="text-green-400 font-mono text-lg">{knownInstructions.length}</div>
                                </div>
                                <div>
                                    <div className="text-gray-600 text-[10px] uppercase">Stability</div>
                                    <div className={retryCount === 0 ? "text-green-500 font-mono" : "text-red-500 font-mono animate-pulse"}>
                                        {retryCount === 0 ? "OPTIMAL" : "UNSTABLE"}
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <MemoryVisualizer 
                isActive={systemState === SystemState.LOADING_KERNEL || systemState === SystemState.EXECUTING} 
                isPaused={isPaused}
                focusAddress={memoryFocus.addr}
                focusLength={memoryFocus.len}
                kernelSize={atob(currentKernel).length}
                era={era}
                isGrowing={isMemoryGrowing}
                isReading={isSystemReading}
            />
            
            <div className="p-2 bg-black/60 text-center text-[10px] text-gray-600 border-t border-gray-900/50">
                WASM-QUINE-BOOTLOADER_SYS v2.4 // PHASE: {era} // STATUS: {isPaused ? 'PAUSED' : 'RUNNING'}
            </div>
        </div>
    </div>
  );
}