import { parseSerialLine, parseSerialChunk } from '../../src/parser.js';

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

  it('normalizes structured boot payloads from hardware JSON', () => {
    const events = parseSerialLine(
      '{"t":"boot","meta":{"variant":"hw3","hw":"Uno","drv":"mcp2515"},"connectivity":{"chassisOnline":true,"vehicleOnline":false},"state":{"fsd":true,"profile":{"value":3,"pinned":true},"offset":{"value":5,"pinned":false}},"features":{"fsd":true}}',
    );

    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message).toMatchObject({
      t: 'boot',
      variant: 'hw3',
      hw: 'Uno',
      drv: 'mcp2515',
      chassisOnline: true,
      vehicleOnline: false,
      fsd: true,
      sp: 3,
      spPin: true,
      offset: 5,
      offsetPin: false,
      features: { fsd: true },
    });
  });

  it('normalizes structured status payloads without dropping nested sections', () => {
    const events = parseSerialLine(
      '{"t":"status","meta":{"variant":"hw4","hw":"ESP32","drv":"native","up":1234},"state":{"stream":{"on":true,"emitted":7},"nagKiller":true},"driverAssist":{"maxSpeed":860},"battery":{"nomFullPack":7123},"safety":{"banShield":true},"can":{"clockReqMHz":16,"clockMHz":16}}',
    );

    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message).toMatchObject({
      t: 'status',
      variant: 'hw4',
      hw: 'ESP32',
      drv: 'native',
      up: 1234,
      nagKiller: true,
      stream: { on: true, emitted: 7 },
      maxSpeed: 860,
      bmsNomFullPack: 7123,
      banShield: true,
      canClockReqMHz: 16,
      canClockMHz: 16,
      meta: { variant: 'hw4', hw: 'ESP32', drv: 'native', up: 1234 },
    });
  });

  it('parses ban shield fields from status payload', () => {
    const events = parseSerialLine('{"t":"status","banShield":1,"banThreat":2,"banDetectCount":7}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message?.banShield).toBe(1);
    expect(events[0].message?.banThreat).toBe(2);
    expect(events[0].message?.banDetectCount).toBe(7);
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

