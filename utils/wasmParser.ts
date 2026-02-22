
export interface Instruction {
  opcode: number;
  args: number[];
  length: number;
  originalOffset?: number;
}

export const getOpcodeName = (byte: number): string => {
    switch(byte) {
        case 0x00: return "unreachable";
        case 0x01: return "nop";
        case 0x0B: return "end";
        case 0x1A: return "drop";
        case 0x20: return "local.get";
        case 0x21: return "local.set";
        case 0x22: return "local.tee";
        case 0x41: return "i32.const";
        case 0x10: return "call";
        default: return `0x${byte.toString(16).toUpperCase()}`;
    }
};

export const decodeLEB128 = (bytes: Uint8Array, offset: number): { value: number, length: number } => {
  let result = 0;
  let shift = 0;
  let count = 0;
  while (true) {
    if (offset + count >= bytes.length) break;
    const byte = bytes[offset + count];
    result |= (byte & 0x7f) << shift;
    shift += 7;
    count++;
    if ((byte & 0x80) === 0) break;
  }
  if (count === 0 && offset < bytes.length) return { value: bytes[offset], length: 1 };
  if (count === 0) return { value: 0, length: 0 };
  
  return { value: result, length: count };
};

export const encodeLEB128 = (value: number): number[] => {
  const bytes: number[] = [];
  let val = value;
  if (val === 0) return [0];
  while (true) {
    let byte = val & 0x7f;
    val >>>= 7;
    if (val === 0) {
      bytes.push(byte);
      break;
    } else {
      bytes.push(byte | 0x80);
    }
  }
  return bytes;
};

export const parseInstructions = (bytes: Uint8Array): Instruction[] => {
  const instructions: Instruction[] = [];
  let ptr = 0;
  
  while (ptr < bytes.length) {
    const opcode = bytes[ptr];
    let args: number[] = [];
    let len = 1;

    if (opcode === 0x41) { // i32.const
        const leb = decodeLEB128(bytes, ptr + 1);
        len += leb.length;
        args = Array.from(bytes.slice(ptr + 1, ptr + len));
    } else if (opcode === 0x20 || opcode === 0x21 || opcode === 0x22 || opcode === 0x10) {
        const leb = decodeLEB128(bytes, ptr + 1);
        len += leb.length;
        args = Array.from(bytes.slice(ptr + 1, ptr + len));
    }

    if (ptr + len > bytes.length) {
        len = bytes.length - ptr;
        args = Array.from(bytes.slice(ptr + 1, ptr + len));
    }

    instructions.push({ opcode, args, length: len, originalOffset: ptr });
    ptr += len;
  }
  return instructions;
};

export const extractCodeSection = (bytes: Uint8Array) => {
  // Helper to find code section and instructions for UI
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
  
  if (codeSectionStart === -1) return null;

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
  
  return parseInstructions(bytes.slice(instructionStart, endOpIndex));
};
