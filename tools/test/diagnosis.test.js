import { describe, it, expect } from '@jest/globals';
import { buildMatrix, diagnose, tagPhase } from '../lib/diagnosis.js';

function msg(t, extra = {}) {
  const m = { t, ...extra };
  return { raw: JSON.stringify(m), msg: m, type: t };
}

describe('buildMatrix', () => {
  it('detects boot + pong → protocolOk', () => {
    const m = buildMatrix([msg('boot'), msg('pong')], null);
    expect(m.bootSeen).toBe(true);
    expect(m.pongSeen).toBe(true);
    expect(m.protocolOk).toBe(true);
  });

  it('no boot → protocolOk false', () => {
    const m = buildMatrix([msg('pong')], null);
    expect(m.bootSeen).toBe(false);
    expect(m.protocolOk).toBe(false);
  });

  it('detects boot from raw text fallback', () => {
    const entry = { raw: '{"t":"boot","variant":"hw4"}', msg: null, type: 'parse-error' };
    const m = buildMatrix([entry, msg('pong')], null);
    expect(m.bootSeen).toBe(true);
  });

  it('counts frames', () => {
    const messages = [
      msg('boot'), msg('pong'),
      msg('frame', { id: 69 }),
      msg('frame', { id: 281 }),
    ];
    const m = buildMatrix(messages, null);
    expect(m.totalFrameCount).toBe(2);
  });

  it('detects rawCanSupported via ack', () => {
    const messages = [msg('boot'), msg('pong'), msg('ack', { cmd: 'can:raw:on' })];
    const m = buildMatrix(messages, null);
    expect(m.rawCanSupported).toBe(true);
    expect(m.firmwareNeedsRawMode).toBe(false);
  });

  it('detects rawCanSupported via status field', () => {
    const messages = [msg('boot'), msg('pong'), msg('status', { rawCan: 1 })];
    const m = buildMatrix(messages, null);
    expect(m.rawCanSupported).toBe(true);
    expect(m.rawCanEnabled).toBe(true);
  });

  it('counts parse errors', () => {
    const messages = [
      msg('boot'), msg('pong'),
      { raw: 'garbage', msg: null, type: 'parse-error' },
    ];
    const m = buildMatrix(messages, null);
    expect(m.parseErrorCount).toBe(1);
    expect(m.noiseLikely).toBe(true);
  });

  it('detects filterMismatchLikely', () => {
    const rawFrame = msg('frame', { id: 69 });
    rawFrame._phase = 'raw-can';
    const messages = [
      msg('boot'), msg('pong'),
      msg('status', { rawCan: 1 }),
      msg('ack', { cmd: 'can:raw:on' }),
      rawFrame,
    ];
    const m = buildMatrix(messages, null);
    expect(m.filterMismatchLikely).toBe(true);
  });

  it('detects physicalCanIssueLikely', () => {
    const messages = [
      msg('boot'), msg('pong'),
      msg('status', { rawCan: 1 }),
      msg('ack', { cmd: 'can:raw:on' }),
    ];
    const m = buildMatrix(messages, null);
    expect(m.physicalCanIssueLikely).toBe(true);
  });
});

describe('diagnose', () => {
  it('handshake failed', () => {
    expect(diagnose({ protocolOk: false })).toMatch(/handshake failed/i);
  });

  it('firmware needs raw mode', () => {
    expect(diagnose({ protocolOk: true, firmwareNeedsRawMode: true })).toMatch(/raw/i);
  });

  it('filter mismatch', () => {
    expect(diagnose({
      protocolOk: true, firmwareNeedsRawMode: false, filterMismatchLikely: true,
    })).toMatch(/variant|filter/i);
  });

  it('physical CAN issue', () => {
    expect(diagnose({
      protocolOk: true, firmwareNeedsRawMode: false, filterMismatchLikely: false,
      physicalCanIssueLikely: true,
    })).toMatch(/wiring/i);
  });

  it('working', () => {
    expect(diagnose({
      protocolOk: true, firmwareNeedsRawMode: false, filterMismatchLikely: false,
      physicalCanIssueLikely: false, filteredFrameCount: 5,
    })).toMatch(/working/i);
  });

  it('inconclusive fallback', () => {
    expect(diagnose({
      protocolOk: true, firmwareNeedsRawMode: false, filterMismatchLikely: false,
      physicalCanIssueLikely: false, filteredFrameCount: 0, latestStatus: null,
    })).toMatch(/inconclusive/i);
  });
});

describe('tagPhase', () => {
  it('tags un-phased messages', () => {
    const msgs = [{ _phase: undefined }, { _phase: 'existing' }, {}];
    const fakeSession = { messages: () => msgs };
    tagPhase(fakeSession, 'test-phase');
    expect(msgs[0]._phase).toBe('test-phase');
    expect(msgs[1]._phase).toBe('existing');
    expect(msgs[2]._phase).toBe('test-phase');
  });
});
