import type { CanFrame } from '@teslacanmodder/protocol';

export interface FilterPreset {
  label: string;
  bus?: number;
  dir?: string;
  idMin?: number;
  idMax?: number;
}

export const BUILT_IN_PRESETS: FilterPreset[] = [
  { label: 'All Frames' },
  { label: 'FSD Bus Only', bus: 0 },
  { label: 'Vehicle Bus Only', bus: 1 },
  { label: 'Body Bus Only', bus: 2 },
  { label: 'RX Only', dir: 'rx' },
  { label: 'TX Only', dir: 'tx' },
];

export function framesToCsv(frames: CanFrame[]): string {
  const header = 'Time,Bus,BusName,Dir,ID,DLC,Data';
  const rows = frames.map(f =>
    `${f.ts},${f.bus},${f.busName},${f.dir},0x${f.id.toString(16).toUpperCase()},${f.dlc},${f.data || ''}`
  );
  return [header, ...rows].join('\n');
}

export function framesToJson(frames: CanFrame[]): string {
  return JSON.stringify(frames, null, 2);
}
