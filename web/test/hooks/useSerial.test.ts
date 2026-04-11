import { describe, it, expect, vi, beforeEach } from 'vitest';
import { renderHook, act } from '@testing-library/react';
import { useSerial } from '../../src/hooks/useSerial';

// Mock Web Serial API
const mockPort = {
  open: vi.fn().mockResolvedValue(undefined),
  close: vi.fn().mockResolvedValue(undefined),
  readable: {
    getReader: vi.fn().mockReturnValue({
      read: vi.fn().mockResolvedValue({ value: undefined, done: true }),
      cancel: vi.fn().mockResolvedValue(undefined),
      releaseLock: vi.fn(),
    }),
  },
  writable: {
    getWriter: vi.fn().mockReturnValue({
      write: vi.fn().mockResolvedValue(undefined),
      releaseLock: vi.fn(),
      close: vi.fn().mockResolvedValue(undefined),
    }),
  },
};

beforeEach(() => {
  vi.clearAllMocks();
  Object.defineProperty(navigator, 'serial', {
    value: {
      requestPort: vi.fn().mockResolvedValue(mockPort),
    },
    writable: true,
    configurable: true,
  });
});

describe('useSerial', () => {
  it('starts disconnected', () => {
    const { result } = renderHook(() => useSerial());
    expect(result.current.connected).toBe(false);
    expect(result.current.transport).toBeNull();
  });

  it('reports canUseSerial based on navigator.serial', () => {
    const { result } = renderHook(() => useSerial());
    expect(result.current.canUseSerial).toBe(true);
  });

  it('reports canUseSerial=false when navigator.serial missing', () => {
    // @ts-expect-error — deliberately removing serial for test
    delete (navigator as Record<string, unknown>).serial;
    const { result } = renderHook(() => useSerial());
    expect(result.current.canUseSerial).toBe(false);
  });

  it('connects and sets connected=true', async () => {
    const { result } = renderHook(() => useSerial());
    await act(async () => {
      await result.current.connect('usb');
    });
    expect(result.current.connected).toBe(true);
    expect(result.current.transport).toBe('usb');
    expect(mockPort.open).toHaveBeenCalledWith({ baudRate: 115200 });
  });

  it('disconnects and resets state', async () => {
    const { result } = renderHook(() => useSerial());
    await act(async () => {
      await result.current.connect('usb');
    });
    expect(result.current.connected).toBe(true);

    await act(async () => {
      await result.current.disconnect();
    });
    expect(result.current.connected).toBe(false);
    expect(result.current.transport).toBeNull();
  });

  it('send is a no-op when not connected', async () => {
    const { result } = renderHook(() => useSerial());
    await act(async () => {
      await result.current.send('ping');
    });
    // No error thrown, writer not called
  });
});
