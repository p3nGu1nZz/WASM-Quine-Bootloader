import React, { useRef, useEffect, useState } from 'react';
import { SystemEra } from '../types';

interface MemoryVisualizerProps {
  isActive: boolean;
  isPaused: boolean;
  focusAddress: number;
  focusLength: number;
  kernelSize: number;
  era: SystemEra;
  isGrowing?: boolean;
  isReading?: boolean;
}

export const MemoryVisualizer: React.FC<MemoryVisualizerProps> = (props) => {
  const {
    kernelSize, 
    isPaused
  } = props;

  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const [dimensions, setDimensions] = useState({ width: 0, height: 0 });

  // Use a ref to access latest props inside the animation loop without restarting it
  // This ensures perfect synchronization without React render-cycle jitter
  const propsRef = useRef(props);
  useEffect(() => { propsRef.current = props; }, [props]);

  // Heatmap state to track "fading" blocks
  // Map index -> opacity (0.0 to 1.0)
  const heatMap = useRef<Float32Array>(new Float32Array(0));

  // Resize Observer
  useEffect(() => {
    if (!containerRef.current) return;
    const observer = new ResizeObserver((entries) => {
      const { width, height } = entries[0].contentRect;
      setDimensions({ width, height });
    });
    observer.observe(containerRef.current);
    return () => observer.disconnect();
  }, []);

  // Theme Config
  const getTheme = (currentEra: SystemEra) => {
    switch(currentEra) {
        case SystemEra.EXPANSION: return { base: '#064e3b', active: '#34d399', read: '#10b981', border: 'rgba(52, 211, 153, 0.2)' };
        case SystemEra.COMPLEXITY: return { base: '#3b0764', active: '#c084fc', read: '#d8b4fe', border: 'rgba(192, 132, 252, 0.2)' };
        case SystemEra.SINGULARITY: return { base: '#450a0a', active: '#ef4444', read: '#fca5a5', border: 'rgba(239, 68, 68, 0.2)' };
        default: return { base: '#1e293b', active: '#22d3ee', read: '#7dd3fc', border: 'rgba(34, 211, 238, 0.2)' };
    }
  };

  // Main Draw Loop
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || dimensions.width === 0) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Configuration
    const BLOCK_SIZE = kernelSize < 256 ? 8 : kernelSize < 1024 ? 5 : 3;
    const GAP = 1;
    const COLS = Math.floor(dimensions.width / (BLOCK_SIZE + GAP));
    const bytesPerBlock = kernelSize < 256 ? 1 : kernelSize < 1024 ? 4 : 16;
    const TOTAL_BLOCKS = Math.ceil(kernelSize / bytesPerBlock); 
    
    // Resize buffer if needed
    if (heatMap.current.length !== TOTAL_BLOCKS) {
        const newHeatMap = new Float32Array(TOTAL_BLOCKS);
        newHeatMap.set(heatMap.current.slice(0, Math.min(heatMap.current.length, TOTAL_BLOCKS)));
        heatMap.current = newHeatMap;
    }

    // Canvas scaling for high DPI
    const dpr = window.devicePixelRatio || 1;
    const computedHeight = Math.ceil(TOTAL_BLOCKS / COLS) * (BLOCK_SIZE + GAP) + 20;
    
    canvas.width = dimensions.width * dpr;
    canvas.height = Math.max(200, computedHeight) * dpr;
    ctx.scale(dpr, dpr);
    canvas.style.width = `${dimensions.width}px`;
    canvas.style.height = `${canvas.height / dpr}px`;

    let animationId: number;

    const render = () => {
       // Access latest props via ref to avoid closure staleness
       const currentProps = propsRef.current;
       
       // Safety check, though loop should be managed by useEffect dependency
       if (currentProps.isPaused) return;

       const theme = getTheme(currentProps.era);
       ctx.clearRect(0, 0, dimensions.width, canvas.height / dpr);
       
       // Differentiate Read vs Write colors
       const activeColor = currentProps.isReading ? theme.read : theme.active;

       for (let i = 0; i < TOTAL_BLOCKS; i++) {
           const col = i % COLS;
           const row = Math.floor(i / COLS);
           const x = col * (BLOCK_SIZE + GAP);
           const y = row * (BLOCK_SIZE + GAP);
           
           // Calculate Focus State
           const blockStart = i * bytesPerBlock;
           const blockEnd = blockStart + bytesPerBlock;
           const isFocused = currentProps.isActive && 
                            (blockStart < (currentProps.focusAddress + currentProps.focusLength) && 
                             blockEnd > currentProps.focusAddress);
           
           // Update Heatmap
           if (isFocused) {
               heatMap.current[i] = 1.0;
           } else if (currentProps.isReading) {
               // Random flicker scan effect
               if (Math.random() > 0.98) heatMap.current[i] = Math.min(1.0, heatMap.current[i] + 0.5);
           } 
           
           // Decay heat
           // 0.85 = fast decay for "brief" animation (approx 250ms tail at 60fps)
           heatMap.current[i] *= 0.85;
           if (heatMap.current[i] < 0.005) heatMap.current[i] = 0;

           const heat = heatMap.current[i];

           // Draw Base Block
           ctx.fillStyle = theme.base;
           ctx.globalAlpha = 0.3;
           ctx.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE);
           
           // Draw Active State
           if (heat > 0.01) {
               ctx.globalAlpha = heat; 

               // Pulse Effect: Scale based on heat
               const scale = 1 + (heat * 0.6); 
               const size = BLOCK_SIZE * scale;
               const offset = (size - BLOCK_SIZE) / 2;
               
               if (heat > 0.5) {
                   // Core hot flash
                   ctx.fillStyle = '#ffffff'; 
                   ctx.shadowBlur = 10 * heat;
                   ctx.shadowColor = activeColor;
               } else {
                   // Cooling down
                   ctx.fillStyle = activeColor;
                   ctx.shadowBlur = 0;
               }

               ctx.fillRect(x - offset, y - offset, size, size);
               ctx.shadowBlur = 0;
           }

           // Global "Growing" Flash
           if (currentProps.isGrowing && Math.random() > 0.98) {
               ctx.fillStyle = '#ffffff';
               ctx.globalAlpha = 0.4;
               ctx.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE);
           }
       }
       
       ctx.globalAlpha = 1.0;
       animationId = requestAnimationFrame(render);
    };

    if (!isPaused) {
        render();
    }

    return () => {
        if (animationId) cancelAnimationFrame(animationId);
    };
  }, [dimensions, kernelSize, isPaused]); // Only restart loop if these structural/control props change

  const currentTheme = getTheme(props.era);

  return (
    <div className={`backdrop-blur-md border-t font-mono relative overflow-hidden flex-shrink-0 bg-black/40 h-48 flex flex-col`} style={{borderColor: currentTheme.border}}>
      
      {/* HUD Header */}
      <div className="flex justify-between items-center px-4 py-2 border-b border-white/5 bg-black/20 z-20">
        <div className="flex items-center space-x-3">
           <div 
             className={`w-2 h-2 rounded-sm ${(!isPaused && (props.isActive || props.isGrowing || props.isReading)) ? 'bg-white' : 'bg-gray-700'}`}
             style={{ animation: (!isPaused && (props.isActive || props.isGrowing || props.isReading)) ? 'spin 1s linear infinite' : 'none' }}
           ></div>
           <span className="tracking-widest uppercase font-bold text-xs text-gray-400">
               SYSTEM_MEMORY_MAP <span className="text-gray-600 mx-2">//</span> HEAP_VISUALIZER
           </span>
        </div>
        <div className="flex space-x-6 text-[10px] text-gray-500 font-bold tracking-wider">
           <div className="flex flex-col items-end">
               <span className="text-gray-700 uppercase">Size</span>
               <span className={props.isGrowing ? 'text-white animate-pulse' : 'text-gray-300'}>{kernelSize} Bytes</span>
           </div>
           <div className="flex flex-col items-end">
               <span className="text-gray-700 uppercase">Blocks</span>
               <span className="text-gray-300">{Math.ceil(kernelSize / (kernelSize < 256 ? 1 : kernelSize < 1024 ? 4 : 16))}</span>
           </div>
        </div>
      </div>

      {/* Canvas Container */}
      <div ref={containerRef} className="flex-1 overflow-hidden relative p-4">
          <canvas ref={canvasRef} className="block" />
      </div>
      
      {/* Footer Overlay */}
      <div className="absolute bottom-2 right-4 text-[9px] text-gray-500 bg-black/80 px-2 py-1 rounded border border-gray-800 backdrop-blur-sm flex items-center gap-2 pointer-events-none z-20 shadow-lg">
          <span>PTR: 0x{props.focusAddress.toString(16).toUpperCase().padStart(4, '0')}</span>
          {props.isReading && <span className="text-green-400 font-bold animate-pulse">[READ]</span>}
          {props.isActive && !props.isReading && <span className="text-cyan-400 font-bold animate-pulse">[WRITE]</span>}
      </div>
    </div>
  );
};