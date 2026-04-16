import { useState, useCallback, useRef } from 'react';

type TransportType = 'usb' | 'bluetooth' | null;

/** Pending command waiting for ack from the board. */
export interface PendingCommand {
  command: string;
  sentAt: number;
}

export interface UseSerialReturn {
  connected: boolean;
  transport: TransportType;
  connect: (type?: string) => Promise<void>;
  disconnect: () => Promise<void>;
  send: (command: string) => Promise<void>;
  setOnMessage: (callback: (msg: Record<string, unknown>) => void) => void;
  canUseSerial: boolean;
  lastError: string | null;
  pendingCommand: PendingCommand | null;
  clearError: () => void;
  ackReceived: (cmd: string) => void;
}

/** Default ack timeout in milliseconds. */
const ACK_TIMEOUT_MS = 5000;

export function useSerial(): UseSerialReturn {
  const [connected, setConnected] = useState(false);
  const [transport, setTransport] = useState<TransportType>(null);
  const [lastError, setLastError] = useState<string | null>(null);
  const [pendingCommand, setPendingCommand] = useState<PendingCommand | null>(null);
  const portRef = useRef<SerialPort | null>(null);
  const readerRef = useRef<ReadableStreamDefaultReader<Uint8Array> | null>(null);
  const writerRef = useRef<WritableStreamDefaultWriter<Uint8Array> | null>(null);
  const onMessageRef = useRef<((msg: Record<string, unknown>) => void) | null>(null);
  const ackTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const clearError = useCallback(() => setLastError(null), []);

  const clearAckTimer = useCallback(() => {
    if (ackTimerRef.current) {
      clearTimeout(ackTimerRef.current);
      ackTimerRef.current = null;
    }
  }, []);

  const ackReceived = useCallback((cmd: string) => {
    setPendingCommand(prev => {
      if (prev && prev.command === cmd) {
        clearAckTimer();
        return null;
      }
      return prev;
    });
  }, [clearAckTimer]);

  const connect = useCallback(async (type = 'usb') => {
    setLastError(null);
    try {
      if (type === 'usb') {
        const port = await navigator.serial.requestPort();
        await port.open({ baudRate: 115200 });
        
        portRef.current = port;
        writerRef.current = port.writable.getWriter();
        
        setConnected(true);
        setTransport('usb');
        
        // Start reading
        const reader = port.readable.getReader();
        readerRef.current = reader;
        
        const decoder = new TextDecoder();
        let buffer = '';
        
        (async () => {
          try {
            while (true) {
              const { value, done } = await reader.read();
              if (done) break;
              
              buffer += decoder.decode(value, { stream: true });
              const lines = buffer.split('\n');
              buffer = lines.pop() || '';
              
              for (const line of lines) {
                const trimmed = line.trim();
                if (trimmed && onMessageRef.current) {
                  try {
                    const msg = JSON.parse(trimmed);
                    onMessageRef.current(msg);
                  } catch {
                    // Invalid JSON, ignore
                  }
                }
              }
            }
          } catch (err) {
            const message = err instanceof Error ? err.message : 'Connection lost';
            setLastError(message);
            setConnected(false);
            setTransport(null);
          }
        })();
      }
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Connection failed';
      setLastError(message);
      throw err;
    }
  }, []);

  const disconnect = useCallback(async () => {
    clearAckTimer();
    setPendingCommand(null);
    try {
      if (readerRef.current) {
        await readerRef.current.cancel();
        readerRef.current = null;
      }
      if (writerRef.current) {
        await writerRef.current.close();
        writerRef.current = null;
      }
      if (portRef.current) {
        await portRef.current.close();
        portRef.current = null;
      }
    } catch (err) {
      console.error('Disconnect error:', err);
    } finally {
      setConnected(false);
      setTransport(null);
    }
  }, [clearAckTimer]);

  const send = useCallback(async (command: string) => {
    if (!writerRef.current) return;
    
    setLastError(null);
    setPendingCommand({ command, sentAt: Date.now() });
    clearAckTimer();

    ackTimerRef.current = setTimeout(() => {
      setPendingCommand(prev => {
        if (prev && prev.command === command) {
          setLastError(`Command "${command}" timed out — no ack received`);
          return null;
        }
        return prev;
      });
    }, ACK_TIMEOUT_MS);

    try {
      const encoder = new TextEncoder();
      await writerRef.current.write(encoder.encode(command + '\n'));
    } catch (err) {
      clearAckTimer();
      setPendingCommand(null);
      const message = err instanceof Error ? err.message : 'Send failed';
      setLastError(message);
    }
  }, [clearAckTimer]);

  const setOnMessage = useCallback((callback: (msg: Record<string, unknown>) => void) => {
    onMessageRef.current = callback;
  }, []);

  return {
    connected,
    transport,
    connect,
    disconnect,
    send,
    setOnMessage,
    canUseSerial: typeof navigator !== 'undefined' && 'serial' in navigator,
    lastError,
    pendingCommand,
    clearError,
    ackReceived,
  };
}
