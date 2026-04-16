/**
 * BLE Transport — connects to Arduino (HC-05/HM-10) or ESP32 (NUS) via react-native-ble-plx.
 *
 * Supported BLE services (auto-detected):
 *  - Nordic UART Service (NUS):  ESP32 native BLE
 *  - HM-10/HC-05 FFE:           Arduino HC-05 / HM-10 modules
 *  - SPP-over-BLE:              Classic HC-05 SPP bridge
 *
 * Features:
 *  - Automatic reconnect with bounded retries on unexpected disconnect
 *  - Permission diagnostics for BLE availability
 */

import { BleManager, Device, State, Subscription } from 'react-native-ble-plx';
import { Platform, PermissionsAndroid } from 'react-native';
import type { Transport, TransportEvents, ScannedDevice } from './types';

// Classic SPP-over-BLE (HC-05)
const SPP_SERVICE = '00001101-0000-1000-8000-00805f9b34fb';
const SPP_CHAR_RX = '00002a19-0000-1000-8000-00805f9b34fb';
const SPP_CHAR_TX = '00002a20-0000-1000-8000-00805f9b34fb';

// HM-10 / HC-05 FFE serial passthrough
const HM_SERVICE = '0000ffe0-0000-1000-8000-00805f9b34fb';
const HM_CHAR = '0000ffe1-0000-1000-8000-00805f9b34fb';

// Nordic UART Service (NUS) — ESP32 native BLE
const NUS_SERVICE = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NUS_CHAR_RX = '6e400002-b5a3-f393-e0a9-e50e24dcca9e'; // write (phone → board)
const NUS_CHAR_TX = '6e400003-b5a3-f393-e0a9-e50e24dcca9e'; // notify (board → phone)

/** Reconnect configuration. */
const MAX_RECONNECT_ATTEMPTS = 3;
const RECONNECT_BASE_DELAY_MS = 1000;

interface BleServiceMapping {
  service: string;
  rxChar: string;  // characteristic for receiving (notify)
  txChar: string;  // characteristic for writing
}

function resolveService(serviceIds: string[]): BleServiceMapping {
  const lower = serviceIds.map(s => s.toLowerCase());
  if (lower.includes(NUS_SERVICE))
    return { service: NUS_SERVICE, rxChar: NUS_CHAR_TX, txChar: NUS_CHAR_RX };
  if (lower.includes(HM_SERVICE))
    return { service: HM_SERVICE, rxChar: HM_CHAR, txChar: HM_CHAR };
  return { service: SPP_SERVICE, rxChar: SPP_CHAR_RX, txChar: SPP_CHAR_TX };
}

let manager: BleManager | null = null;

function getManager(): BleManager {
  if (!manager) manager = new BleManager();
  return manager;
}

// ── Permission Diagnostics ──────────────────────────────────────────────────

export interface BleDiagnostics {
  adapterState: string;
  locationPermission: boolean;
  bluetoothPermission: boolean;
  ready: boolean;
  message: string;
}

/** Check BLE adapter state and OS permissions. Returns a diagnostics snapshot. */
export async function checkBlePermissions(): Promise<BleDiagnostics> {
  const ble = getManager();
  const state = await ble.state();
  const adapterState = state as string;

  let locationPermission = true;
  let bluetoothPermission = true;

  if (Platform.OS === 'android') {
    const apiLevel = Platform.Version;

    if (apiLevel >= 31) {
      // Android 12+ requires BLUETOOTH_SCAN and BLUETOOTH_CONNECT
      const scanResult = await PermissionsAndroid.check(
        'android.permission.BLUETOOTH_SCAN' as any,
      );
      const connectResult = await PermissionsAndroid.check(
        'android.permission.BLUETOOTH_CONNECT' as any,
      );
      bluetoothPermission = scanResult && connectResult;
    }

    // Location still needed for scanning on Android < 12
    if (apiLevel < 31) {
      locationPermission = await PermissionsAndroid.check(
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      );
    }
  }

  const adapterReady = state === State.PoweredOn;
  const ready = adapterReady && locationPermission && bluetoothPermission;

  let message = 'BLE ready';
  if (state === State.PoweredOff) message = 'Bluetooth is turned off. Enable it in Settings.';
  else if (state === State.Unauthorized) message = 'Bluetooth permission denied. Grant access in Settings.';
  else if (state === State.Unsupported) message = 'This device does not support Bluetooth Low Energy.';
  else if (!bluetoothPermission) message = 'Bluetooth scan/connect permission not granted. Check app permissions.';
  else if (!locationPermission) message = 'Location permission required for BLE scanning on this Android version.';
  else if (!adapterReady) message = `Bluetooth adapter state: ${adapterState}. Waiting for it to become ready.`;

  return { adapterState, locationPermission, bluetoothPermission, ready, message };
}

/** Request required BLE permissions (Android). Resolves true if all granted. */
export async function requestBlePermissions(): Promise<boolean> {
  if (Platform.OS !== 'android') return true;

  const apiLevel = Platform.Version;
  const permissions: string[] = [];

  if (apiLevel >= 31) {
    permissions.push(
      'android.permission.BLUETOOTH_SCAN' as any,
      'android.permission.BLUETOOTH_CONNECT' as any,
    );
  } else {
    permissions.push(PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION);
  }

  const results = await PermissionsAndroid.requestMultiple(permissions as any);
  return Object.values(results).every(r => r === PermissionsAndroid.RESULTS.GRANTED);
}

// ── Scanner ─────────────────────────────────────────────────────────────────

