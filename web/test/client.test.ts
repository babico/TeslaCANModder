import { describe, it, expect } from 'vitest';
import { parseSerialChunk, parseSerialLine } from '../src/lib/board/client.ts';

describe('parseSerialLine', () => {
  it('extracts valid JSON from noisy serial lines', () => {
    const events = parseSerialLine('garbage{"t":"status","variant":"hw4","rawCan":0}tail');
    expect(events.length).toBe(1);
    expect(events[0]!.type).toBe('message');
    expect(events[0]!.message!.t).toBe('status');
    expect(events[0]!.message!.variant).toBe('hw4');
  });

  it('strips embedded null bytes before parsing JSON', () => {
    const events = parseSerialLine('{"t":"status","hw":"Arduino\u0000Uno","sp":3}');
    expect(events.length).toBe(1);
    expect(events[0]!.type).toBe('message');
    expect(events[0]!.message!.hw).toBe('ArduinoUno');
    expect(events[0]!.message!.sp).toBe(3);
  });

  it('ignores truncated non-JSON fragments instead of surfacing fake parse errors', () => {
    const events = parseSerialLine('n":0,"emitted":60},"rawCan":0,"up":41347}');
    expect(events.length).toBe(1);
    expect(events[0]!.type).toBe('ignore');
  });
});

describe('parseSerialChunk', () => {
  it('keeps the unfinished trailing fragment as remainder', () => {
    const { remainder, events } = parseSerialChunk('', '{"t":"pong"}\n{"t":"ack","cmd":"profile:2"}');
    expect(events.length).toBe(1);
    expect(events[0]!.type).toBe('message');
    expect(events[0]!.message!.t).toBe('pong');
    expect(remainder).toBe('{"t":"ack","cmd":"profile:2"}');
  });
});
