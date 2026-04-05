/**
 * useTransport — unified hook for BLE + USB Serial transport.
 * Auto-selects transport based on platform.
 * Mobile: BLE only. Web: Serial (USB) primary.
 */

import { useState, useCallback, useRef } from 'react';
import { Platform } from 'react-native';
import type { Transport, TransportEvents } from '../lib/transport/types';

export function useTransport() {
  const [connected, setConnected] = useState(false);
  const [transportType, setTransportType] = useState<'ble' | 'serial' | null>(null);
  const [deviceName, setDeviceName] = useState<string | null>(null);
  const transportRef = useRef<Transport | null>(null);
  const onMessageRef = useRef<((msg: Record<string, unknown>) => void) | null>(null);

  const connect = useCallback(async (type: 'ble' | 'serial', deviceId?: string) => {
    let transport: Transport;

    if (type === 'ble') {
      const { BleTransport } = await import('../lib/transport/ble');
      if (!deviceId) throw new Error('Device ID required for BLE connection');
      transport = new BleTransport(deviceId);
    } else {
      const { SerialTransport } = await import('../lib/transport/serial');
      transport = new SerialTransport();
    }

    const events: TransportEvents = {
      onMessage: (msg) => onMessageRef.current?.(msg),
      onDisconnect: () => {
        setConnected(false);
        setTransportType(null);
        setDeviceName(null);
        transportRef.current = null;
      },
      onError: (err) => console.error('Transport error:', err.message),
    };

    transport.setListeners(events);
    await transport.connect();

    transportRef.current = transport;
    setConnected(true);
    setTransportType(type);
    setDeviceName(transport.deviceName);
  }, []);

  const disconnect = useCallback(async () => {
    if (transportRef.current) {
      await transportRef.current.disconnect();
      transportRef.current = null;
    }
    setConnected(false);
    setTransportType(null);
    setDeviceName(null);
  }, []);

  const send = useCallback(async (command: string) => {
    if (transportRef.current?.connected) {
      await transportRef.current.send(command);
    }
  }, []);

  const setOnMessage = useCallback((callback: (msg: Record<string, unknown>) => void) => {
    onMessageRef.current = callback;
  }, []);

  const canUseBle = Platform.OS !== 'web';
  const canUseSerial = Platform.OS === 'web' && typeof navigator !== 'undefined' && 'serial' in navigator;

  return {
    connected,
    transportType,
    deviceName,
    connect,
    disconnect,
    send,
    setOnMessage,
    canUseBle,
    canUseSerial,
  };
}
