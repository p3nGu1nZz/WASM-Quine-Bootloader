export enum SystemState {
  IDLE = 'IDLE',
  BOOTING = 'BOOTING',
  LOADING_KERNEL = 'LOADING_KERNEL',
  EXECUTING = 'EXECUTING',
  VERIFYING_QUINE = 'VERIFYING_QUINE',
  SYSTEM_HALT = 'SYSTEM_HALT',
  REPAIRING = 'REPAIRING', // New state for self-repair
}

export enum SystemEra {
  PRIMORDIAL = 'PRIMORDIAL',
  EXPANSION = 'EXPANSION',
  COMPLEXITY = 'COMPLEXITY',
  SINGULARITY = 'SINGULARITY',
}

export interface LogEntry {
  id: string;
  timestamp: number;
  message: string;
  type: 'info' | 'success' | 'warning' | 'error' | 'system' | 'mutation';
}

export interface HistoryEntry {
  generation: number;
  timestamp: string;
  size: number;
  action: string;
  details: string;
  success: boolean;
}

export interface BootConfig {
  memorySizePages: number;
  autoReboot: boolean;
  rebootDelayMs: number;
}
