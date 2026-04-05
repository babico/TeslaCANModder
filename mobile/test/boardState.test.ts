import { renderHook, act } from '@testing-library/react-native';
import { useBoardState } from '../hooks/useBoardState';

describe('useBoardState', () => {
  it('returns initial state', () => {
    const { result } = renderHook(() => useBoardState());
    expect(result.current.state.variant).toBe('hw4');
    expect(result.current.state.fsd).toBe(false);
    expect(result.current.state.frames).toEqual([]);
    expect(result.current.state.messages).toEqual([]);
  });

  it('handles boot message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'boot', variant: 'hw3', hw: 'ESP32', drv: 'MCP2515' });
    });
    expect(result.current.state.variant).toBe('hw3');
    expect(result.current.state.hardware).toBe('ESP32');
    expect(result.current.state.driver).toBe('MCP2515');
    // Should add "Board connected" message
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].text).toContain('Board connected');
  });

  it('handles status message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({
        t: 'status', variant: 'hw4', up: 12345, rate: 500,
        fsd: true, nag: false, sp: 2, spPin: true,
        stream: { on: true },
      });
    });
    expect(result.current.state.uptime).toBe(12345);
    expect(result.current.state.rate).toBe(500);
    expect(result.current.state.fsd).toBe(true);
    expect(result.current.state.profile).toBe(2);
    expect(result.current.state.profilePinned).toBe(true);
    expect(result.current.state.streaming).toBe(true);
  });

  it('handles frame message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'frame', id: 69, dir: 'rx', dlc: 8, d: 'AABB', seq: 1 });
    });
    expect(result.current.state.frames).toHaveLength(1);
    expect(result.current.state.frames[0].id).toBe(69);
    expect(result.current.state.frameCount).toBe(1);
  });

  it('caps frames at 100', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      for (let i = 0; i < 110; i++) {
        result.current.handleMessage({ t: 'frame', id: i, dir: 'rx', dlc: 8, d: 'AA', seq: i });
      }
    });
    expect(result.current.state.frames).toHaveLength(100);
    expect(result.current.state.frameCount).toBe(110);
  });

  it('handles ack message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'ack', cmd: 'fsd:on' });
    });
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].text).toContain('OK fsd:on');
  });

  it('handles error message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'error', msg: 'Unknown command' });
    });
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].type).toBe('error');
    expect(result.current.state.messages[0].text).toBe('Unknown command');
  });

  it('handles log message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'log', msg: 'Something happened' });
    });
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].text).toBe('Something happened');
  });

  it('sets summonActive on Summon burst started', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'log', msg: 'Summon burst started' });
    });
    expect(result.current.state.summonActive).toBe(true);
  });

  it('clears summonActive on Summon burst complete', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'log', msg: 'Summon burst started' });
    });
    act(() => {
      result.current.handleMessage({ t: 'log', msg: 'Summon burst complete' });
    });
    expect(result.current.state.summonActive).toBe(false);
  });

  it('handles pong message', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'pong' });
    });
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].text).toBe('Pong received');
  });

  it('ignores unknown message type', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'unknown_type' });
    });
    expect(result.current.state.messages).toEqual([]);
  });

  it('addMessage works', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.addMessage('info', 'Manual message');
    });
    expect(result.current.state.messages).toHaveLength(1);
    expect(result.current.state.messages[0].text).toBe('Manual message');
  });

  it('clearFrames resets frames', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'frame', id: 1, dir: 'rx', dlc: 8, d: 'AA' });
    });
    act(() => {
      result.current.clearFrames();
    });
    expect(result.current.state.frames).toEqual([]);
    expect(result.current.state.frameCount).toBe(0);
  });

  it('clearMessages resets messages', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.addMessage('info', 'test');
    });
    act(() => {
      result.current.clearMessages();
    });
    expect(result.current.state.messages).toEqual([]);
  });

  it('reset returns to initial state', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      result.current.handleMessage({ t: 'boot', variant: 'hw3', hw: 'ESP32' });
      result.current.handleMessage({ t: 'frame', id: 1, dir: 'rx', dlc: 8, d: 'AA' });
    });
    act(() => {
      result.current.reset();
    });
    expect(result.current.state.variant).toBe('hw4');
    expect(result.current.state.frames).toEqual([]);
    expect(result.current.state.messages).toEqual([]);
  });

  it('caps messages at 100', () => {
    const { result } = renderHook(() => useBoardState());
    act(() => {
      for (let i = 0; i < 110; i++) {
        result.current.addMessage('info', `msg ${i}`);
      }
    });
    expect(result.current.state.messages).toHaveLength(100);
  });
});
