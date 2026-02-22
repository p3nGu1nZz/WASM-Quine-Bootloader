// Evolution Strategies for the WASM Binary
import { 
    getOpcodeName, 
    decodeLEB128, 
    encodeLEB128, 
    parseInstructions, 
    Instruction 
} from './wasmParser';

export enum EvolutionAction {
  MODIFY = 0,
  INSERT = 1,
  ADD = 2,
  DELETE = 3
}

export interface EvolutionResult {
  binary: string;
  mutationSequence: number[] | null;
  actionUsed: EvolutionAction;
  description: string;
}

// 1. DYNAMIC GENOME GENERATION

const generateRandomConstDrop = (): number[] => {
    // Generate i32.const <random_byte>, drop
    // Opcode 0x41, Value (0-127), Opcode 0x1A
    return [0x41, Math.floor(Math.random() * 128), 0x1A];
};

const generateSafeMath = (): number[] => {
    // Generate: i32.const A, i32.const B, <op>, drop
    // Ops: add(0x6A), sub(0x6B), mul(0x6C), and(0x71), or(0x72), xor(0x73)
    const ops = [0x6A, 0x6B, 0x71, 0x72, 0x73]; 
    const op = ops[Math.floor(Math.random() * ops.length)];
    
    // Use slightly larger numbers to make output logs interesting
    return [
        0x41, Math.floor(Math.random() * 128), 
        0x41, Math.floor(Math.random() * 128), 
        op, 
        0x1A
    ];
};

const generateLocalTee = (): number[] => {
    // Complex pattern: i32.const X, local.tee 0, drop
    // This exercises variable storage without corrupting the stack state
    // [0x41, val, 0x22, local_idx, 0x1A]
    return [0x41, Math.floor(Math.random() * 255), 0x22, 0x00, 0x1A];
};

const generateIfTrue = (): number[] => {
    // Control Flow: i32.const 1, if, i32.const X, drop, end
    // [0x41, 1, 0x04, 0x40, 0x41, val, 0x1A, 0x0B]
    return [
        0x41, 0x01,       // const 1 (true)
        0x04, 0x40,       // if (void)
        0x41, Math.floor(Math.random() * 64), // const X
        0x1A,             // drop
        0x0B              // end
    ];
};

const BASE_SAFE_GENOMES = [
    [0x20, 0x00, 0x1A], // local.get 0, drop
    [0x20, 0x01, 0x1A], // local.get 1, drop
    [0x41, 0x00, 0x1A], // i32.const 0, drop
    [0x41, 0x01, 0x1A], // i32.const 1, drop
];

// Helper to describe a sequence of bytes as human-readable instructions
const describeSequence = (bytes: number[]): string => {
    try {
        const instrs = parseInstructions(new Uint8Array(bytes));
        return instrs.map(i => {
            const name = getOpcodeName(i.opcode);
            if (i.opcode === 0x41 && i.args.length > 0) return `${name} ${i.args[0]}`;
            if (i.opcode === 0x20 && i.args.length > 0) return `${name} ${i.args[0]}`;
            if (i.opcode === 0x22 && i.args.length > 0) return `${name} ${i.args[0]}`;
            return name;
        }).join(', ');
    } catch {
        return "unknown sequence";
    }
};

