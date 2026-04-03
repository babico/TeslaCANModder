import { useCallback, useEffect, useRef, useState } from 'react';
import { SerialBoardClient } from '../lib/board/client';
import { BoardCommands, BOARD_COMMANDS } from '../lib/board/commands';
import { getBoardBrowserCapabilities } from '../lib/board/capabilities';
import {
  MAX_CONSOLE_LINES,
  MAX_FRAME_BUFFER,
  createBaseTelemetry,
  formatClockTime,
  normalizeFrameMessage,
  normalizeStatusMessage,
  trimList,
} from '../lib/board/protocol';
import { buildInitialPackageState, derivePackageState } from '../lib/can/core/packages';
import { toolkitPackages } from '../packages';

export function useBoardLink() {
  const [capabilities] = useState(() => getBoardBrowserCapabilities());
  const [isConnected, setIsConnected] = useState(false);
  const [isStreaming, setIsStreaming] = useState(false);
  const [status, setStatus] = useState('Not Connected');
  const [activeTransport, setActiveTransport] = useState(null);
  const [telemetry, setTelemetry] = useState(createBaseTelemetry);
  const [packages, setPackages] = useState(() => buildInitialPackageState(toolkitPackages));
  const [frameCount, setFrameCount] = useState(0);
  const [consoleLines, setConsoleLines] = useState([]);
  const [deviceInfo, setDeviceInfo] = useState(null);
  const [client] = useState(() => new SerialBoardClient());

  const frameBufferRef = useRef([]);
  const messageWindowRef = useRef([]);

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
    frameBufferRef.current = [];
    messageWindowRef.current = [];
    setIsConnected(false);
    setIsStreaming(false);
    setStatus('Not Connected');
    setActiveTransport(null);
    setFrameCount(0);
    setTelemetry(createBaseTelemetry());
    setPackages(buildInitialPackageState(toolkitPackages));
    setDeviceInfo(null);
  }, []);

  const refreshStatusRate = useCallback((updater) => {
    const now = Date.now();
    messageWindowRef.current = messageWindowRef.current.filter((timestamp) => now - timestamp < 2000);
    const rate = Math.round(messageWindowRef.current.length / 2);
    updater(rate);
  }, []);

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
        setTelemetry(normalizeStatusMessage(message, rate));
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
          btEnabled: typeof message.bt !== 'undefined' ? Boolean(Number(message.bt)) : previous?.btEnabled || false,
          bt: previous?.bt || null,
        }));
      });
      return;
    }

    if (message.t === 'frame') {
      const frame = normalizeFrameMessage(message);
      frameBufferRef.current = [frame, ...frameBufferRef.current];
      trimList(frameBufferRef.current, MAX_FRAME_BUFFER);
      setFrameCount((count) => count + 1);
      return;
    }

    appendConsole('rx', JSON.stringify(message));
  }, [appendConsole, refreshStatusRate]);

  const handleClientOpen = useCallback((kind) => {
    frameBufferRef.current = [];
    messageWindowRef.current = [];
    setConsoleLines([]);
    setFrameCount(0);
    setDeviceInfo(null);
    setIsConnected(true);
    setIsStreaming(false);
    setActiveTransport(kind);
    setStatus(kind === 'bluetooth' ? 'Online via HC-05' : 'Online via USB');
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
      onError: handleClientError,
      onClose: handleClientClose,
    });
  }, [appendConsole, client, handleClientClose, handleClientError, handleClientOpen, handleMessage]);

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
  }, [appendConsole, client]);

  const clearFrames = useCallback(() => {
    frameBufferRef.current = [];
    setFrameCount(0);
  }, []);

  const clearConsole = useCallback(() => {
    setConsoleLines([]);
  }, []);

  const getFrames = useCallback(() => frameBufferRef.current, []);

  return {
    connect,
    disconnect,
    sendCommand,
    clearFrames,
    clearConsole,
    getFrames,
    isConnected,
    isStreaming,
    status,
    telemetry,
    packages,
    frameCount,
    consoleLines,
    deviceInfo,
    activeTransport,
    capabilities,
    boardCommands: BoardCommands,
  };
}
