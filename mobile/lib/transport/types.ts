/** Transport abstraction — unifies BLE and USB Serial communication. */

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
