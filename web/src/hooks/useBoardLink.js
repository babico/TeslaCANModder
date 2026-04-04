import { startTransition, useCallback, useEffect, useRef, useState } from 'react';
import { SerialBoardClient } from '../lib/board/client';
import { BoardCommands, BOARD_COMMANDS } from '../lib/board/commands';
import { getBoardBrowserCapabilities } from '../lib/board/capabilities';
import {
  MAX_CONSOLE_LINES,
  createBaseTelemetry,
  createFrameMonitorState,
  formatClockTime,
  ingestFrameMonitorBatch,
  normalizeStatusMessage,
  trimList,
} from '../lib/board/protocol';
import { buildInitialPackageState, derivePackageState } from '../lib/can/core/packages';
import { toolkitPackages } from '../packages';

// Commands debounced at 150ms to protect slow HC-05 links (9600 baud).
// Instant commands (ping, status, stream, variant) bypass the queue.
const DEBOUNCED_COMMAND_SET = new Set([
  BOARD_COMMANDS.fsdOn,
  BOARD_COMMANDS.fsdOff,
  BOARD_COMMANDS.fsdToggle,
  BOARD_COMMANDS.isaChimeOn,
  BOARD_COMMANDS.isaChimeOff,
]);
const DEBOUNCED_PREFIXES = ['profile:', 'offset:', 'isa-chime:'];

function isCommandDebounced(command) {
  if (DEBOUNCED_COMMAND_SET.has(command)) return true;
  return DEBOUNCED_PREFIXES.some((prefix) => command.startsWith(prefix));
}

