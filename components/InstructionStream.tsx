import React, { useEffect, useRef } from 'react';
import { Instruction, getOpcodeName } from '../utils/wasmParser';

interface InstructionStreamProps {
  instructions: Instruction[];
  currentIndex: number;
}

export const InstructionStream: React.FC<InstructionStreamProps> = ({ instructions, currentIndex }) => {
  const activeRef = useRef<HTMLDivElement>(null);
  
  useEffect(() => {
    if (activeRef.current) {
        activeRef.current.scrollIntoView({
            behavior: 'smooth',
            block: 'center',
            inline: 'nearest'
        });
    }
  }, [currentIndex]);

  return (
    <div className="flex flex-col w-72 bg-black/20 backdrop-blur-sm border-r border-gray-800/50 border-l border-white/5 h-full relative group">
        {/* Header */}
        <div className="p-3 bg-black/40 border-b border-gray-800/50 text-gray-400 font-bold uppercase tracking-wider text-[10px] flex justify-between z-20 shadow-sm">
            <span>Instruction Stack</span>
            <span className={currentIndex >= 0 ? "text-green-400 animate-pulse" : "text-gray-600"}>
                IP: {currentIndex >= 0 ? String(currentIndex).padStart(3, '0') : "WAIT"}
            </span>
        </div>

        {/* Stream Container */}
        <div className="flex-1 relative overflow-hidden bg-black/10">
             <div className="absolute inset-0 overflow-y-auto custom-scrollbar scroll-smooth">
                <div className="py-[60%] px-1">
                    {instructions.map((inst, idx) => {
                        const isActive = idx === currentIndex;
                        const name = getOpcodeName(inst.opcode);
                        const isNop = inst.opcode === 0x01; // NOP
                        
                        return (
                            <div 
                                key={idx} 
                                ref={isActive ? activeRef : null}
                                className={`
                                    relative z-10 px-4 py-2 flex justify-between items-center my-0.5 transition-all duration-100 font-mono text-base border-l-2
                                    ${isActive 
                                        ? 'text-white font-bold bg-white/10 border-white pl-4' 
                                        : 'text-gray-500 opacity-70 border-transparent hover:opacity-100'}
                                    ${isNop && !isActive ? 'opacity-30' : ''}
                                `}
                            >
                                <span className={`uppercase tracking-tight ${isNop ? 'italic' : ''}`}>{name}</span>
                                {inst.args.length > 0 && (
                                    <span className={`text-[11px] font-mono px-1.5 py-0.5 rounded ml-2 border ${isActive ? 'bg-black/40 border-white/20 text-white' : 'border-white/5 text-gray-600'}`}>
                                        {inst.args.map(a => '0x' + a.toString(16).toUpperCase()).join(' ')}
                                    </span>
                                )}
                            </div>
                        );
                    })}
                    {instructions.length === 0 && (
                        <div className="text-center text-gray-700 italic py-8 text-sm">
                            Waiting for Kernel...
                        </div>
                    )}
                </div>
            </div>
        </div>
        
        <style>{`
            .custom-scrollbar::-webkit-scrollbar {
                width: 6px;
                background-color: rgba(0,0,0,0.2);
            }
            .custom-scrollbar::-webkit-scrollbar-thumb {
                background-color: rgba(255,255,255,0.1);
                border-radius: 3px;
            }
            .custom-scrollbar::-webkit-scrollbar-thumb:hover {
                background-color: rgba(255,255,255,0.3);
            }
        `}</style>
    </div>
  );
};
