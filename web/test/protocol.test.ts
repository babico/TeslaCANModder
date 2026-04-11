import { describe, it, expect } from 'vitest';
import {
  MAX_FRAME_BUFFER,
  createFrameMonitorState,
  ingestFrameMonitorMessage,
  normalizeStatusMessage,
} from '../src/lib/board/protocol.ts';

describe('normalizeStatusMessage', () => {
  it('uses board stream metadata', () => {
    const status = normalizeStatusMessage({
      up: 1234,
      variant: 'hw4',
      drv: 'arduino-mcp2515',
      hw: 'ArduinoUno',
      can: 'MCP2515',
      ready: 'runtime-ready',
      cap: 'usb+bluetooth',
      bt: 1,
      offset: 25,
      isaChime: 0,
      stream: { on: 1, emitted: 42 },
      features: { fsd: 1, profile: 1, nag: 1, speedOffset: 0, isaSpeedChime: 1, forceFsd: 0 },
    }, 18);

    expect(status.streamEnabled).toBe(true);
    expect(status.streamedFrameCount).toBe(42);
    expect(status.rate).toBe(18);
    expect(status.isaChimeEnabled).toBe(false);
  });
});

describe('ingestFrameMonitorMessage', () => {
  it('keeps board timestamp and tx diff data', () => {
    let monitor = createFrameMonitorState();

    monitor = ingestFrameMonitorMessage(monitor, {
      id: 1021, dir: 'rx', dlc: 3, seq: 1, ms: 400, ext: 0, d: 'AABBCC',
    });

    monitor = ingestFrameMonitorMessage(monitor, {
      id: 1021, dir: 'tx', dlc: 3, seq: 2, ms: 405, ext: 0, d: 'AABDCC',
    });

    expect(monitor.totalReceived).toBe(2);
    expect(monitor.frames[0].boardMs).toBe(405);
    expect(monitor.frames[0].changedByteIndexes).toEqual([1]);
    expect(monitor.groups[0].count).toBe(2);
  });

  it('tracks trimmed frames when the rolling buffer overflows', () => {
    let monitor = createFrameMonitorState();

    for (let index = 0; index < MAX_FRAME_BUFFER + 5; index += 1) {
      monitor = ingestFrameMonitorMessage(monitor, {
        id: 1000 + (index % 3), dir: 'rx', dlc: 2, seq: index + 1, ms: index * 10, ext: 0, d: 'ABCD',
      });
    }

    expect(monitor.frames.length).toBe(MAX_FRAME_BUFFER);
    expect(monitor.totalReceived).toBe(MAX_FRAME_BUFFER + 5);
    expect(monitor.trimmedCount).toBe(5);
  });

  it('rejects malformed frame payloads and counts them as parse errors', () => {
    let monitor = createFrameMonitorState();

    monitor = ingestFrameMonitorMessage(monitor, {
      id: 1021, dir: 'rx', dlc: 3, seq: 1, ms: 100, ext: 0, d: 'ABC',
    });

    expect(monitor.frames.length).toBe(0);
    expect(monitor.totalReceived).toBe(0);
    expect(monitor.parseErrors).toBe(1);
  });
});