export const evolveBinary = (
  currentBase64: string, 
  knownInstructions: number[][],
  attemptSeed: number
): EvolutionResult => {
  const binaryString = atob(currentBase64);
  const bytes = new Uint8Array(binaryString.length);
  for (let i = 0; i < binaryString.length; i++) bytes[i] = binaryString.charCodeAt(i);

  // 1. Locate Code Section
  let ptr = 8;
  let codeSectionStart = -1;
  let codeSectionContentStart = -1;
  
  while (ptr < bytes.length) {
    const id = bytes[ptr];
    const sizeData = decodeLEB128(bytes, ptr + 1);
    if (id === 10) {
        codeSectionStart = ptr;
        codeSectionContentStart = ptr + 1 + sizeData.length;
        break;
    }
    ptr = ptr + 1 + sizeData.length + sizeData.value;
  }
  if (codeSectionStart === -1) throw new Error("Code section missing");

  // 2. Locate Function Body
  const numFuncsData = decodeLEB128(bytes, codeSectionContentStart);
  const funcBodySizeOffset = codeSectionContentStart + numFuncsData.length;
  const funcBodySizeData = decodeLEB128(bytes, funcBodySizeOffset);
  const funcContentStart = funcBodySizeOffset + funcBodySizeData.length;
  
  const localCountData = decodeLEB128(bytes, funcContentStart);
  let instructionPtr = funcContentStart + localCountData.length;
  
  for (let i = 0; i < localCountData.value; i++) {
    const countData = decodeLEB128(bytes, instructionPtr);
    instructionPtr += countData.length + 1;
  }
  
  const instructionStart = instructionPtr;
  const funcEnd = funcContentStart + funcBodySizeData.value; 
  const endOpIndex = funcEnd - 1;

  // 3. Parse Instructions
  const instructionBytes = bytes.slice(instructionStart, endOpIndex);
  const parsedInstructions = parseInstructions(instructionBytes);

  // 4. Evolution Logic
  const action = attemptSeed % 4; 
  let mutationSequence: number[] | null = null;
  let newInstructionsBytes: number[] = [];
  let description = "";

  const flatten = (instrs: Instruction[]) => {
      const flat: number[] = [];
      instrs.forEach(i => {
          flat.push(i.opcode);
          i.args.forEach(a => flat.push(a));
      });
      return flat;
  };

  // GENOME SELECTION STRATEGY
  const getGenome = () => {
      const rand = Math.random();
      
      // PRIORITY 1: Known Instructions (Memory)
      // Increased probability from 0.5 to 0.7 to prioritize "learning"
      if (knownInstructions.length > 2 && rand < 0.7) {
          return knownInstructions[Math.floor(Math.random() * knownInstructions.length)];
      }

      // PRIORITY 2: Intelligent Mutation
      const subRand = Math.random();
      if (subRand < 0.30) return generateRandomConstDrop(); // 30% Simple
      if (subRand < 0.60) return generateSafeMath();        // 30% Arithmetic
      if (subRand < 0.80) return generateLocalTee();        // 20% Variable Usage
      if (subRand < 0.95) return generateIfTrue();          // 15% Control Flow
      
      // Fallback
      return BASE_SAFE_GENOMES[Math.floor(Math.random() * BASE_SAFE_GENOMES.length)];
  };

  switch (action) {
      case EvolutionAction.MODIFY: {
          // Modifying instructions in place is highly dangerous for WASM validation (changing opcode lengths).
          // We treat modify as "Insert" for safety in this version.
          const seq = getGenome();
          mutationSequence = seq;
          const idx = Math.floor(Math.random() * (parsedInstructions.length + 1));
          const before = flatten(parsedInstructions.slice(0, idx));
          const after = flatten(parsedInstructions.slice(idx));
          newInstructionsBytes = [...before, ...seq, ...after];
          description = `Modified: Inserted [${describeSequence(seq)}] at ${idx}`;
          break;
      }
      case EvolutionAction.DELETE: {
          if (parsedInstructions.length > 0) {
              let targetIdx = -1;
              let deleteCount = 0;
              
              // Priority 1: Delete NOPs (Clean up)
              const nopIndices = parsedInstructions
                  .map((inst, idx) => inst.opcode === 0x01 ? idx : -1)
                  .filter(idx => idx !== -1);

              if (nopIndices.length > 0) {
                  targetIdx = nopIndices[Math.floor(Math.random() * nopIndices.length)];
                  deleteCount = 1;
                  description = `Deleted NOP at index ${targetIdx}`;
              } 
              else {
                  // Priority 2: Delete "Safe Math" sequences [Const, Const, Op, Drop] (Length 4)
                  const mathIndices = parsedInstructions.map((inst, idx) => {
                       if (idx >= parsedInstructions.length - 3) return -1;
                       const i3 = parsedInstructions[idx+2];
                       const i4 = parsedInstructions[idx+3];
                       if (parsedInstructions[idx].opcode === 0x41 && parsedInstructions[idx+1].opcode === 0x41) {
                           if ([0x6A, 0x6B, 0x6C, 0x71, 0x72, 0x73].includes(i3.opcode) && i4.opcode === 0x1A) return idx;
                       }
                       return -1;
                  }).filter(idx => idx !== -1);

                  if (mathIndices.length > 0 && Math.random() < 0.6) {
                      targetIdx = mathIndices[Math.floor(Math.random() * mathIndices.length)];
                      deleteCount = 4;
                      const opName = getOpcodeName(parsedInstructions[targetIdx+2].opcode);
                      description = `Pruned math sequence [${opName}]`;
                  }
                  else {
                      // Priority 3: Delete "If True" Blocks [Const 1, If, Const, Drop, End] (Length 5)
                      const ifIndices = parsedInstructions.map((inst, idx) => {
                          if (idx >= parsedInstructions.length - 4) return -1;
                          if (inst.opcode === 0x41 && inst.args[0] === 1 &&     // Const 1
                              parsedInstructions[idx+1].opcode === 0x04 &&      // If
                              parsedInstructions[idx+4].opcode === 0x0B) {      // End
                              return idx;
                          }
                          return -1;
                      }).filter(idx => idx !== -1);

                      if (ifIndices.length > 0 && Math.random() < 0.5) {
                          targetIdx = ifIndices[Math.floor(Math.random() * ifIndices.length)];
                          deleteCount = 5;
                          description = `Pruned control flow block`;
                      }
                      else {
                          // Priority 4: Delete Producer-Consumer pairs (e.g., const -> drop, get -> drop, tee -> drop)
                          const pairIndices = parsedInstructions
                              .map((inst, idx) => {
                                  if (idx >= parsedInstructions.length - 1) return -1;
                                  const next = parsedInstructions[idx+1];
                                  if ((inst.opcode === 0x41 || inst.opcode === 0x20 || inst.opcode === 0x22) && next.opcode === 0x1A) {
                                      return idx;
                                  }
                                  return -1;
                              })
                              .filter(idx => idx !== -1);
                          
                          if (pairIndices.length > 0) {
                              targetIdx = pairIndices[Math.floor(Math.random() * pairIndices.length)];
                              deleteCount = 2;
                              const name = getOpcodeName(parsedInstructions[targetIdx].opcode);
                              description = `Deleted balanced pair [${name}, drop]`;
                          } 
                          // CRITICAL FIX: Removed the random single-instruction deletion fallback.
                          // Deleting random opcodes in a stack machine causes instability (stack underflow/overflow).
                          // If no safe pattern is found, we skip deletion.
                      }
                  }
              }

              if (targetIdx !== -1) {
                  parsedInstructions.splice(targetIdx, deleteCount);
              } else {
                  description = "No safe deletion targets found (Skipped)";
              }
              mutationSequence = null;
          } else {
              description = "Instruction set empty";
          }
          newInstructionsBytes = flatten(parsedInstructions);
          break;
      }
      case EvolutionAction.INSERT: {
          const seq = getGenome();
          mutationSequence = seq;
          
          let idx: number;
          if (Math.random() > 0.3) {
             const min = Math.floor(parsedInstructions.length / 2);
             idx = min + Math.floor(Math.random() * (parsedInstructions.length - min + 1));
          } else {
             idx = Math.floor(Math.random() * (parsedInstructions.length + 1));
          }

          const before = flatten(parsedInstructions.slice(0, idx));
          const after = flatten(parsedInstructions.slice(idx));
          newInstructionsBytes = [...before, ...seq, ...after];
          
          description = `Inserted [${describeSequence(seq)}] at offset ${idx}`;
          break;
      }
      case EvolutionAction.ADD: {
          const seq = getGenome();
          mutationSequence = seq;
          const current = flatten(parsedInstructions);
          newInstructionsBytes = [...current, ...seq];
          
          description = `Appended [${describeSequence(seq)}]`;
          break;
      }
  }
  
  if (newInstructionsBytes.length === 0 && parsedInstructions.length > 0) {
      newInstructionsBytes = flatten(parsedInstructions);
  }

  // 5. Reconstruct Binary
  const preInstructions = bytes.slice(funcContentStart, instructionStart);
  const instructionsArray = new Uint8Array(newInstructionsBytes);
  const postInstructions = bytes.slice(endOpIndex);

  const newFuncBodyLen = preInstructions.length + instructionsArray.length + postInstructions.length; 

  if (newFuncBodyLen > 32768) throw new Error("Evolution Limit: 32KB");

  const newFuncBodySizeEncoded = encodeLEB128(newFuncBodyLen);
  
  const preFuncSize = bytes.slice(codeSectionContentStart, funcBodySizeOffset); 
  const newSectionContentLen = preFuncSize.length + newFuncBodySizeEncoded.length + newFuncBodyLen;
  const newSectionSizeEncoded = encodeLEB128(newSectionContentLen);
  
  const preCode = bytes.slice(0, codeSectionStart + 1);

  const finalSize = 
      preCode.length + 
      newSectionSizeEncoded.length + 
      preFuncSize.length + 
      newFuncBodySizeEncoded.length + 
      preInstructions.length + 
      instructionsArray.length + 
      postInstructions.length;

  const newBytes = new Uint8Array(finalSize);
  let offset = 0;

  newBytes.set(preCode, offset); offset += preCode.length;
  newBytes.set(new Uint8Array(newSectionSizeEncoded), offset); offset += newSectionSizeEncoded.length;
  newBytes.set(preFuncSize, offset); offset += preFuncSize.length;
  newBytes.set(new Uint8Array(newFuncBodySizeEncoded), offset); offset += newFuncBodySizeEncoded.length;
  newBytes.set(preInstructions, offset); offset += preInstructions.length;
  newBytes.set(instructionsArray, offset); offset += instructionsArray.length;
  newBytes.set(postInstructions, offset);

  let binary = '';
  for (let i = 0; i < newBytes.length; i++) binary += String.fromCharCode(newBytes[i]);
  
  return {
      binary: btoa(binary),
      mutationSequence,
      actionUsed: action,
      description
  };
};