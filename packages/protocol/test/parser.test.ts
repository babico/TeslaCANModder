import { parseSerialLine, parseSerialChunk } from '../src/parser.js';

describe('parseSerialLine', () => {
  it('extracts valid JSON from a line', () => {
    const events = parseSerialLine('{"t":"boot","variant":"hw4"}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message).toEqual({ t: 'boot', variant: 'hw4' });
  });

  it('extracts JSON from noisy line', () => {
    const events = parseSerialLine('noise {"t":"pong"} more');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message?.t).toBe('pong');
  });

  it('returns ignore for non-JSON lines', () => {
    const events = parseSerialLine('just plain text');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('ignore');
  });

  it('returns parse-error for invalid JSON', () => {
    const events = parseSerialLine('{bad json}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('parse-error');
    expect(events[0].reason).toBeDefined();
  });

  it('returns empty for empty string', () => {
    expect(parseSerialLine('')).toEqual([]);
  });

  it('returns empty for null bytes only', () => {
    expect(parseSerialLine('\0\0\0')).toEqual([]);
  });

  it('returns empty for whitespace only', () => {
    expect(parseSerialLine('   ')).toEqual([]);
  });

  it('handles nested JSON', () => {
    const events = parseSerialLine('{"t":"status","stream":{"on":true}}');
    expect(events).toHaveLength(1);
    expect(events[0].message?.stream).toEqual({ on: true });
  });

  it('strips null bytes before parsing', () => {
    const events = parseSerialLine('\0{"t":"pong"}\0');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
  });
});

describe('parseSerialChunk', () => {
  it('splits chunk on newlines', () => {
    const { events, remainder } = parseSerialChunk('', '{"t":"boot"}\n{"t":"pong"}\n');
    expect(events).toHaveLength(2);
    expect(remainder).toBe('');
  });

  it('carries partial line into remainder', () => {
    const { events, remainder } = parseSerialChunk('', '{"t":"bo');
    expect(events).toHaveLength(0);
    expect(remainder).toBe('{"t":"bo');
  });

  it('joins remainder with next chunk', () => {
    const r1 = parseSerialChunk('', '{"t":"bo');
    const r2 = parseSerialChunk(r1.remainder, 'ot"}\n');
    expect(r2.events).toHaveLength(1);
    expect(r2.events[0].type).toBe('message');
    expect(r2.events[0].message?.t).toBe('boot');
  });

  it('handles multiple chunks with leftovers', () => {
    const r1 = parseSerialChunk('', '{"t":"a"}\n{"t":');
    expect(r1.events).toHaveLength(1);
    expect(r1.remainder).toBe('{"t":');

    const r2 = parseSerialChunk(r1.remainder, '"b"}\n');
    expect(r2.events).toHaveLength(1);
    expect(r2.events[0].message?.t).toBe('b');
    expect(r2.remainder).toBe('');
  });
});