export function scanForDevices(
  onFound: (device: ScannedDevice) => void,
  durationMs = 8000,
): () => void {
  const ble = getManager();
  const seen = new Set<string>();

  ble.startDeviceScan(null, { allowDuplicates: false }, (error, device) => {
    if (error || !device) return;
    if (seen.has(device.id)) return;
    seen.add(device.id);
    onFound({
      id: device.id,
      name: device.name || device.localName || null,
      rssi: device.rssi,
      serviceUuids: device.serviceUUIDs ?? undefined,
    });
  });

  const timer = setTimeout(() => ble.stopDeviceScan(), durationMs);

  return () => {
    clearTimeout(timer);
    ble.stopDeviceScan();
  };
}

// ── BLE Transport ───────────────────────────────────────────────────────────

export class BleTransport implements Transport {
  readonly type = 'ble' as const;
  private device: Device | null = null;
  private listeners: TransportEvents | null = null;
  private subscription: Subscription | null = null;
  private buffer = '';
  private _connected = false;
  private _deviceName: string | null = null;
  private targetDeviceId: string;
  private serviceMapping: BleServiceMapping | null = null;
  private intentionalDisconnect = false;
  private reconnectAttempt = 0;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;

  constructor(deviceId: string) {
    this.targetDeviceId = deviceId;
  }

  get connected() { return this._connected; }
  get deviceName() { return this._deviceName; }

  setListeners(events: TransportEvents) {
    this.listeners = events;
  }

  async connect(): Promise<void> {
    const ble = getManager();
    const device = await ble.connectToDevice(this.targetDeviceId, {
      requestMTU: 512,
    });
    await device.discoverAllServicesAndCharacteristics();

    this.device = device;
    this._deviceName = device.name || device.localName || 'BLE Device';
    this._connected = true;
    this.intentionalDisconnect = false;
    this.reconnectAttempt = 0;

    // Auto-detect BLE service: NUS (ESP32), FFE (HM-10/HC-05), or SPP
    const services = await device.services();
    const serviceIds = services.map(s => s.uuid.toLowerCase());
    this.serviceMapping = resolveService(serviceIds);

    this.startMonitoring();

    device.onDisconnected(() => {
      this._connected = false;
      this.subscription?.remove();
      this.subscription = null;

      if (this.intentionalDisconnect) {
        this.listeners?.onDisconnect();
        return;
      }

      // Unexpected disconnect — attempt reconnect
      this.attemptReconnect();
    });
  }

  private startMonitoring(): void {
    if (!this.device || !this.serviceMapping) return;

    this.subscription = this.device.monitorCharacteristicForService(
      this.serviceMapping.service,
      this.serviceMapping.rxChar,
      (error, char) => {
        if (error) {
          this.listeners?.onError(new Error(error.message));
          return;
        }
        if (!char?.value) return;

        const decoded = atob(char.value);
        this.buffer += decoded;
        const lines = this.buffer.split('\n');
        this.buffer = lines.pop() || '';

        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed) continue;
          const start = trimmed.indexOf('{');
          const end = trimmed.lastIndexOf('}');
          if (start >= 0 && end > start) {
            try {
              const msg = JSON.parse(trimmed.slice(start, end + 1));
              this.listeners?.onMessage(msg);
            } catch { /* not valid JSON */ }
          }
        }
      },
    );
  }

  private attemptReconnect(): void {
    if (this.reconnectAttempt >= MAX_RECONNECT_ATTEMPTS) {
      this.listeners?.onError(
        new Error(`Reconnect failed after ${MAX_RECONNECT_ATTEMPTS} attempts`),
      );
      this.listeners?.onDisconnect();
      return;
    }

    this.reconnectAttempt++;
    const delay = RECONNECT_BASE_DELAY_MS * this.reconnectAttempt;

    this.listeners?.onError(
      new Error(`Connection lost. Reconnecting (attempt ${this.reconnectAttempt}/${MAX_RECONNECT_ATTEMPTS})…`),
    );

    this.reconnectTimer = setTimeout(async () => {
      try {
        const ble = getManager();
        const device = await ble.connectToDevice(this.targetDeviceId, {
          requestMTU: 512,
        });
        await device.discoverAllServicesAndCharacteristics();

        this.device = device;
        this._connected = true;
        this.reconnectAttempt = 0;
        this.buffer = '';

        const services = await device.services();
        const serviceIds = services.map(s => s.uuid.toLowerCase());
        this.serviceMapping = resolveService(serviceIds);

        this.startMonitoring();

        device.onDisconnected(() => {
          this._connected = false;
          this.subscription?.remove();
          this.subscription = null;

          if (!this.intentionalDisconnect) {
            this.attemptReconnect();
          } else {
            this.listeners?.onDisconnect();
          }
        });

        // Notify listeners that we're back
        this.listeners?.onMessage({ t: 'log', msg: 'BLE reconnected successfully' });
      } catch {
        this.attemptReconnect();
      }
    }, delay);
  }

  async disconnect(): Promise<void> {
    this.intentionalDisconnect = true;
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    this.subscription?.remove();
    this.subscription = null;
    if (this.device) {
      try {
        await getManager().cancelDeviceConnection(this.device.id);
      } catch { /* already disconnected */ }
    }
    this.device = null;
    this._connected = false;
    this.buffer = '';
    this.reconnectAttempt = 0;
  }

  async send(command: string): Promise<void> {
    if (!this.device || !this._connected || !this.serviceMapping) return;

    const encoded = btoa(command + '\n');
    await this.device.writeCharacteristicWithResponseForService(
      this.serviceMapping.service,
      this.serviceMapping.txChar,
      encoded,
    );
  }
}
