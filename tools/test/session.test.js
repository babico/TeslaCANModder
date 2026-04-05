import { describe, it, expect, jest, beforeEach } from '@jest/globals';
import { BoardSession } from '../lib/session.js';
import { EventEmitter } from 'node:events';

/** Create a fake serial port (EventEmitter with write/close). */
function fakePort() {
  const sp = new EventEmitter();
  sp.write = jest.fn((data, cb) => cb?.());
  sp.close = jest.fn((cb) => cb?.());
  return sp;
}

describe('BoardSession', () => {
  let sp, session;

  beforeEach(() => {
    sp = fakePort();
    session = new BoardSession(sp);
  });

  function feedLine(line) {
    // Simulate readline 'line' event — BoardSession uses readline on the port
    // We trigger _onRawLine directly since we can't easily mock readline here
    session._onRawLine(line);
  }

  it('parses JSON messages', () => {
    feedLine('{"t":"boot","variant":"hw4"}');
    const msgs = session.messages();
    expect(msgs).toHaveLength(1);
    expect(msgs[0].msg.t).toBe('boot');
    expect(msgs[0].msg.variant).toBe('hw4');
  });

  it('handles non-JSON lines as text', () => {
    feedLine('random noise');
    const msgs = session.messages();
    expect(msgs).toHaveLength(1);
    expect(msgs[0].type).toBe('text');
    expect(msgs[0].msg).toBeNull();
  });

  it('handles parse errors', () => {
    feedLine('{invalid json}');
    const msgs = session.messages();
    expect(msgs).toHaveLength(1);
    expect(msgs[0].type).toBe('parse-error');
  });

  it('ignores empty lines', () => {
    feedLine('');
    feedLine('  ');
    feedLine('\r');
    expect(session.messages()).toHaveLength(0);
  });

  it('extracts JSON from noisy lines', () => {
    feedLine('noise {"t":"pong"} more noise');
    const msgs = session.messages();
    expect(msgs).toHaveLength(1);
    expect(msgs[0].msg.t).toBe('pong');
  });

  it('waitForType resolves on matching message', async () => {
    const promise = session.waitForType('pong');
    feedLine('{"t":"pong"}');
    const entry = await promise;
    expect(entry.msg.t).toBe('pong');
  });

  it('waitForType returns null on timeout', async () => {
    const entry = await session.waitForType('boot', 50);
    expect(entry).toBeNull();
  });

  it('waitFor resolves from queue if message already there', async () => {
    feedLine('{"t":"boot"}');
    const entry = await session.waitFor(e => e.msg?.t === 'boot');
    expect(entry.msg.t).toBe('boot');
  });

  it('waitForAck matches cmd field', async () => {
    const promise = session.waitForAck('stream:on');
    feedLine('{"t":"ack","cmd":"stream:on"}');
    const entry = await promise;
    expect(entry.msg.cmd).toBe('stream:on');
  });

  it('drainType returns and removes matching entries', () => {
    feedLine('{"t":"frame","id":69}');
    feedLine('{"t":"frame","id":281}');
    feedLine('{"t":"status"}');
    const frames = session.drainType('frame');
    expect(frames).toHaveLength(2);
    // status should still be in queue
    const remaining = session.messages().filter(m => m.msg?.t === 'status');
    expect(remaining).toHaveLength(1);
  });

  it('send writes to serial port', () => {
    session.send('ping');
    expect(sp.write).toHaveBeenCalled();
  });
});
