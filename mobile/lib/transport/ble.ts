/**
 * BLE Transport — connects to Arduino (HC-05/HM-10) or ESP32 (NUS) via react-native-ble-plx.
 *
 * Supported BLE services (auto-detected):
 *  - Nordic UART Service (NUS):  ESP32 native BLE
 *  - HM-10/HC-05 FFE:           Arduino HC-05 / HM-10 modules
 *  - SPP-over-BLE:              Classic HC-05 SPP bridge
 */

import { BleManager, Device, Subscription } from 'react-native-ble-plx';
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
    });
  });

  const timer = setTimeout(() => ble.stopDeviceScan(), durationMs);

  return () => {
    clearTimeout(timer);
    ble.stopDeviceScan();
  };
}

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

    // Auto-detect BLE service: NUS (ESP32), FFE (HM-10/HC-05), or SPP
    const services = await device.services();
    const serviceIds = services.map(s => s.uuid.toLowerCase());
    this.serviceMapping = resolveService(serviceIds);

    this.subscription = device.monitorCharacteristicForService(
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

    device.onDisconnected(() => {
      this._connected = false;
      this.listeners?.onDisconnect();
    });
  }

  async disconnect(): Promise<void> {
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
