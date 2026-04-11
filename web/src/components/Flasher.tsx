import { useState } from 'react';
import './Flasher.css';

// ── Connectivity options per board ───────────────────────────────────────────

interface ConnOption {
  key: string;
  label: string;
  desc: string;
  locked?: boolean;
}

const CONN_ARDUINO: ConnOption[] = [
  { key: 'serial', label: 'USB Serial', desc: 'Always on', locked: true },
  { key: 'bt',     label: 'HC-05 Bluetooth', desc: 'HC-05 module' },
];

const CONN_ESP32: ConnOption[] = [
  { key: 'serial', label: 'USB Serial', desc: 'Always on', locked: true },
  { key: 'wifi',   label: 'WiFi',       desc: 'REST API + OTA' },
  { key: 'ble',    label: 'BLE',        desc: 'NimBLE GATT' },
];

function resolveEnv(board: 'arduino' | 'esp32', conn: Record<string, number>): string {
  if (board === 'arduino') {
    return conn.bt ? 'uno_bt' : 'uno';
  }
  const w = conn.wifi ? 1 : 0;
  const b = conn.ble ? 1 : 0;
  if (w && b) return 'esp32_wifi_ble';
  if (w)      return 'esp32_wifi';
  if (b)      return 'esp32_ble';
  return 'esp32';
}

const BUS_OPTIONS = [
  { key: 'fsd',     label: 'FSD',     desc: 'X179 pins 13-14', default: true },
  { key: 'vehicle', label: 'Vehicle', desc: 'X179 pins 9-10',  default: false },
  { key: 'body',    label: 'Body',    desc: 'X179 pins 2-3',   default: false },
];