export function useBoardLink() {
  const MONITOR_FLUSH_INTERVAL_MS = 120;
  const MONITOR_FLUSH_FRAME_THRESHOLD = 24;
  const [capabilities] = useState(() => getBoardBrowserCapabilities());
  const [isConnected, setIsConnected] = useState(false);
  const [isStreaming, setIsStreaming] = useState(false);
  const [status, setStatus] = useState('Not Connected');
  const [activeTransport, setActiveTransport] = useState(null);
  const [telemetry, setTelemetry] = useState(createBaseTelemetry);
  const [monitor, setMonitor] = useState(createFrameMonitorState);
  const [packages, setPackages] = useState(() => buildInitialPackageState(toolkitPackages));
  const [consoleLines, setConsoleLines] = useState([]);
  const [deviceInfo, setDeviceInfo] = useState(null);
  const [client] = useState(() => new SerialBoardClient());

  const messageWindowRef = useRef([]);
  const pendingMonitorRef = useRef({ frames: [], parseErrors: 0 });
  const monitorFlushTimerRef = useRef(null);
  const commandDebounceRef = useRef(null);

  const appendConsole = useCallback((type, text) => {
    const line = {
      id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
      type,
      text,
      timestamp: formatClockTime(new Date()),
    };

    setConsoleLines((previous) => {
      const next = [line, ...previous];
      trimList(next, MAX_CONSOLE_LINES);
      return next;
    });
  }, []);

  const resetBoardState = useCallback(() => {
    // Connection state is reset as one unit so reconnects always start from a
    // clean transport session instead of reusing stale telemetry or package
    // state from a previous board connection.
    if (typeof window !== 'undefined' && monitorFlushTimerRef.current !== null) {
      window.clearTimeout(monitorFlushTimerRef.current);
    }
    monitorFlushTimerRef.current = null;
    if (commandDebounceRef.current !== null) {
      clearTimeout(commandDebounceRef.current);
      commandDebounceRef.current = null;
    }
    pendingMonitorRef.current = { frames: [], parseErrors: 0 };
    messageWindowRef.current = [];
    setIsConnected(false);
    setIsStreaming(false);
    setStatus('Not Connected');
    setActiveTransport(null);
    setTelemetry(createBaseTelemetry());
    setMonitor(createFrameMonitorState());
    setPackages(buildInitialPackageState(toolkitPackages));
    setDeviceInfo(null);
  }, []);

  const refreshStatusRate = useCallback((updater) => {
    const now = Date.now();
    messageWindowRef.current = messageWindowRef.current.filter((timestamp) => now - timestamp < 2000);
    const rate = Math.round(messageWindowRef.current.length / 2);
    updater(rate);
  }, []);

  const flushPendingMonitor = useCallback(() => {
    const pending = pendingMonitorRef.current;
    if (!pending.frames.length && !pending.parseErrors) {
      monitorFlushTimerRef.current = null;
      return;
    }

    pendingMonitorRef.current = { frames: [], parseErrors: 0 };
    monitorFlushTimerRef.current = null;

    startTransition(() => {
      setMonitor((previous) => ingestFrameMonitorBatch(previous, pending.frames, pending.parseErrors));
    });
  }, []);

  const schedulePendingMonitorFlush = useCallback(() => {
    if (monitorFlushTimerRef.current !== null) {
      return;
    }

    if (typeof window === 'undefined') {
      flushPendingMonitor();
      return;
    }

    monitorFlushTimerRef.current = window.setTimeout(flushPendingMonitor, MONITOR_FLUSH_INTERVAL_MS);
  }, [flushPendingMonitor, MONITOR_FLUSH_INTERVAL_MS]);

  const enqueueMonitorFrame = useCallback((message) => {
    pendingMonitorRef.current.frames.push(message);

    if (pendingMonitorRef.current.frames.length >= MONITOR_FLUSH_FRAME_THRESHOLD) {
      if (typeof window !== 'undefined' && monitorFlushTimerRef.current !== null) {
        window.clearTimeout(monitorFlushTimerRef.current);
      }
      monitorFlushTimerRef.current = null;
      flushPendingMonitor();
      return;
    }

    schedulePendingMonitorFlush();
  }, [flushPendingMonitor, schedulePendingMonitorFlush, MONITOR_FLUSH_FRAME_THRESHOLD]);

  const enqueueMonitorParseError = useCallback(() => {
    pendingMonitorRef.current.parseErrors += 1;
    schedulePendingMonitorFlush();
  }, [schedulePendingMonitorFlush]);

  const handleMessage = useCallback((message) => {
    // The browser receives one mixed message stream from the board. This
    // router keeps boot/status/frame/log handling explicit and normalizes only
    // the message shapes that the UI renders directly.
    messageWindowRef.current.push(Date.now());

    if (message.t === 'boot') {
      setDeviceInfo(message);
      appendConsole('info', `Board connected: ${message.hw} + ${message.can}${message.bt ? ` + ${message.bt}` : ''}`);
      return;
    }

    if (message.t === 'log') {
      appendConsole('info', message.msg || 'Log message received.');
      return;
    }

    if (message.t === 'pong') {
      appendConsole('info', 'Pong received from board.');
      return;
    }

    if (message.t === 'ack') {
      appendConsole('info', `Command acknowledged: ${message.cmd}`);
      return;
    }

    if (message.t === 'error') {
      appendConsole('error', message.msg || 'Board returned an error.');
      return;
    }

    if (message.t === 'status') {
      refreshStatusRate((rate) => {
        const normalizedStatus = normalizeStatusMessage(message, rate);
        setTelemetry(normalizedStatus);
        setIsStreaming(normalizedStatus.streamEnabled);
        // Packages derive from the canonical board status payload instead of
        // keeping their own transport subscriptions.
        setPackages(derivePackageState(toolkitPackages, message));
        setDeviceInfo((previous) => ({
          ...(previous || {}),
          hw: message.hw || previous?.hw || null,
          can: message.can || previous?.can || null,
          drv: message.drv || previous?.drv || null,
          variant: message.variant || previous?.variant || null,
          cap: message.cap || previous?.cap || null,
          ready: message.ready || previous?.ready || null,
          offset: Number(message.offset) || previous?.offset || 0,
          isaChime: typeof message.isaChime !== 'undefined' ? Boolean(Number(message.isaChime)) : previous?.isaChime || false,
          stream: message.stream || previous?.stream || null,
          features: message.features || previous?.features || null,
          btEnabled: typeof message.bt !== 'undefined' ? Boolean(Number(message.bt)) : previous?.btEnabled || false,
          bt: previous?.bt || null,
        }));
      });
      return;
    }

    if (message.t === 'frame') {
      enqueueMonitorFrame(message);
      return;
    }

    appendConsole('rx', JSON.stringify(message));
  }, [appendConsole, enqueueMonitorFrame, refreshStatusRate]);

  const handleClientOpen = useCallback((kind) => {
    messageWindowRef.current = [];
    setConsoleLines([]);
    setDeviceInfo(null);
    setIsConnected(true);
    setIsStreaming(false);
    setActiveTransport(kind);
    setStatus(kind === 'bluetooth' ? 'Online via HC-05' : 'Online via USB');
    pendingMonitorRef.current = { frames: [], parseErrors: 0 };
    setMonitor(createFrameMonitorState());
  }, []);

  const handleClientClose = useCallback(() => {
    resetBoardState();
  }, [resetBoardState]);

  const handleClientError = useCallback((error) => {
    if (!error) {
      return;
    }

    appendConsole('error', error.message || String(error));
  }, [appendConsole]);

  useEffect(() => {
    // The SerialBoardClient stays imperative and transport-focused. This hook
    // owns React state and injects the state update callbacks once.
    client.setCallbacks({
      onOpen: handleClientOpen,
      onMessage: handleMessage,
      onText: (line) => appendConsole('rx', line),
      onParseError: () => {
        enqueueMonitorParseError();
      },
      onError: handleClientError,
      onClose: handleClientClose,
    });
    return () => {
      if (typeof window !== 'undefined' && monitorFlushTimerRef.current !== null) {
        window.clearTimeout(monitorFlushTimerRef.current);
      }
      monitorFlushTimerRef.current = null;
    };
  }, [appendConsole, client, enqueueMonitorParseError, handleClientClose, handleClientError, handleClientOpen, handleMessage]);

  const connect = useCallback(async (kind = 'usb') => {
    try {
      await client.connect(kind);
      appendConsole('info', `Serial connection opened via ${kind === 'bluetooth' ? 'HC-05' : 'USB'}.`);
    } catch (error) {
      setStatus('Connection Failed');
      appendConsole('error', `Connection failed: ${error.message}`);
    }
  }, [appendConsole, client]);

  const disconnect = useCallback(async () => {
    await client.disconnect();
  }, [client]);

  const sendCommand = useCallback(async (command) => {
    if (!command) {
      return;
    }

    const doSend = async () => {
      try {
        await client.send(command);
        appendConsole('tx', command);

        // Streaming is toggled optimistically so the UI reacts immediately; the
        // following status/ack traffic will keep the hook state honest.
        if (command === BOARD_COMMANDS.streamOn) {
          setIsStreaming(true);
        } else if (command === BOARD_COMMANDS.streamOff) {
          setIsStreaming(false);
        }
      } catch (error) {
        appendConsole('error', `Command failed: ${error.message}`);
      }
    };

    if (isCommandDebounced(command)) {
      if (commandDebounceRef.current !== null) {
        clearTimeout(commandDebounceRef.current);
      }
      commandDebounceRef.current = setTimeout(() => {
        commandDebounceRef.current = null;
        void doSend();
      }, 150);
      return;
    }

    void doSend();
  }, [appendConsole, client]);

  const clearFrames = useCallback(() => {
    if (typeof window !== 'undefined' && monitorFlushTimerRef.current !== null) {
      window.clearTimeout(monitorFlushTimerRef.current);
    }
    monitorFlushTimerRef.current = null;
    pendingMonitorRef.current = { frames: [], parseErrors: 0 };
    setMonitor(createFrameMonitorState());
  }, []);

  const clearConsole = useCallback(() => {
    setConsoleLines([]);
  }, []);

  return {
    connect,
    disconnect,
    sendCommand,
    clearFrames,
    clearConsole,
    isConnected,
    isStreaming,
    status,
    telemetry,
    packages,
    monitor,
    frameCount: monitor.totalReceived,
    consoleLines,
    deviceInfo,
    activeTransport,
    capabilities,
    boardCommands: BoardCommands,
  };
}
