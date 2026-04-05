/**
 * BLE Transport — connects to HC-05 Bluetooth module via react-native-ble-plx.
 *
 * HC-05 exposes SPP-over-BLE with a standard serial service.
 * Service UUID: 00001101-0000-1000-8000-00805F9B34FB (SPP)
 * We scan for devices named "HC-05" or user-selected device.
 */

import { BleManager, Device, Subscription } from 'react-native-ble-plx';
import type { Transport, TransportEvents, ScannedDevice } from './types';

const SPP_SERVICE = '00001101-0000-1000-8000-00805f9b34fb';
const SPP_CHAR_RX = '00002a19-0000-1000-8000-00805f9b34fb';
const SPP_CHAR_TX = '00002a20-0000-1000-8000-00805f9b34fb';

// Many HC-05 modules use FFE0/FFE1 for serial passthrough
const HM_SERVICE = '0000ffe0-0000-1000-8000-00805f9b34fb';
const HM_CHAR = '0000ffe1-0000-1000-8000-00805f9b34fb';

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

    // Try HM-10/HC-05 FFE service first, fall back to SPP
    let charUUID = HM_CHAR;
    let serviceUUID = HM_SERVICE;

    const services = await device.services();
    const serviceIds = services.map(s => s.uuid.toLowerCase());

    if (!serviceIds.includes(HM_SERVICE)) {
      serviceUUID = SPP_SERVICE;
      charUUID = SPP_CHAR_RX;
    }

    this.subscription = device.monitorCharacteristicForService(
      serviceUUID,
      charUUID,
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
    if (!this.device || !this._connected) return;

    const services = await this.device.services();
    const serviceIds = services.map(s => s.uuid.toLowerCase());
    const serviceUUID = serviceIds.includes(HM_SERVICE) ? HM_SERVICE : SPP_SERVICE;
    const charUUID = serviceIds.includes(HM_SERVICE) ? HM_CHAR : SPP_CHAR_TX;

    const encoded = btoa(command + '\n');
    await this.device.writeCharacteristicWithResponseForService(
      serviceUUID,
      charUUID,
      encoded,
    );
  }
}
