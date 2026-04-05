import { useState, useCallback, useRef } from 'react';

export function useSerial() {
  const [connected, setConnected] = useState(false);
  const [transport, setTransport] = useState(null);
  const portRef = useRef(null);
  const readerRef = useRef(null);
  const writerRef = useRef(null);
  const onMessageRef = useRef(null);

  const connect = useCallback(async (type = 'usb') => {
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
            console.error('Read error:', err);
          }
        })();
      }
    } catch (err) {
      console.error('Connection failed:', err);
      throw err;
    }
  }, []);

  const disconnect = useCallback(async () => {
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
  }, []);

  const send = useCallback(async (command) => {
    if (!writerRef.current) return;
    
    const encoder = new TextEncoder();
    await writerRef.current.write(encoder.encode(command + '\n'));
  }, []);

  const setOnMessage = useCallback((callback) => {
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
  };
}
