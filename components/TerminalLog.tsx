import React, { useEffect, useRef } from 'react';
import { LogEntry, SystemEra } from '../types';

interface TerminalLogProps {
  logs: LogEntry[];
  era?: SystemEra;
}

export const TerminalLog: React.FC<TerminalLogProps> = ({ logs, era }) => {
  const bottomRef = useRef<HTMLDivElement>(null);
  
  // Strict auto-scroll
  useEffect(() => {
    if (bottomRef.current) {
        bottomRef.current.scrollIntoView({ behavior: 'smooth' });
    }
  }, [logs]);

  return (
    <div className="flex-1 overflow-y-auto font-mono text-sm p-6 space-y-1.5 bg-transparent custom-scrollbar relative">
      {logs.length === 0 && <div className="text-gray-700 italic">Waiting for boot sequence...</div>}
      
      {logs.map((log) => (
        <div key={log.id} className="flex items-start leading-relaxed hover:bg-white/5 p-0.5 rounded px-2 -mx-2 transition-colors">
          <span className="text-gray-600 mr-3 shrink-0 text-xs mt-0.5 font-mono opacity-70">
            {new Date(log.timestamp).toISOString().split('T')[1].slice(0, -1)}
          </span>
          <span
            className={`
              break-all font-medium text-xs md:text-sm
              ${log.type === 'info' ? 'text-gray-400' : ''}
              ${log.type === 'success' ? 'text-green-400 font-bold' : ''}
              ${log.type === 'warning' ? 'text-yellow-400' : ''}
              ${log.type === 'error' ? 'text-red-500 font-bold bg-red-950/30 px-1 rounded' : ''}
              ${log.type === 'system' ? 'text-cyan-400' : ''}
              ${log.type === 'mutation' ? 'text-purple-400 italic opacity-90' : ''}
            `}
          >
            {log.type === 'system' && <span className="mr-2 text-cyan-600">➜</span>}
            {log.message}
          </span>
        </div>
      ))}
      <div ref={bottomRef} className="h-1" />
    </div>
  );
};
