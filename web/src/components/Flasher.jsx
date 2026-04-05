import { useState } from 'react';

const VARIANTS = [
  {
    id: 'uno_usb',
    name: 'USB Only',
    env: 'uno_usb',
    bt: false,
    dual: false,
    desc: 'Lightest firmware. USB serial control only.',
    flags: 'BOARD_ENABLE_BT=0, BOARD_ENABLE_MCP2515_2=0'
  },
  {
    id: 'uno_usb_bt',
    name: 'USB + Bluetooth',
    env: 'uno_usb_bt',
    bt: true,
    dual: false,
    desc: 'Adds HC-05 Bluetooth support for wireless control.',
    flags: 'BOARD_ENABLE_BT=1, BOARD_ENABLE_MCP2515_2=0'
  },
  {
    id: 'uno_usb_mcp2',
    name: 'USB + Dual CAN',
    env: 'uno_usb_mcp2',
    bt: false,
    dual: true,
    desc: 'Adds second MCP2515 for monitoring two CAN buses.',
    flags: 'BOARD_ENABLE_BT=0, BOARD_ENABLE_MCP2515_2=1'
  },
  {
    id: 'uno_full',
    name: 'Full (USB + BT + Dual CAN)',
    env: 'uno_full',
    bt: true,
    dual: true,
    desc: 'Everything enabled. Bluetooth + dual MCP2515.',
    flags: 'BOARD_ENABLE_BT=1, BOARD_ENABLE_MCP2515_2=1'
  }
];

export default function Flasher() {
  const [selected, setSelected] = useState(null);
  const [status, setStatus] = useState('idle'); // idle | connecting | flashing | done | error
  const [message, setMessage] = useState('');

  async function handleFlash() {
    if (!selected) return;
    const variant = VARIANTS.find(v => v.id === selected);
    if (!variant) return;

    setStatus('connecting');
    setMessage('Requesting serial port...');

    try {
      if (!navigator.serial) {
        throw new Error('Web Serial API not supported. Use Chrome or Edge.');
      }

      const port = await navigator.serial.requestPort();
      await port.open({ baudRate: 115200 });

      setStatus('flashing');
      setMessage(`Flashing ${variant.name} (env: ${variant.env})...`);

      // Send flash command via serial
      const encoder = new TextEncoder();
      const writer = port.writable.getWriter();
      const cmd = JSON.stringify({ cmd: 'flash', env: variant.env }) + '\n';
      await writer.write(encoder.encode(cmd));
      writer.releaseLock();

      // Read response
      const decoder = new TextDecoder();
      const reader = port.readable.getReader();
      let result = '';
      const timeout = setTimeout(() => {
        reader.cancel();
      }, 30000);

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

      if (result.includes('ERROR')) {
        throw new Error(result);
      }

      setStatus('done');
      setMessage(`${variant.name} flashed successfully.`);
    } catch (err) {
      setStatus('error');
      setMessage(err.message || 'Flash failed.');
    }
  }

  function handleManualDownload() {
    if (!selected) return;
    const variant = VARIANTS.find(v => v.id === selected);
    if (!variant) return;
    // Open PlatformIO docs / build guide
    setMessage(`To flash manually: pio run -e ${variant.env} -t upload`);
    setStatus('idle');
  }

  return (
    <div className="flasher">
      <div className="panel">
        <div className="panel-header"><h2>Select Firmware</h2></div>
        <div className="panel-body">
          <div className="flasher-grid">
            {VARIANTS.map(v => (
              <button
                key={v.id}
                className={`flasher-card ${selected === v.id ? 'selected' : ''}`}
                onClick={() => { setSelected(v.id); setStatus('idle'); setMessage(''); }}
              >
                <div className="flasher-card-name">{v.name}</div>
                <div className="flasher-card-badges">
                  <span className={`badge ${v.bt ? 'badge-on' : 'badge-off'}`}>BT</span>
                  <span className={`badge ${v.dual ? 'badge-on' : 'badge-off'}`}>Dual CAN</span>
                </div>
                <div className="flasher-card-desc">{v.desc}</div>
                <div className="flasher-card-flags"><code>{v.flags}</code></div>
              </button>
            ))}
          </div>
        </div>
      </div>

      {selected && (
        <div className="panel">
          <div className="panel-header"><h2>Flash</h2></div>
          <div className="panel-body flasher-actions">
            <div className="flasher-selected">
              <strong>Selected:</strong> {VARIANTS.find(v => v.id === selected)?.name}
            </div>
            <div className="flasher-buttons">
              <button
                className="btn btn-primary"
                onClick={handleFlash}
                disabled={status === 'connecting' || status === 'flashing'}
              >
                {status === 'connecting' ? 'Connecting...' :
                 status === 'flashing' ? 'Flashing...' :
                 'Flash via USB'}
              </button>
              <button className="btn btn-secondary" onClick={handleManualDownload}>
                Manual (PlatformIO CLI)
              </button>
            </div>
            {message && (
              <div className={`flasher-msg ${status === 'error' ? 'flasher-error' : status === 'done' ? 'flasher-success' : ''}`}>
                {message}
              </div>
            )}
          </div>
        </div>
      )}

      <div className="panel">
        <div className="panel-header"><h2>Firmware Details</h2></div>
        <div className="panel-body">
          <table className="wiring-table">
            <thead>
              <tr>
                <th>Feature</th>
                <th>USB Only</th>
                <th>USB+BT</th>
                <th>USB+Dual</th>
                <th>Full</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td>USB Serial</td>
                <td>✓</td><td>✓</td><td>✓</td><td>✓</td>
              </tr>
              <tr>
                <td>Bluetooth (HC-05)</td>
                <td>—</td><td>✓</td><td>—</td><td>✓</td>
              </tr>
              <tr>
                <td>MCP2515 #1</td>
                <td>✓</td><td>✓</td><td>✓</td><td>✓</td>
              </tr>
              <tr>
                <td>MCP2515 #2</td>
                <td>—</td><td>—</td><td>✓</td><td>✓</td>
              </tr>
              <tr>
                <td>FSD / Nag / Profile</td>
                <td>✓</td><td>✓</td><td>✓</td><td>✓</td>
              </tr>
              <tr>
                <td>Vehicle Commands</td>
                <td>✓</td><td>✓</td><td>✓</td><td>✓</td>
              </tr>
              <tr>
                <td>Flash Size</td>
                <td>Smallest</td><td>Medium</td><td>Medium</td><td>Largest</td>
              </tr>
            </tbody>
          </table>
          <p className="note">All variants share the same CAN protocol and feature logic. Only I/O capabilities differ.</p>
        </div>
      </div>

      <div className="panel">
        <div className="panel-header"><h2>PlatformIO CLI</h2></div>
        <div className="panel-body">
          <p>To build and flash from terminal:</p>
          <pre className="code-block">{`# Install PlatformIO CLI
pip install platformio

# Build specific variant
cd hardware
pio run -e uno_usb          # USB only
pio run -e uno_usb_bt       # USB + Bluetooth
pio run -e uno_usb_mcp2     # USB + Dual CAN
pio run -e uno_full          # Full

# Flash (connect Arduino via USB first)
pio run -e <env_name> -t upload`}</pre>
        </div>
      </div>
    </div>
  );
}
