import { useState } from 'react';
import './SetupGuidePage.css';

const ARDUINO_ENVS = [
  { env: 'uno', bt: false, label: 'Serial' },
  { env: 'uno_bt', bt: true, label: 'Serial + BT' },
];

const ESP32_ENVS = [
  { env: 'esp32', wifi: false, ble: false, label: 'Serial' },
  { env: 'esp32_wifi', wifi: true, ble: false, label: 'WiFi' },
  { env: 'esp32_ble', wifi: false, ble: true, label: 'BLE' },
  { env: 'esp32_wifi_ble', wifi: true, ble: true, label: 'WiFi + BLE' },
];

const VARIANTS = [
  { id: 'hw4', name: 'HW4', vehicles: '2023+ with HW4 (FSD v14+)', features: ['FSD', 'Nag', 'Profile', 'ISA Chime', 'Summon'] },
  { id: 'hw3', name: 'HW3', vehicles: '2019-2023 with HW3', features: ['FSD', 'Nag', 'Profile', 'Speed Offset', 'Summon'] },
  { id: 'legacy', name: 'Legacy', vehicles: 'Pre-HW3 vehicles', features: ['FSD', 'Nag', 'Profile (0-2)'] },
];

function Badge({ children, variant = 'default' }: { children: React.ReactNode; variant?: string }) {
  return <span className={`sg-badge sg-badge--${variant}`}>{children}</span>;
}

function Tag({ children, active }: { children: React.ReactNode; active?: boolean }) {
  return <span className={`sg-tag ${active ? 'sg-tag--active' : ''}`}>{children}</span>;
}

