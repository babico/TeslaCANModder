/** Shared types for TeslaCANModder firmware protocol. */

// ── Board Message Types (discriminated union on `t` field) ──────────────────

export interface BoardFeatures {
  fsd: boolean;
  profile: boolean;
  nag: boolean;
  speedOffset: boolean;
  isaSpeedChime: boolean;
  summon: boolean;
}

export interface BootMessage {
  t: 'boot';
  variant?: string;
  hw?: string;
  drv?: string;
  features?: BoardFeatures;
  fsd?: number;
  nag?: number;
  sp?: number;
  spPin?: number;
  offset?: number;
  offsetPin?: number;
  isaChime?: number;
  summonInject?: number;
  canOnline?: number;
  standby?: number;
  bus1?: number;
  bus2?: number;
  bus3?: number;
  busFsd?: number;
  busVehicle?: number;
  busBody?: number;
}

export interface StatusMessage {
  t: 'status';
  variant?: string;
  hw?: string;
  drv?: string;
  up?: number;
  rate?: number;
  fsd?: number;
  nag?: number;
  sp?: number;
  spPin?: number;
  offset?: number;
  offsetPin?: number;
  isaChime?: number;
  summonInject?: number;
  stream?: { on: number; emitted: number };
  features?: BoardFeatures;
  canOnline?: number;
  standby?: number;
  bus1?: number;
  bus2?: number;
  bus3?: number;
  busFsd?: number;
  busVehicle?: number;
  busBody?: number;
}

export interface FrameMessage {
  t: 'frame';
  id: number;
  dir: string;
  dlc: number;
  seq?: number;
  ms?: number;
  ext?: number;
  d: string;
  bus: number;
}

export interface AckMessage {
  t: 'ack';
  cmd: string;
}

export interface ErrorMessage {
  t: 'error';
  msg: string;
}

export interface LogMessage {
  t: 'log';
  msg: string;
}

export interface PongMessage {
  t: 'pong';
}

export type BoardMessage =
  | BootMessage
  | StatusMessage
  | FrameMessage
  | AckMessage
  | ErrorMessage
  | LogMessage
  | PongMessage;

// ── Board State ─────────────────────────────────────────────────────────────

export interface CanFrame {
  key: string;
  id: number;
  dir: string;
  bus: number;
  busName: string;
  seq?: number;
  dlc: number;
  data: string;
  ts: string;
}

export interface ConsoleMessage {
  id: number;
  type: 'info' | 'error';
  text: string;
  ts: string;
}

export interface BoardState {
  variant: string;
  hardware: string;
  driver: string;
  board: 'arduino' | 'esp32' | 'unknown';
  uptime: number;
  rate: number;

  fsd: boolean;
  nag: boolean;
  profile: number;
  profilePinned: boolean;
  offset: number;
  offsetPinned: boolean;
  isaChime: boolean;
  summonInject: boolean;
  summonActive: boolean;

  canOnline: boolean;
  standby: boolean;
  bus1: boolean;
  bus2: boolean;
  bus3: boolean;
  busFsd: boolean;
  busVehicle: boolean;
  busBody: boolean;

  streaming: boolean;
  frames: CanFrame[];
  frameCount: number;

  messages: ConsoleMessage[];

  features: BoardFeatures;
}

// ── Transport Types ─────────────────────────────────────────────────────────

export interface TransportEvents {
  onMessage: (msg: Record<string, unknown>) => void;
  onDisconnect: () => void;
  onError: (err: Error) => void;
}

export interface Transport {
  readonly type: 'ble' | 'serial';
  readonly connected: boolean;
  readonly deviceName: string | null;
  connect(): Promise<void>;
  disconnect(): Promise<void>;
  send(command: string): Promise<void>;
  setListeners(events: TransportEvents): void;
}

export interface ScannedDevice {
  id: string;
  name: string | null;
  rssi: number | null;
  serviceUuids?: string[];
}

// ── Parser Types ────────────────────────────────────────────────────────────

export interface ParsedEvent {
  type: 'message' | 'ignore' | 'parse-error';
  message?: Record<string, unknown>;
  raw: string;
  reason?: string;
}
