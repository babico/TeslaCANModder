/** Web-specific board protocol helpers for frame monitoring and status normalization. */

export interface MonitorFrame {
  canId: number;
  dir: string;
  dlc: number;
  seq: number;
  boardMs: number;
  ext: number;
  bytes: number[];
  hex: string;
  changedByteIndexes: number[];
}

export interface FrameGroup {
  canId: number;
  count: number;
}

export interface FrameMonitorState {
  frames: MonitorFrame[];
  groups: FrameGroup[];
  totalReceived: number;
  trimmedCount: number;
  parseErrors: number;
}

export interface RawFrameMessage {
  id: number;
  dir: string;
  dlc: number;
  seq: number;
  ms: number;
  ext: number;
  d: string;
}

export interface RawStatusMessage {
  up?: number;
  variant?: string;
  drv?: string;
  hw?: string;
  can?: string;
  ready?: string;
  cap?: string;
  bt?: number;
  offset?: number;
  isaChime?: number;
  stream?: { on: number; emitted: number };
  features?: Record<string, number>;
}

export const MAX_FRAME_BUFFER = 500;

export function createFrameMonitorState(): FrameMonitorState {
  return {
    frames: [],
    groups: [],
    totalReceived: 0,
    trimmedCount: 0,
    parseErrors: 0,
  };
}

export function normalizeStatusMessage(msg: RawStatusMessage, rate: number) {
  return {
    uptime: msg.up || 0,
    variant: msg.variant || 'hw4',
    driver: msg.drv || '—',
    hardware: msg.hw || '—',
    canChip: msg.can || '—',
    ready: msg.ready || '',
    capabilities: msg.cap || '',
    bluetooth: Boolean(msg.bt),
    offset: msg.offset ?? 0,
    isaChimeEnabled: Boolean(msg.isaChime),
    streamEnabled: Boolean(msg.stream?.on),
    streamedFrameCount: msg.stream?.emitted ?? 0,
    rate: rate || 0,
    features: msg.features || {},
  };
}

function parseHexBytes(hex: string): number[] {
  const bytes = [];
  for (let i = 0; i < hex.length; i += 2) {
    bytes.push(parseInt(hex.slice(i, i + 2), 16));
  }
  return bytes;
}

export function ingestFrameMonitorMessage(state: FrameMonitorState, frame: RawFrameMessage): FrameMonitorState {
  // Validate: hex data length must be 2 * dlc
  if (!frame.d || frame.d.length !== frame.dlc * 2) {
    return { ...state, parseErrors: state.parseErrors + 1 };
  }

  const bytes = parseHexBytes(frame.d);

  // Find most recent frame with same id to compute changed bytes
  const prev = state.frames.find(f => f.canId === frame.id);
  const changedByteIndexes = [];
  if (prev) {
    for (let i = 0; i < bytes.length; i++) {
      if (bytes[i] !== prev.bytes[i]) {
        changedByteIndexes.push(i);
      }
    }
  }

  const entry = {
    canId: frame.id,
    dir: frame.dir,
    dlc: frame.dlc,
    seq: frame.seq,
    boardMs: frame.ms,
    ext: frame.ext,
    bytes,
    hex: frame.d,
    changedByteIndexes,
  };

  // Prepend new frame to rolling buffer
  const frames = [entry, ...state.frames];

  // Trim if over buffer
  const trimmed = frames.length > MAX_FRAME_BUFFER
    ? frames.slice(0, MAX_FRAME_BUFFER)
    : frames;
  const newTrimmed = frames.length - trimmed.length;

  // Update groups
  const groupIdx = state.groups.findIndex(g => g.canId === frame.id);
  const groups = [...state.groups];
  if (groupIdx >= 0) {
    groups[groupIdx] = { ...groups[groupIdx], count: groups[groupIdx].count + 1 };
  } else {
    groups.push({ canId: frame.id, count: 1 });
  }

  return {
    frames: trimmed,
    groups,
    totalReceived: state.totalReceived + 1,
    trimmedCount: state.trimmedCount + newTrimmed,
    parseErrors: state.parseErrors,
  };
}
