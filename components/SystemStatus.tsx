import React from 'react';
import { SystemState, SystemEra } from '../types';

interface SystemStatusProps {
  state: SystemState;
  generation: number;
  uptime: number;
  era: SystemEra;
}

export const SystemStatus: React.FC<SystemStatusProps> = ({ state, generation, uptime, era }) => {
  const getStateColor = (s: SystemState) => {
    switch (s) {
      case SystemState.IDLE: return 'text-gray-500';
      case SystemState.BOOTING: return 'text-yellow-400';
      case SystemState.LOADING_KERNEL: return 'text-blue-400';
      case SystemState.EXECUTING: return 'text-green-400';
      case SystemState.VERIFYING_QUINE: return 'text-purple-400';
      case SystemState.SYSTEM_HALT: return 'text-red-500';
      case SystemState.REPAIRING: return 'text-orange-500 animate-pulse';
      default: return 'text-gray-500';
    }
  };

  const getEraColor = (e: SystemEra) => {
      switch(e) {
          case SystemEra.PRIMORDIAL: return 'text-gray-500';
          case SystemEra.EXPANSION: return 'text-emerald-400';
          case SystemEra.COMPLEXITY: return 'text-purple-400';
          case SystemEra.SINGULARITY: return 'text-red-500 drop-shadow-[0_0_5px_rgba(239,68,68,0.8)]';
      }
  }

  return (
    <div className="flex flex-col gap-4 md:gap-0 md:flex-row items-start md:items-center justify-between w-full">
      <div className="flex items-center space-x-6">
        <div className="relative w-4 h-4">
            <div className={`w-full h-full rounded-full ${state === SystemState.EXECUTING ? 'bg-green-500 animate-ping' : 'bg-gray-600'}`}></div>
            <div className={`absolute top-0 left-0 w-full h-full rounded-full ${state === SystemState.EXECUTING ? 'bg-green-500' : 'bg-gray-600'}`}></div>
        </div>
        <div>
            <h1 className="text-2xl font-black tracking-tighter text-gray-100 flex items-center gap-2">
              QUINE<span className="text-cyan-500">OS</span> 
              <span className="text-xs font-normal text-gray-600 bg-gray-900 px-2 py-0.5 rounded border border-gray-800">v2.0.4</span>
            </h1>
        </div>
      </div>
      
      <div className="flex flex-wrap gap-8 text-xs font-mono">
        <div>
            <span className="text-gray-500 block uppercase text-[10px] tracking-widest mb-1">System Era</span>
            <span className={`text-base font-bold ${getEraColor(era)}`}>{era}</span>
        </div>
        <div>
            <span className="text-gray-500 block uppercase text-[10px] tracking-widest mb-1">Generation</span>
            <span className="text-white text-base">{String(generation).padStart(4, '0')}</span>
        </div>
        <div>
            <span className="text-gray-500 block uppercase text-[10px] tracking-widest mb-1">State</span>
            <span className={`${getStateColor(state)} text-base font-bold`}>{state}</span>
        </div>
         <div className="hidden lg:block">
            <span className="text-gray-500 block uppercase text-[10px] tracking-widest mb-1">Runtime</span>
            <span className="text-gray-300 text-base">{(uptime / 1000).toFixed(1)}s</span>
        </div>
      </div>
    </div>
  );
};