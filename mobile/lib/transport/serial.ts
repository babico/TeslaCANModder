/**
 * Web Serial Transport — USB connection via Web Serial API.
 * Only available on web platform (Chrome/Edge).
 */

import type { Transport, TransportEvents } from './types';

export class SerialTransport implements Transport {
  readonly type = 'serial' as const;
  private port: SerialPort | null = null;
  private reader: ReadableStreamDefaultReader<Uint8Array> | null = null;
  private writer: WritableStreamDefaultWriter<Uint8Array> | null = null;
  private listeners: TransportEvents | null = null;
  private _connected = false;
  private _deviceName: string | null = null;

  get connected() { return this._connected; }
  get deviceName() { return this._deviceName; }

  setListeners(events: TransportEvents) {
    this.listeners = events;
  }

  async connect(): Promise<void> {
    if (typeof navigator === 'undefined' || !('serial' in navigator)) {
      throw new Error('Web Serial API not available. Use Chrome or Edge.');
    }

    const port = await (navigator as any).serial.requestPort();
    await port.open({ baudRate: 115200 });

    this.port = port;
    this.writer = port.writable.getWriter();
    this._connected = true;
    this._deviceName = 'USB Serial';

    const reader = port.readable.getReader();
    this.reader = reader;

    const decoder = new TextDecoder();
    let buffer = '';

    (async () => {
      try {
        while (true) {
          const { value, done } = await reader.read();
          if (done) break;

          buffer += decoder.decode(value, { stream: true });
          const lines = buffer.split('\n');
          buffer = lines.pop() || '';

          for (const line of lines) {
            const trimmed = line.trim();
            if (!trimmed) continue;
            const start = trimmed.indexOf('{');
            const end = trimmed.lastIndexOf('}');
            if (start >= 0 && end > start) {
              try {
                const msg = JSON.parse(trimmed.slice(start, end + 1));
                this.listeners?.onMessage(msg);
              } catch { /* not valid JSON */ }
            }
          }
        }
      } catch (err) {
        this.listeners?.onError(err instanceof Error ? err : new Error(String(err)));
      } finally {
        this._connected = false;
        this.listeners?.onDisconnect();
      }
    })();
  }

  async disconnect(): Promise<void> {
    try {
      if (this.reader) { await this.reader.cancel(); this.reader = null; }
      if (this.writer) { await this.writer.close(); this.writer = null; }
      if (this.port) { await this.port.close(); this.port = null; }
    } catch { /* ignore cleanup errors */ }
    this._connected = false;
  }

  async send(command: string): Promise<void> {
    if (!this.writer) return;
    const encoder = new TextEncoder();
    await this.writer.write(encoder.encode(command + '\n'));
  }
}

export function canUseWebSerial(): boolean {
  return typeof navigator !== 'undefined' && 'serial' in navigator;
}