function WiringTable({ rows, caption }: { rows: Record<string, React.ReactNode>[]; caption?: string }) {
  return (
    <div className="sg-wiring">
      {caption && <div className="sg-wiring__caption">{caption}</div>}
      <table className="sg-table">
        <thead>
          <tr>{Object.keys(rows[0]).map(k => <th key={k}>{k}</th>)}</tr>
        </thead>
        <tbody>
          {rows.map((r, i) => (
            <tr key={i}>{Object.values(r).map((v, j) => <td key={j}>{v as React.ReactNode}</td>)}</tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

function Section({ title, icon, children, id }: { title: string; icon?: string; children: React.ReactNode; id?: string }) {
  return (
    <section className="sg-section" id={id}>
      <div className="sg-section__head">
        {icon && <span className="sg-section__icon">{icon}</span>}
        <h2>{title}</h2>
      </div>
      <div className="sg-section__body">{children}</div>
    </section>
  );
}

function Alert({ variant = 'info', children }: { variant?: 'info' | 'warn' | 'danger' | 'tip'; children: React.ReactNode }) {
  const icons: Record<string, string> = { info: 'ℹ', warn: '⚠', danger: '⛔', tip: '💡' };
  return (
    <div className={`sg-alert sg-alert--${variant}`}>
      <span className="sg-alert__icon">{icons[variant]}</span>
      <div>{children}</div>
    </div>
  );
}

function StepList({ steps }: { steps: string[] }) {
  return (
    <ol className="sg-steps">
      {steps.map((s, i) => (
        <li key={i} className="sg-step">
          <span className="sg-step__num">{i + 1}</span>
          <span className="sg-step__text">{s}</span>
        </li>
      ))}
    </ol>
  );
}

export default function SetupGuidePage() {
  const [board, setBoard] = useState<'arduino' | 'esp32'>('arduino');

  return (
    <div className="page page-setup">
      {/* Hero */}
      <div className="sg-hero">
        <h1>Setup Guide</h1>
        <p>Hardware wiring, firmware selection, and vehicle installation</p>
      </div>

      {/* Board Selector */}
      <div className="sg-board-select">
        <button
          className={`sg-board-btn ${board === 'arduino' ? 'sg-board-btn--active' : ''}`}
          onClick={() => setBoard('arduino')}
        >
          <div className="sg-board-btn__icon">⬡</div>
          <div className="sg-board-btn__info">
            <strong>Arduino Uno R3</strong>
            <span>MCP2515 × 1-3 · HC-05 BT · EEPROM</span>
          </div>
        </button>
        <button
          className={`sg-board-btn ${board === 'esp32' ? 'sg-board-btn--active' : ''}`}
          onClick={() => setBoard('esp32')}
        >
          <div className="sg-board-btn__icon">◈</div>
          <div className="sg-board-btn__info">
            <strong>ESP32-S DevKit</strong>
            <span>MCP2515 × 1-3 · WiFi · BLE · NVS</span>
          </div>
        </button>
      </div>

      <div className="sg-grid">
        {/* Quick Start */}
        <Section title="Quick Start" icon="🚀" id="quick-start">
          {board === 'arduino' ? (
            <StepList steps={[
              'Wire MCP2515 to Arduino Uno (see wiring below)',
              'Go to Flasher tab → select firmware → flash',
              'Go to Dashboard → Connect USB → verify boot message',
              'Select your variant (HW4 / HW3 / Legacy)',
              'Enable features (FSD, Nag, etc.) — all OFF by default',
              'Install in vehicle via X179 connector',
            ]} />
          ) : (
            <StepList steps={[
              'Wire 1-3× MCP2515 modules to ESP32 via SPI',
              'Go to Flasher tab → select ESP32 firmware → flash',
              'Connect to WiFi AP "TeslaCANModder" (password: teslacan123)',
              'Open http://192.168.4.1 for the embedded dashboard',
              'Or connect via BLE using nRF Connect / LightBlue',
              'Select your variant (HW4 / HW3 / Legacy)',
              'Enable features — all OFF by default',
              'Install in vehicle via X179 connector',
            ]} />
          )}
        </Section>

        {/* Required Hardware */}
        <Section title="Required Hardware" icon="🔧" id="hardware">
          <div className="sg-hw-grid">
            {board === 'arduino' ? (
              <>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">Arduino Uno R3</div>
                  <div className="sg-hw-card__desc">CH340 or ATmega16U2 USB. Runs the firmware.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">MCP2515 CAN Module × 1-3</div>
                  <div className="sg-hw-card__desc">8 MHz crystal + TJA1050 transceiver. One per CAN bus.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">9V-36V → 5V Buck Converter</div>
                  <div className="sg-hw-card__desc">3A minimum. Powers Arduino from vehicle 12V rail.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">Tesla X179 Connector</div>
                  <div className="sg-hw-card__desc">Behind center screen. 12V power + CAN bus access.</div>
                </div>
                <div className="sg-hw-card sg-hw-card--optional">
                  <div className="sg-hw-card__name">HC-05 Bluetooth Module <Badge variant="muted">Optional</Badge></div>
                  <div className="sg-hw-card__desc">Wireless serial via SoftwareSerial. 9600 baud. Android only (iOS blocks SPP).</div>
                </div>
              </>
            ) : (
              <>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">ESP32-S DevKit (30 or 38 pin)</div>
                  <div className="sg-hw-card__desc">Built-in WiFi + BLE. No external wireless module needed.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">MCP2515 CAN Module × 1-3</div>
                  <div className="sg-hw-card__desc">8 MHz crystal + TJA1050. All CAN buses use MCP2515 over SPI.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">9V-36V → 5V Buck Converter</div>
                  <div className="sg-hw-card__desc">3A minimum. Powers ESP32 from vehicle 12V via VIN pin.</div>
                </div>
                <div className="sg-hw-card">
                  <div className="sg-hw-card__name">Tesla X179 Connector</div>
                  <div className="sg-hw-card__desc">Behind center screen. 12V power + CAN bus access.</div>
                </div>
              </>
            )}
          </div>
          {board === 'esp32' && (
            <Alert variant="tip">No external Bluetooth module needed. ESP32 has built-in BLE (NimBLE) and WiFi. Both can operate simultaneously.</Alert>
          )}
        </Section>

        {/* Wiring */}
        <Section title={board === 'arduino' ? 'Wiring — MCP2515 → Arduino Uno' : 'Wiring — MCP2515 → ESP32'} icon="🔌" id="wiring">
          {board === 'arduino' ? (
            <>
              {/* Bus 0 */}
              <WiringTable
                caption="MCP2515 #1 — Bus 0 (Required)"
                rows={[
                  { Module: 'VCC', Arduino: '5V', Notes: 'Power' },
                  { Module: 'GND', Arduino: 'GND', Notes: 'Ground' },
                  { Module: 'CS', Arduino: 'D10', Notes: 'SPI chip select' },
                  { Module: 'INT', Arduino: 'D2', Notes: 'Hardware interrupt (INT0)' },
                  { Module: 'SCK', Arduino: 'D13', Notes: 'SPI clock' },
                  { Module: 'MISO', Arduino: 'D12', Notes: 'SPI data out' },
                  { Module: 'MOSI', Arduino: 'D11', Notes: 'SPI data in' },
                  { Module: 'CAN-H', Arduino: '→ X179 pin 13', Notes: 'FSD CAN high' },
                  { Module: 'CAN-L', Arduino: '→ X179 pin 14', Notes: 'FSD CAN low' },
                ]}
              />

              {/* Bus 1 */}
              <WiringTable
                caption="MCP2515 #2 — Bus 1 (Optional, vehicle control)"
                rows={[
                  { Module: 'CS', Arduino: 'D9', Notes: 'Unique chip select' },
                  { Module: 'INT', Arduino: 'D3', Notes: 'Hardware interrupt (INT1)' },
                  { Module: 'SCK/MISO/MOSI', Arduino: 'D13/D12/D11', Notes: 'Shared SPI with #1' },
                  { Module: 'CAN-H', Arduino: '→ X179 pin 9', Notes: 'Vehicle Control CAN high' },
                  { Module: 'CAN-L', Arduino: '→ X179 pin 10', Notes: 'Vehicle Control CAN low' },
                ]}
              />

              {/* Bus 2 */}
              <WiringTable
                caption="MCP2515 #3 — Bus 2 (Optional, body control)"
                rows={[
                  { Module: 'CS', Arduino: 'D8', Notes: 'Unique chip select' },
                  { Module: 'INT', Arduino: 'D6', Notes: 'Polled (no HW interrupt)' },
                  { Module: 'SCK/MISO/MOSI', Arduino: 'D13/D12/D11', Notes: 'Shared SPI with #1/#2' },
                  { Module: 'CAN-H', Arduino: '→ X179 pin 2', Notes: 'Body Control CAN high' },
                  { Module: 'CAN-L', Arduino: '→ X179 pin 3', Notes: 'Body Control CAN low' },
                ]}
              />
              <Alert variant="info">Bus 2 on Uno uses polling (D6 is not a HW interrupt pin). Slightly higher latency but fully functional.</Alert>

              {/* HC-05 */}
              <WiringTable
                caption="HC-05 Bluetooth (Optional)"
                rows={[
                  { Module: 'VCC', Arduino: '5V', Notes: 'Power' },
                  { Module: 'GND', Arduino: 'GND', Notes: 'Ground' },
                  { Module: 'TXD', Arduino: 'D4 (RX)', Notes: 'HC-05 TX → Arduino RX' },
                  { Module: 'RXD', Arduino: 'D5 (TX)', Notes: 'Via voltage divider 5V→3.3V' },
                ]}
              />
              <Alert variant="warn">HC-05 RX is 3.3V logic. Use a 1kΩ + 2kΩ voltage divider on the Arduino D5 → HC-05 RXD line. Baud rate: 9600.</Alert>
            </>
          ) : (
            <>
              <Alert variant="info">All MCP2515 modules share the same SPI bus (SCK=18, MISO=19, MOSI=23). Only CS and INT pins differ per module.</Alert>

              <WiringTable
                caption="MCP2515 #1 — Bus 0: FSD / Autopilot (X179 pins 13-14)"
                rows={[
                  { Module: 'VCC', ESP32: '5V (VIN)', Notes: 'MCP2515 needs 5V' },
                  { Module: 'GND', ESP32: 'GND', Notes: 'Ground' },
                  { Module: 'CS', ESP32: 'GPIO 15', Notes: 'SPI chip select' },
                  { Module: 'INT', ESP32: 'GPIO 34', Notes: 'Input-only, good for interrupt' },
                  { Module: 'SCK', ESP32: 'GPIO 18', Notes: 'VSPI clock' },
                  { Module: 'MISO', ESP32: 'GPIO 19', Notes: 'VSPI data out' },
                  { Module: 'MOSI', ESP32: 'GPIO 23', Notes: 'VSPI data in' },
                ]}
              />

              <WiringTable
                caption="MCP2515 #2 — Bus 1: Vehicle Control (X179 pins 9-10)"
                rows={[
                  { Module: 'CS', ESP32: 'GPIO 27', Notes: 'Unique chip select' },
                  { Module: 'INT', ESP32: 'GPIO 35', Notes: 'Input-only pin' },
                  { Module: 'SCK/MISO/MOSI', ESP32: '18/19/23', Notes: 'Shared SPI with #1' },
                ]}
              />

              <WiringTable
                caption="MCP2515 #3 — Bus 2: Body Control (X179 pins 2-3)"
                rows={[
                  { Module: 'CS', ESP32: 'GPIO 26', Notes: 'Unique chip select' },
                  { Module: 'INT', ESP32: 'GPIO 33', Notes: 'Hardware interrupt' },
                  { Module: 'SCK/MISO/MOSI', ESP32: '18/19/23', Notes: 'Shared SPI with #1/#2' },
                ]}
              />
            </>
          )}
        </Section>

        {/* X179 Connector */}
        <Section title="X179 Vehicle Connection" icon="🚗" id="x179">
          <div className="sg-x179-layout">
            <img
              src="/reference/X179_Connector_Pinout_Colored.png"
              alt="Tesla X179 connector pinout"
              className="sg-x179-img"
            />
            <WiringTable rows={[
              { Pin: '1', Connect: 'Buck VIN+', Purpose: '12V power' },
              { Pin: '20', Connect: 'Buck VIN−', Purpose: 'Ground' },
              { Pin: '13', Connect: 'MCP2515 #1 CAN-H', Purpose: 'FSD / Autopilot CAN' },
              { Pin: '14', Connect: 'MCP2515 #1 CAN-L', Purpose: 'FSD / Autopilot CAN' },
              { Pin: '9', Connect: 'MCP2515 #2 CAN-H', Purpose: 'Vehicle Control CAN' },
              { Pin: '10', Connect: 'MCP2515 #2 CAN-L', Purpose: 'Vehicle Control CAN' },
              { Pin: '2', Connect: 'MCP2515 #3 CAN-H', Purpose: 'Body Control CAN' },
              { Pin: '3', Connect: 'MCP2515 #3 CAN-L', Purpose: 'Body Control CAN' },
            ]} />
          </div>
          <Alert variant="info">
            Buck converter 5V output → {board === 'arduino' ? 'Arduino USB port or 5V pin' : 'ESP32 VIN pin'}.
            Connect only the CAN buses you need (1-CAN = just pins 13/14).
          </Alert>
        </Section>

        {/* Firmware Variants */}
        <Section title="Firmware Variants" icon="📦" id="firmware">
          {board === 'arduino' ? (
            <div className="sg-fw-grid">
              {ARDUINO_ENVS.map(e => (
                <div key={e.env} className="sg-fw-card">
                  <div className="sg-fw-card__head">
                    <code className="sg-fw-card__env">{e.env}</code>
                  </div>
                  <div className="sg-fw-card__tags">
                    <Tag active>3 CAN</Tag>
                    <Tag active={e.bt}>{e.bt ? 'HC-05 BT' : 'No BT'}</Tag>
                    <Tag active>USB</Tag>
                  </div>
                </div>
              ))}
            </div>
          ) : (
            <div className="sg-fw-grid">
              {ESP32_ENVS.map(e => (
                <div key={e.env} className="sg-fw-card">
                  <div className="sg-fw-card__head">
                    <code className="sg-fw-card__env">{e.env}</code>
                  </div>
                  <div className="sg-fw-card__tags">
                    <Tag active>3 CAN</Tag>
                    <Tag active={e.wifi}>{e.wifi ? 'WiFi' : 'No WiFi'}</Tag>
                    <Tag active={e.ble}>{e.ble ? 'BLE' : 'No BLE'}</Tag>
                    <Tag active>USB</Tag>
                  </div>
                </div>
              ))}
            </div>
          )}
          <Alert variant="info">All variants support the same FSD/vehicle features. Only connectivity and bus count differ. Select in the Flasher tab.</Alert>
        </Section>

        {/* Connectivity (board-specific) */}
        {board === 'arduino' ? (
          <Section title="HC-05 Bluetooth" icon="📡" id="connectivity">
            <div className="sg-conn-grid">
              <div className="sg-conn-card">
                <h4>USB Serial</h4>
                <div className="sg-conn-card__detail">
                  <span>Always available</span>
                  <code>115200 baud</code>
                </div>
                <p>Primary connection. Web dashboard uses Web Serial API (Chrome/Edge).</p>
              </div>
              <div className="sg-conn-card">
                <h4>HC-05 Bluetooth <Badge variant="muted">Optional</Badge></h4>
                <div className="sg-conn-card__detail">
                  <span>SoftwareSerial on D4/D5</span>
                  <code>9600 baud</code>
                </div>
                <p>Wireless serial via Bluetooth Classic SPP. Same JSON protocol as USB. Android only — iOS does not support SPP.</p>
                <div className="sg-conn-card__notes">
                  <span>• Requires <code>_bt</code> firmware variant</span>
                  <span>• USB and BT work simultaneously</span>
                  <span>• Range: ~10m line-of-sight</span>
                  <span>• No encryption</span>
                </div>
              </div>
            </div>
          </Section>
        ) : (
          <Section title="Connectivity" icon="📡" id="connectivity">
            <div className="sg-conn-grid">
              <div className="sg-conn-card">
                <h4>USB Serial</h4>
                <div className="sg-conn-card__detail">
                  <span>Always available</span>
                  <code>115200 baud</code>
                </div>
                <p>Same Web Serial flow as Arduino.</p>
              </div>
              <div className="sg-conn-card">
                <h4>WiFi <Badge variant="accent">REST API</Badge></h4>
                <div className="sg-conn-card__detail">
                  <span>AP: TeslaCANModder</span>
                  <code>teslacan123</code>
                </div>
                <p>Embedded HTML dashboard at 192.168.4.1. Supports AP ↔ STA mode switching. Config saved to NVS.</p>
                <div className="sg-conn-card__endpoints">
                  <div><code>GET /</code> Dashboard</div>
                  <div><code>GET /api/status</code> Full state</div>
                  <div><code>POST /api/command</code> Send command</div>
                  <div><code>GET /api/disable</code> Emergency kill</div>
                  <div><code>POST /api/wifi/config</code> WiFi mode</div>
                  <div><code>GET /api/ble/status</code> BLE state</div>
                  <div><code>POST /api/ble/config</code> Toggle BLE</div>
                </div>
              </div>
              <div className="sg-conn-card">
                <h4>BLE <Badge variant="accent">NimBLE</Badge></h4>
                <div className="sg-conn-card__detail">
                  <span>Nordic UART Service</span>
                  <code>TeslaCANModder</code>
                </div>
                <p>Works with iOS and Android. Use nRF Connect or LightBlue. Same command protocol as serial.</p>
                <div className="sg-conn-card__notes">
                  <span>• TX: 6E400003 (notify)</span>
                  <span>• RX: 6E400002 (write)</span>
                  <span>• 256-byte ring buffer</span>
                  <span>• Runtime toggle via WiFi API</span>
                </div>
              </div>
            </div>
          </Section>
        )}

        {/* Vehicle Variants */}
        <Section title="Vehicle Variants" icon="🏎" id="variants">
          <div className="sg-variant-grid">
            {VARIANTS.map(v => (
              <div key={v.id} className="sg-variant-card">
                <div className="sg-variant-card__head">
                  <Badge variant="accent">{v.name}</Badge>
                </div>
                <div className="sg-variant-card__vehicles">{v.vehicles}</div>
                <div className="sg-variant-card__features">
                  {v.features.map(f => <Tag key={f} active>{f}</Tag>)}
                </div>
              </div>
            ))}
          </div>
          <Alert variant="info">
            Select variant in the Dashboard connection bar. Setting is saved to {board === 'arduino' ? 'EEPROM' : 'NVS flash'} and persists across power cycles.
          </Alert>
        </Section>

        {/* CAN Bus Map */}
        <Section title="CAN Bus & Feature Map" icon="📊" id="can-map">
          <WiringTable rows={[
            { Feature: 'FSD / Nag', 'CAN ID': '0x3FD (1021)', Bus: 'Bus 0 (FSD)', Variants: 'HW4, HW3' },
            { Feature: 'FSD / Nag (legacy)', 'CAN ID': '0x3EE (1006)', Bus: 'Bus 0', Variants: 'Legacy' },
            { Feature: 'Speed Profile', 'CAN ID': '0x3F8 (1016)', Bus: 'Bus 0 (FSD)', Variants: 'HW4, HW3' },
            { Feature: 'Speed Profile (legacy)', 'CAN ID': '0x45 (69)', Bus: 'Bus 0', Variants: 'Legacy' },
            { Feature: 'ISA Chime', 'CAN ID': '0x399 (921)', Bus: 'Bus 0 (FSD)', Variants: 'HW4' },
            { Feature: 'Speed Offset', 'CAN ID': '0x3FD (1021)', Bus: 'Bus 0 (FSD)', Variants: 'HW3' },
            { Feature: 'Summon / Vehicle Ctrl', 'CAN ID': '0x273 (627)', Bus: 'Bus 1 (Vehicle)', Variants: 'HW4, HW3' },
            { Feature: 'Climate', 'CAN ID': '0x2F3 (755)', Bus: 'Bus 1 (Vehicle)', Variants: 'HW4, HW3' },
            { Feature: 'Charge', 'CAN ID': '0x333 (819)', Bus: 'Bus 1 (Vehicle)', Variants: 'HW4, HW3' },
            { Feature: 'Drive Config', 'CAN ID': '0x334 (820)', Bus: 'Bus 1 (Vehicle)', Variants: 'HW4, HW3' },
            { Feature: 'Window Vent', 'CAN ID': '0x119 (281)', Bus: 'Bus 2 (Body)', Variants: 'All' },
            { Feature: 'Sentry', 'CAN ID': '0x284 (644)', Bus: 'Bus 2 (Body)', Variants: 'All' },
          ]} />
          <Alert variant="info">Filters are set automatically per variant. Use <code>can:raw:on</code> to see all bus traffic.</Alert>
        </Section>

        {/* Testing */}
        <Section title="Testing Your Setup" icon="🧪" id="testing">
          <div className="sg-test-grid">
            <div className="sg-test-card">
              <h4>Bench Test (before vehicle)</h4>
              <StepList steps={[
                `Power ${board === 'arduino' ? 'Arduino' : 'ESP32'} via USB from PC`,
                'Open Dashboard → Connect USB',
                'Verify boot message in Console',
                'Send ping — should get pong',
                'Toggle FSD on/off — check Console',
                ...(board === 'esp32' ? [
                  'Connect to WiFi AP "TeslaCANModder"',
                  'Open http://192.168.4.1 — verify dashboard',
                  'Test BLE with nRF Connect or Serial BT Terminal',
                ] : []),
                ...(board === 'arduino' ? [
                  'If BT variant: pair HC-05, open serial terminal at 9600 baud',
                ] : []),
              ]} />
            </div>
            <div className="sg-test-card">
              <h4>Vehicle Test</h4>
              <StepList steps={[
                'Install harness, power on vehicle',
                `${board === 'arduino' ? 'Arduino' : 'ESP32'} LED should blink (CAN activity)`,
                `Connect via USB${board === 'esp32' ? ', WiFi, or BLE' : board === 'arduino' ? ' or Bluetooth' : ''}`,
                'Start Stream → verify frame IDs appear',
                'HW4: expect 921, 1016, 1021, 627',
                'HW3: expect 1016, 1021, 627',
                'Legacy: expect 69, 1006',
                'Enable features one at a time',
              ]} />
            </div>
          </div>
        </Section>

        {/* Installation */}
        <Section title="Vehicle Installation" icon="🔩" id="install">
          <h4 style={{marginBottom: 'var(--space-md)'}}>X179 Location — behind center screen</h4>
          <StepList steps={[
            'Disconnect 12V battery before working on vehicle electrical',
            'Remove center screen trim panel',
            'Locate X179 connector (white 20-pin)',
            'Connect your harness (see wiring above)',
            'Route cables to a hidden location',
            `Secure ${board === 'arduino' ? 'Arduino + modules' : 'ESP32 + CAN transceivers'} with zip ties or velcro`,
            'Reinstall trim',
          ]} />
          <Alert variant="danger">
            This device modifies vehicle CAN bus messages. Use at your own risk. Improper use may affect vehicle safety systems. Educational and research purposes only.
          </Alert>
        </Section>

        {/* Troubleshooting */}
        <Section title="Troubleshooting" icon="🔍" id="troubleshoot">
          <div className="sg-trouble-grid">
            <div className="sg-trouble-card">
              <h4>Board not connecting</h4>
              <ul>
                <li>Use a data-capable USB cable (not charge-only)</li>
                <li>Install {board === 'arduino' ? 'CH340' : 'CP2102/CH340'} drivers</li>
                <li>Close Arduino IDE / other serial monitors</li>
                <li>Use Chrome or Edge (not Firefox/Safari)</li>
                {board === 'esp32' && <li>Hold BOOT button during upload if flash fails</li>}
              </ul>
            </div>
            <div className="sg-trouble-card">
              <h4>No CAN frames</h4>
              <ul>
                {board === 'arduino' ? (
                  <>
                    <li>Check MCP2515 wiring: CS→D10, INT→D2</li>
                    <li>Verify 8 MHz crystal on module</li>
                  </>
                ) : (
                  <>
                    <li>Check MCP2515 #1: CS→GPIO15, INT→GPIO34</li>
                    <li>Power MCP2515 from 5V (VIN), not 3.3V</li>
                  </>
                )}
                <li>Verify CAN-H/CAN-L to X179 pins 13/14</li>
                <li>Vehicle must be powered on (screen active)</li>
              </ul>
            </div>
            {board === 'arduino' && (
              <div className="sg-trouble-card">
                <h4>HC-05 not working</h4>
                <ul>
                  <li>Must flash a <code>_bt</code> firmware variant</li>
                  <li>Check TXD→D4, RXD→D5 wiring</li>
                  <li>Verify voltage divider on RXD line</li>
                  <li>Baud rate must be 9600</li>
                  <li>HC-05 LED: blinking = pairing, solid = connected</li>
                </ul>
              </div>
            )}
            {board === 'esp32' && (
              <>
                <div className="sg-trouble-card">
                  <h4>WiFi not connecting</h4>
                  <ul>
                    <li>Must flash a WiFi-enabled firmware variant</li>
                    <li>SSID: TeslaCANModder, Pass: teslacan123</li>
                    <li>Try http://192.168.4.1/api/ping</li>
                    <li>Check Console for "WiFi AP started"</li>
                  </ul>
                </div>
                <div className="sg-trouble-card">
                  <h4>Bus not detected</h4>
                  <ul>
                    <li>SPI: SCK→18, MISO→19, MOSI→23</li>
                    <li>Bus 0: CS→15, INT→34</li>
                    <li>Bus 1: CS→27, INT→35</li>
                    <li>Bus 2: CS→26, INT→33</li>
                  </ul>
                </div>
              </>
            )}
            <div className="sg-trouble-card">
              <h4>Summon not working</h4>
              <ul>
                <li>Must be HW3 or HW4 variant</li>
                <li>"Waiting for 0x273" = no cached frame yet</li>
                <li>Enable streaming, verify ID 627</li>
                <li>Need multi-CAN firmware for 0x273 on Bus 1</li>
              </ul>
            </div>
            <div className="sg-trouble-card">
              <h4>Board stops after car sleep</h4>
              <ul>
                <li>Board enters standby when CAN goes silent (10s)</li>
                <li>LED blinks slowly in standby</li>
                <li>Auto-recovers when car wakes</li>
              </ul>
            </div>
          </div>
        </Section>
      </div>
    </div>
  );
}
