/**
 * Reducer lifecycle notification tests.
 * Ensures ack/error/pong/log messages and notification cap behavior remain stable.
 */

import { addNotification, initialBoardState, reduceBoardMessage } from '../../src/reducer.js';
import type { BoardState } from '../../src/types.js';

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, '12:34:56');

describe('reducer: lifecycle notifications', () => {
  beforeEach(() => {
    nextId = 0;
  });

  it('adds ack notification as info', () => {
    const state = reduce({ ...initialBoardState }, { t: 'ack', cmd: 'fsd:on' });
    expect(state.messages[0]).toEqual({
      id: 1,
      type: 'info',
      text: 'OK fsd:on',
      ts: '12:34:56',
    });
  });

  it('adds error notification as error with fallback text', () => {
    const withMessage = reduce({ ...initialBoardState }, { t: 'error', msg: 'bad command' });
    expect(withMessage.messages[0].type).toBe('error');
    expect(withMessage.messages[0].text).toBe('bad command');

    const fallback = reduce({ ...initialBoardState }, { t: 'error' });
    expect(fallback.messages[0].type).toBe('error');
    expect(fallback.messages[0].text).toBe('Error');
  });

  it('adds pong notification', () => {
    const state = reduce({ ...initialBoardState }, { t: 'pong' });
    expect(state.messages[0].text).toBe('Pong received');
    expect(state.messages[0].type).toBe('info');
  });

  it('toggles summonActive on log lifecycle texts', () => {
    const started = reduce({ ...initialBoardState, summonActive: false }, { t: 'log', msg: 'Summon burst started' });
    expect(started.summonActive).toBe(true);

    const completed = reduce({ ...started, summonActive: true }, { t: 'log', msg: 'Summon burst complete' });
    expect(completed.summonActive).toBe(false);
  });

  it('caps notifications at 100 entries', () => {
    let state = { ...initialBoardState };
    for (let i = 0; i < 105; i += 1) {
      state = addNotification(state, 'info', `msg-${i}`, i, '12:34:56');
    }

    expect(state.messages).toHaveLength(100);
    expect(state.messages[0].text).toBe('msg-104');
    expect(state.messages[99].text).toBe('msg-5');
  });
});