export default function Flasher() {
  const [board, setBoard] = useState<'arduino' | 'esp32'>('arduino');
  const [conn, setConn] = useState<Record<string, number>>({ serial: 1, bt: 0, wifi: 0, ble: 0 });
  const [buses, setBuses] = useState<Record<string, number>>({ fsd: 1, vehicle: 0, body: 0 });
  const [status, setStatus] = useState<string>('idle');
  const [message, setMessage] = useState('');

  const connOpts = board === 'arduino' ? CONN_ARDUINO : CONN_ESP32;
  const env = resolveEnv(board, conn);
  const ext = board === 'arduino' ? '.hex' : '.bin';

  function switchBoard(b: 'arduino' | 'esp32') {
    setBoard(b);
    setConn({ serial: 1, bt: 0, wifi: 0, ble: 0 });
    setBuses({ fsd: 1, vehicle: 0, body: 0 });
    setStatus('idle');
    setMessage('');
  }

  function toggleConn(key: string) {
    if (key === 'serial') return; // Serial always on
    setConn(prev => ({ ...prev, [key]: prev[key] ? 0 : 1 }));
  }

  function toggleBus(key: string) {
    if (key === 'fsd') return; // FSD always on
    setBuses(prev => ({ ...prev, [key]: prev[key] ? 0 : 1 }));
  }

  async function handleBuild() {
    setStatus('building');
    setMessage('Requesting build from server...');

    try {
      const res = await fetch('/api/build', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ env, ...buses }),
      });

      if (!res.ok) {
        const err = await res.json().catch(() => ({ error: 'Build failed' }));
        throw new Error(err.error || `HTTP ${res.status}`);
      }

      const blob = await res.blob();
      const parts = [env];
      if (buses.vehicle) parts.push('vehicle');
      if (buses.body) parts.push('body');
      const filename = `${parts.join('_')}${ext}`;

      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = filename;
      a.click();
      URL.revokeObjectURL(url);

      setStatus('done');
      setMessage(`${filename} built and downloaded.`);
    } catch (err: unknown) {
      setStatus('error');
      setMessage(err instanceof Error ? err.message : 'Build failed.');
    }
  }

  async function handleFlash() {
    setStatus('connecting');
    setMessage('Requesting serial port...');

    try {
      if (!navigator.serial) {
        throw new Error('Web Serial API not supported. Use Chrome or Edge.');
      }

      const port = await navigator.serial.requestPort();
      await port.open({ baudRate: 115200 });

      setStatus('flashing');
      setMessage(`Flashing ${env}...`);

      const encoder = new TextEncoder();
      const writer = port.writable.getWriter();
      const cmd = JSON.stringify({ cmd: 'flash', env }) + '\n';
      await writer.write(encoder.encode(cmd));
      writer.releaseLock();

      const decoder = new TextDecoder();
      const reader = port.readable.getReader();
      let result = '';
      const timeout = setTimeout(() => { reader.cancel(); }, 30000);

      try {
        while (true) {
          const { value, done } = await reader.read();
          if (done) break;
          result += decoder.decode(value, { stream: true });
          if (result.includes('OK') || result.includes('ERROR')) break;
        }
      } finally {
        clearTimeout(timeout);
        reader.releaseLock();
      }

      await port.close();
      if (result.includes('ERROR')) throw new Error(result);

      setStatus('done');
      setMessage(`${env} flashed successfully.`);
    } catch (err: unknown) {
      setStatus('error');
      setMessage(err instanceof Error ? err.message : 'Flash failed.');
    }
  }

  return (
    <div className="fl-root">
      {/* Board Selector */}
      <div className="sg-board-select">
        <button
          className={`sg-board-btn ${board === 'arduino' ? 'sg-board-btn--active' : ''}`}
          onClick={() => switchBoard('arduino')}
        >
          <div className="sg-board-btn__icon">⬡</div>
          <div className="sg-board-btn__info">
            <strong>Arduino Uno R3</strong>
            <span>MCP2515 × 1-3 · HC-05 BT · EEPROM</span>
          </div>
        </button>
        <button
          className={`sg-board-btn ${board === 'esp32' ? 'sg-board-btn--active' : ''}`}
          onClick={() => switchBoard('esp32')}
        >
          <div className="sg-board-btn__icon">◈</div>
          <div className="sg-board-btn__info">
            <strong>ESP32-S DevKit</strong>
            <span>MCP2515 × 1-3 · WiFi · BLE · NVS</span>
          </div>
        </button>
      </div>

      {/* Connectivity */}
      <section className="fl-section">
        <div className="fl-section__head">
          <span className="fl-section__icon">📡</span>
          <h2>Connectivity</h2>
        </div>
        <div className="fl-section__body">
          <div className="fl-conn-grid">
            {connOpts.map(c => (
              <button
                key={c.key}
                className={`fl-conn-btn ${conn[c.key] ? 'fl-conn-btn--active' : ''} ${c.locked ? 'fl-conn-btn--locked' : ''}`}
                onClick={() => toggleConn(c.key)}
                title={c.locked ? 'USB Serial is always active' : `Toggle ${c.label}`}
              >
                <strong>{c.label}</strong>
                <span>{c.desc}</span>
                <span className="fl-conn-btn__status">{conn[c.key] ? 'ON' : 'OFF'}</span>
              </button>
            ))}
          </div>
          <div className="fl-resolved">
            Resolved environment: <code>{env}</code>
          </div>
        </div>
      </section>

      {/* Bus Selection */}
      <section className="fl-section">
        <div className="fl-section__head">
          <span className="fl-section__icon">🔌</span>
          <h2>CAN Buses</h2>
          <span className="fl-section__count">X179 Connector</span>
        </div>
        <div className="fl-section__body">
          <div className="fl-bus-grid">
            {BUS_OPTIONS.map(bus => (
              <button
                key={bus.key}
                className={`fl-bus-btn ${buses[bus.key] ? 'fl-bus-btn--active' : ''} ${bus.default ? 'fl-bus-btn--locked' : ''}`}
                onClick={() => toggleBus(bus.key)}
                title={bus.default ? 'FSD bus is always active' : `Toggle ${bus.label} bus`}
              >
                <strong>{bus.label}</strong>
                <span>{bus.desc}</span>
                <span className="fl-bus-btn__status">{buses[bus.key] ? 'ON' : 'OFF'}</span>
              </button>
            ))}
          </div>
        </div>
      </section>

      {/* Build Actions */}
      <section className="fl-section">
        <div className="fl-section__head">
          <span className="fl-section__icon">⚡</span>
          <h2>Build & Flash</h2>
        </div>
        <div className="fl-section__body">
          <div className="fl-flash-bar">
            <div className="fl-flash-bar__info">
              <strong>{env}</strong>
              <span className="fl-flash-bar__buses">
                {BUS_OPTIONS.filter(b => buses[b.key]).map(b => b.label).join(' + ')}
              </span>
            </div>
            <div className="fl-flash-bar__actions">
              <button
                className="btn btn-primary"
                onClick={handleBuild}
                disabled={status === 'building'}
              >
                {status === 'building' ? 'Building...' : 'Build & Download'}
              </button>
              <button
                className="btn"
                onClick={handleFlash}
                disabled={status === 'connecting' || status === 'flashing' || status === 'building'}
              >
                {status === 'connecting' ? 'Connecting...' :
                 status === 'flashing' ? 'Flashing...' :
                 'Flash via USB'}
              </button>
            </div>
          </div>
          {message && (
            <div className={`fl-msg fl-msg--${status}`}>{message}</div>
          )}
        </div>
      </section>

      {/* CLI Reference */}
      <section className="fl-section">
        <div className="fl-section__head">
          <span className="fl-section__icon">💻</span>
          <h2>PlatformIO CLI</h2>
        </div>
        <div className="fl-section__body">
          <pre className="fl-code">{`# Build with bus flags
cd hardware
PLATFORMIO_BUILD_FLAGS="-DBUS_FSD_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=0" pio run -e esp32_wifi

# Flash via USB
pio run -e esp32_wifi -t upload`}</pre>
        </div>
      </section>
    </div>
  );
}
