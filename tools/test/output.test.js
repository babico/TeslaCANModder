import { describe, it, expect, jest, beforeEach } from '@jest/globals';
import { createColors, createOutput, ts } from '../lib/output.js';

describe('createColors', () => {
  it('returns ANSI codes when noColor is false', () => {
    const C = createColors(false);
    expect(C.green).toContain('\x1b[');
    expect(C.red).toContain('\x1b[');
    expect(C.reset).toBe('\x1b[0m');
  });

  it('returns empty strings when noColor is true', () => {
    const C = createColors(true);
    expect(C.green).toBe('');
    expect(C.red).toBe('');
    expect(C.yellow).toBe('');
    expect(C.cyan).toBe('');
    expect(C.dim).toBe('');
    expect(C.bold).toBe('');
    expect(C.reset).toBe('');
  });
});

describe('createOutput', () => {
  let out;

  beforeEach(() => {
    jest.spyOn(console, 'log').mockImplementation(() => {});
    out = createOutput(createColors(true));
  });

  it('tracks pass count', () => {
    out.pass('test1');
    out.pass('test2');
    expect(out.counts().passed).toBe(2);
  });

  it('tracks fail count', () => {
    out.fail('bad1');
    expect(out.counts().failed).toBe(1);
  });

  it('tracks warn count', () => {
    out.warn('warn1');
    out.warn('warn2');
    out.warn('warn3');
    expect(out.counts().warned).toBe(3);
  });

  it('resets all counters', () => {
    out.pass('a'); out.fail('b'); out.warn('c');
    out.reset();
    expect(out.counts()).toEqual({ passed: 0, failed: 0, warned: 0 });
  });

  it('info, section, observation log without counting', () => {
    out.info('msg');
    out.section('title');
    out.observation('tag', 'msg');
    expect(out.counts()).toEqual({ passed: 0, failed: 0, warned: 0 });
  });
});

describe('ts', () => {
  it('returns a time string', () => {
    const result = ts();
    // toTimeString().slice(0,12) format varies by locale — just verify it's a non-empty string
    expect(typeof result).toBe('string');
    expect(result.length).toBeGreaterThan(0);
    expect(result).toMatch(/\d{2}:\d{2}:\d{2}/);
  });
});
