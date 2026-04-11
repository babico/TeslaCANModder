import assert from 'node:assert/strict';

import {
  MAX_FRAME_BUFFER,
  createFrameMonitorState,
  ingestFrameMonitorMessage,
  normalizeStatusMessage,
} from '../src/lib/board/protocol.ts';

function run(name, callback) {
  try {
    callback();
    console.log(`PASS ${name}`);
  } catch (error) {
    console.error(`FAIL ${name}`);
    throw error;
  }
}

run('normalizeStatusMessage uses board stream metadata', () => {
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

  assert.equal(status.streamEnabled, true);
  assert.equal(status.streamedFrameCount, 42);
  assert.equal(status.rate, 18);
  assert.equal(status.isaChimeEnabled, false);
});

run('ingestFrameMonitorMessage keeps board timestamp and tx diff data', () => {
  let monitor = createFrameMonitorState();

  monitor = ingestFrameMonitorMessage(monitor, {
    id: 1021,
    dir: 'rx',
    dlc: 3,
    seq: 1,
    ms: 400,
    ext: 0,
    d: 'AABBCC',
  });

  monitor = ingestFrameMonitorMessage(monitor, {
    id: 1021,
    dir: 'tx',
    dlc: 3,
    seq: 2,
    ms: 405,
    ext: 0,
    d: 'AABDCC',
  });

  assert.equal(monitor.totalReceived, 2);
  assert.equal(monitor.frames[0].boardMs, 405);
  assert.deepEqual(monitor.frames[0].changedByteIndexes, [1]);
  assert.equal(monitor.groups[0].count, 2);
});

run('ingestFrameMonitorMessage tracks trimmed frames when the rolling buffer overflows', () => {
  let monitor = createFrameMonitorState();

  for (let index = 0; index < MAX_FRAME_BUFFER + 5; index += 1) {
    monitor = ingestFrameMonitorMessage(monitor, {
      id: 1000 + (index % 3),
      dir: 'rx',
      dlc: 2,
      seq: index + 1,
      ms: index * 10,
      ext: 0,
      d: 'ABCD',
    });
  }

  assert.equal(monitor.frames.length, MAX_FRAME_BUFFER);
  assert.equal(monitor.totalReceived, MAX_FRAME_BUFFER + 5);
  assert.equal(monitor.trimmedCount, 5);
});

run('ingestFrameMonitorMessage rejects malformed frame payloads and counts them as parse errors', () => {
  let monitor = createFrameMonitorState();

  monitor = ingestFrameMonitorMessage(monitor, {
    id: 1021,
    dir: 'rx',
    dlc: 3,
    seq: 1,
    ms: 100,
    ext: 0,
    d: 'ABC',
  });

  assert.equal(monitor.frames.length, 0);
  assert.equal(monitor.totalReceived, 0);
  assert.equal(monitor.parseErrors, 1);
});
