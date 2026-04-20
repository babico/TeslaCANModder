/**
 * Platform & CAN Health Protocol Tests
 * Tests that the reducer correctly maps platform identity and CAN health
 * fields from boot/status messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from '../../src/reducer.js';
import type { BootMessage, StatusMessage, BoardState } from '../../src/types.js';

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, '00:00:00');

describe('reducer: vehicle platform', () => {
  beforeEach(() => { nextId = 0; });

  describe('initialBoardState defaults', () => {
    it('has platform defaults', () => {
      expect(initialBoardState.platformModel).toBe(0);
      expect(initialBoardState.platformHwGen).toBe(0);
      expect(initialBoardState.platformSwYear).toBe(0);
      expect(initialBoardState.platformSwWeek).toBe(0);
      expect(initialBoardState.platformSwRelease).toBe(0);
      expect(initialBoardState.platformFsdProto).toBe(0);
      expect(initialBoardState.platformSwCompat).toBe(0);
      expect(initialBoardState.platformResolved).toBe(false);
    });

    it('has empty canHealth', () => {
      expect(initialBoardState.canHealth).toEqual({});
    });
  });

  describe('applyBoot with platform fields', () => {
    it('maps platform identity from boot', () => {
      const msg: Partial<BootMessage> = {
        t: 'boot',
        hw: 'ESP32S',
        platformModel: 4,       // MODEL_Y
        platformHwGen: 3,       // HW_4
        platformSwYear: 2026,
        platformSwWeek: 14,
        platformSwRelease: 1,
        platformFsdProto: 3,    // FSD_PROTO_V14
        platformSwCompat: 1,    // SW_COMPAT_OK
        platformResolved: 1,
      };
      const state = reduce(initialBoardState, msg);
      expect(state.platformModel).toBe(4);
      expect(state.platformHwGen).toBe(3);
      expect(state.platformSwYear).toBe(2026);
      expect(state.platformSwWeek).toBe(14);
      expect(state.platformSwRelease).toBe(1);
      expect(state.platformFsdProto).toBe(3);
      expect(state.platformSwCompat).toBe(1);
      expect(state.platformResolved).toBe(true);
    });

    it('maps canHealth from boot', () => {
      const msg: Partial<BootMessage> = {
        t: 'boot',
        hw: 'ESP32S',
        canHealth: {
          bus0: { on: 1, det: 1 },
          bus1: { on: 1, det: 0 },
          bus2: { on: 0, det: 0 },
        },
      };
      const state = reduce(initialBoardState, msg);
      expect(state.canHealth.bus0).toEqual({ on: true, det: true });
      expect(state.canHealth.bus1).toEqual({ on: true, det: false });
      expect(state.canHealth.bus2).toEqual({ on: false, det: false });
    });

    it('preserves platform fields when not in message', () => {
      const prev = { ...initialBoardState, platformModel: 3, platformResolved: true };
      const msg: Partial<BootMessage> = { t: 'boot', hw: 'ESP32S' };
      const state = reduce(prev, msg);
      expect(state.platformModel).toBe(3);
      expect(state.platformResolved).toBe(true);
    });
  });

  describe('applyStatus with platform fields', () => {
    it('maps platform identity from status', () => {
      const msg: Partial<StatusMessage> = {
        t: 'status',
        platformModel: 5,       // MODEL_CYBERTRUCK
        platformHwGen: 3,       // HW_4
        platformSwYear: 2026,
        platformSwWeek: 2,
        platformSwRelease: 9,
        platformFsdProto: 3,    // FSD_PROTO_V14
        platformSwCompat: 1,
        platformResolved: 1,
      };
      const state = reduce(initialBoardState, msg);
      expect(state.platformModel).toBe(5);
      expect(state.platformHwGen).toBe(3);
      expect(state.platformResolved).toBe(true);
    });

    it('maps canHealth from status', () => {
      const msg: Partial<StatusMessage> = {
        t: 'status',
        canHealth: {
          bus0: { on: 1, det: 1 },
          bus1: { on: 1, det: 1 },
          bus2: { on: 1, det: 1 },
        },
      };
      const state = reduce(initialBoardState, msg);
      expect(Object.keys(state.canHealth)).toHaveLength(3);
      expect(state.canHealth.bus0.det).toBe(true);
    });
  });
});

describe('commands: platform', () => {
  it('has platform command', async () => {
    const { commands } = await import('../../src/commands.js');
    expect(commands.platform()).toBe('platform');
  });
});

