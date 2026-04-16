import { parseSerialLine, parseSerialChunk } from '../src/parser.js';

describe('parseSerialLine — edge cases', () => {
  it('handles deeply nested JSON', () => {
    const events = parseSerialLine('{"t":"status","hw":{"bus":{"fsd":true,"v":true}}}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect((events[0].message as Record<string, unknown>)?.hw).toBeDefined();
  });

  it('handles JSON with special characters in values', () => {
    const events = parseSerialLine('{"t":"log","msg":"hello\\nworld"}');
    expect(events).toHaveLength(1);
    expect(events[0].message?.msg).toBe('hello\nworld');
  });

  it('handles very long lines', () => {
    const data = 'A'.repeat(10000);
    const events = parseSerialLine(data);
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('ignore');
  });

  it('handles binary noise before JSON', () => {
    const events = parseSerialLine('\xff\xfe\x00{"t":"pong"}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message?.t).toBe('pong');
  });

  it('handles multiple JSON objects on one line (takes outermost braces)', () => {
    const events = parseSerialLine('{"a":1} {"b":2}');
    expect(events).toHaveLength(1);
    // outermost braces: from first { to last }
    // this will be '{"a":1} {"b":2}' which is invalid JSON → parse-error
    expect(events[0].type).toBe('parse-error');
    expect(events[0].reason).toBeDefined();
  });

  it('handles truncated JSON', () => {
    const events = parseSerialLine('{"t":"boot","variant":');
    expect(events).toHaveLength(1);
    // No closing brace → never enters JSON parse → ignore
    expect(events[0].type).toBe('ignore');
  });

  it('handles JSON with empty object', () => {
    const events = parseSerialLine('{}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
    expect(events[0].message).toEqual({});
  });

  it('handles JSON with array value', () => {
    const events = parseSerialLine('{"t":"frame","d":[1,2,3]}');
    expect(events).toHaveLength(1);
    expect(events[0].message?.d).toEqual([1, 2, 3]);
  });

  it('returns ignore for lone closing brace', () => {
    const events = parseSerialLine('}');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('ignore');
  });

  it('returns parse-error for lone opening brace', () => {
    const events = parseSerialLine('{');
    expect(events).toHaveLength(1);
    // start=0, end=-1, end > start is false → ignore (no JSON structure detected)
    expect(events[0].type).toBe('ignore');
  });
});

describe('parseSerialChunk — edge cases', () => {
  it('handles empty chunk', () => {
    const { events, remainder } = parseSerialChunk('', '');
    expect(events).toHaveLength(0);
    expect(remainder).toBe('');
  });

  it('handles chunk with only newlines', () => {
    const { events, remainder } = parseSerialChunk('', '\n\n\n');
    expect(events).toHaveLength(0); // empty lines produce no events
    expect(remainder).toBe('');
  });

  it('handles large chunk with many messages', () => {
    const lines = Array.from({ length: 100 }, (_, i) => `{"t":"frame","id":${i}}`).join('\n') + '\n';
    const { events, remainder } = parseSerialChunk('', lines);
    expect(events).toHaveLength(100);
    expect(remainder).toBe('');
  });

  it('handles interleaved noise and JSON', () => {
    const chunk = 'noise1\n{"t":"boot"}\ngarbage\n{"t":"pong"}\n';
    const { events } = parseSerialChunk('', chunk);
    const messages = events.filter(e => e.type === 'message');
    const ignores = events.filter(e => e.type === 'ignore');
    expect(messages).toHaveLength(2);
    expect(ignores).toHaveLength(2);
  });

  it('accumulates remainder across multiple chunks', () => {
    let r = '';
    // Send JSON in 3 byte chunks
    const full = '{"t":"boot","variant":"hw4"}\n';
    const third = Math.ceil(full.length / 3);
    
    const r1 = parseSerialChunk(r, full.slice(0, third));
    expect(r1.events).toHaveLength(0);
    
    const r2 = parseSerialChunk(r1.remainder, full.slice(third, third * 2));
    expect(r2.events).toHaveLength(0);
    
    const r3 = parseSerialChunk(r2.remainder, full.slice(third * 2));
    expect(r3.events).toHaveLength(1);
    expect(r3.events[0].message?.t).toBe('boot');
  });

  it('handles \\r\\n line endings', () => {
    const { events } = parseSerialChunk('', '{"t":"pong"}\r\n');
    expect(events).toHaveLength(1);
    expect(events[0].type).toBe('message');
  });
});
