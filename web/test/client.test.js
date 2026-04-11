import assert from 'node:assert/strict';

import { parseSerialChunk, parseSerialLine } from '../src/lib/board/client.ts';

function run(name, callback) {
  try {
    callback();
    console.log(`PASS ${name}`);
  } catch (error) {
    console.error(`FAIL ${name}`);
    throw error;
  }
}

run('parseSerialLine extracts valid JSON from noisy serial lines', () => {
  const events = parseSerialLine('garbage{"t":"status","variant":"hw4","rawCan":0}tail');

  assert.equal(events.length, 1);
  assert.equal(events[0].type, 'message');
  assert.equal(events[0].message.t, 'status');
  assert.equal(events[0].message.variant, 'hw4');
});

run('parseSerialLine strips embedded null bytes before parsing JSON', () => {
  const events = parseSerialLine('{"t":"status","hw":"Arduino\u0000Uno","sp":3}');

  assert.equal(events.length, 1);
  assert.equal(events[0].type, 'message');
  assert.equal(events[0].message.hw, 'ArduinoUno');
  assert.equal(events[0].message.sp, 3);
});

run('parseSerialLine ignores truncated non-JSON fragments instead of surfacing fake parse errors', () => {
  const events = parseSerialLine('n":0,"emitted":60},"rawCan":0,"up":41347}');

  assert.equal(events.length, 1);
  assert.equal(events[0].type, 'ignore');
});

run('parseSerialChunk keeps the unfinished trailing fragment as remainder', () => {
  const { remainder, events } = parseSerialChunk('', '{"t":"pong"}\n{"t":"ack","cmd":"profile:2"}');

  assert.equal(events.length, 1);
  assert.equal(events[0].type, 'message');
  assert.equal(events[0].message.t, 'pong');
  assert.equal(remainder, '{"t":"ack","cmd":"profile:2"}');
});

