import { describe, it, expect } from 'vitest';
import { renderHook, act } from '@testing-library/react';
import { useBoardState } from '../../src/hooks/useBoardState';

describe('useBoardState', () => {
  it('initializes with default state', () => {
    const { result } = renderHook(() => useBoardState());
    expect(result.current.state.variant).toBe('hw4');
    expect(result.current.state.busFsd).toBe(true);
    expect(result.current.state.fsd).toBe(false);
    expect(result.current.state.frames).toEqual([]);
    expect(result.current.state.messages).toEqual([]);
  });

  it('handles boot message and merges state', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({
        t: 'boot',
        variant: 'hw3',
        hw: 'ESP32-S',
        drv: 'esp32-twai',
        fsd: 1,
        nag: 1,
        features: { fsd: true, profile: true, nag: true, speedOffset: false, isaSpeedChime: false, summon: false },
      });
    });
    expect(result.current.state.variant).toBe('hw3');
    expect(result.current.state.hardware).toBe('ESP32-S');
    expect(result.current.state.fsd).toBe(true);
    expect(result.current.state.nag).toBe(true);
    // Boot adds a console message
    expect(result.current.state.messages.length).toBeGreaterThan(0);
  });

  it('handles status message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({
        t: 'status',
        variant: 'hw4',
        up: 5000,
        rate: 25,
        fsd: 1,
        nag: 0,
        sp: 2,
        offset: 40,
        stream: { on: 1, emitted: 100 },
        features: { fsd: true, profile: true, nag: true, speedOffset: true, isaSpeedChime: false, summon: false },
      });
    });
    expect(result.current.state.uptime).toBe(5000);
    expect(result.current.state.rate).toBe(25);
    expect(result.current.state.fsd).toBe(true);
    expect(result.current.state.nag).toBe(false);
    expect(result.current.state.profile).toBe(2);
    expect(result.current.state.offset).toBe(40);
  });

  it('handles frame message and caps at 100', () => {
    const { result } = renderHook(() => useBoardState());

    // Add 105 frames
    act(() => {
      for (let i = 0; i < 105; i++) {
        result.current.handleMessage({
          t: 'frame',
          id: 1021,
          dir: 'rx',
          dlc: 3,
          bus: 0,
          d: 'AABBCC',
        });
      }
    });

    expect(result.current.state.frames.length).toBeLessThanOrEqual(100);
    expect(result.current.state.frameCount).toBe(105);
  });

  it('handles ack message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'ack', cmd: 'fsd:1' });
    });
    expect(result.current.state.messages[0].text).toContain('fsd:1');
  });

  it('handles error message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'error', msg: 'Unknown command' });
    });
    expect(result.current.state.messages[0].type).toBe('error');
  });

  it('handles pong message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'pong' });
    });
    expect(result.current.state.messages[0].text).toContain('Pong');
  });

  it('addMessage prepends and caps at 100', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      for (let i = 0; i < 110; i++) {
        result.current.addMessage('info', `msg ${i}`);
      }
    });
    expect(result.current.state.messages.length).toBeLessThanOrEqual(100);
  });

  it('clearFrames resets frames and count', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'frame', id: 1021, dir: 'rx', dlc: 3, bus: 0, d: 'AA' });
    });
    expect(result.current.state.frameCount).toBeGreaterThan(0);

    act(() => { result.current.clearFrames(); });
    expect(result.current.state.frames).toEqual([]);
    expect(result.current.state.frameCount).toBe(0);
  });

  it('clearMessages resets messages', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => { result.current.addMessage('info', 'test'); });
    expect(result.current.state.messages.length).toBe(1);

    act(() => { result.current.clearMessages(); });
    expect(result.current.state.messages).toEqual([]);
  });

  it('reset restores initial state', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'boot', variant: 'hw3', hw: 'ESP', fsd: 1 });
      result.current.addMessage('info', 'test');
    });
    expect(result.current.state.variant).toBe('hw3');

    act(() => { result.current.reset(); });
    expect(result.current.state.variant).toBe('hw4');
    expect(result.current.state.messages).toEqual([]);
  });
});
