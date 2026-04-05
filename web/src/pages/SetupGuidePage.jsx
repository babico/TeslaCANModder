export default function SetupGuidePage() {
  return (
    <div className="page page-setup">
      <div className="page-header">
        <h1>Setup Guide</h1>
        <p>Hardware installation &amp; software configuration for TeslaCANModder</p>
      </div>

      <div className="setup-content">
        {/* Quick Start */}
        <div className="panel">
          <div className="panel-header"><h2>Quick Start</h2></div>
          <div className="panel-body">
            <ol className="setup-steps">
              <li>Wire MCP2515 to Arduino Uno (see wiring below)</li>
              <li>Go to <strong>Flasher</strong> tab &rarr; select firmware &rarr; flash</li>
              <li>Go to <strong>Dashboard</strong> &rarr; Connect USB &rarr; verify boot message</li>
              <li>Select your variant (HW4/HW3/Legacy) in the connection bar</li>
              <li>Enable features (FSD, Nag, etc.) &mdash; all OFF by default</li>
              <li>Install in vehicle via X179 connector</li>
            </ol>
          </div>
        </div>

        {/* Components */}
        <div className="panel">
          <div className="panel-header"><h2>Required Hardware</h2></div>
          <div className="panel-body">
            <ul className="component-list">
              <li>
                <strong>Arduino Uno R3</strong>
                <span>CH340 or ATmega16U2 USB chip. The board that runs the firmware.</span>
              </li>
              <li>
                <strong>MCP2515 CAN Module #1</strong>
                <span>8 MHz crystal + TJA1050 transceiver. Connects to VehicleBus via X179.</span>
              </li>
              <li>
                <strong>9V-36V to 5V Buck Converter</strong>
                <span>3A minimum. Powers the Arduino from the vehicle 12V rail.</span>
              </li>
              <li>
                <strong>Tesla X179 Connector</strong>
                <span>Behind center screen. Provides 12V power + CAN bus access.</span>
              </li>
            </ul>
            <h3 style={{marginTop: 'var(--space-md)', marginBottom: 'var(--space-sm)'}}>Optional</h3>
            <ul className="component-list">
              <li>
                <strong>HC-05 Bluetooth Module</strong>
                <span>Wireless control from phone. Requires voltage divider on RX pin.</span>
              </li>
              <li>
                <strong>MCP2515 CAN Module #2</strong>
                <span>Tap a second CAN bus (e.g. Powertrain). Shares SPI, uses D9/D3.</span>
              </li>
            </ul>
          </div>
        </div>

        {/* Wiring: Primary MCP2515 */}
        <div className="panel">
          <div className="panel-header"><h2>Wiring &mdash; MCP2515 #1 (Required)</h2></div>
          <div className="panel-body">
            <div className="wiring-group">
              <h4>MCP2515 #1 &rarr; Arduino Uno</h4>
              <table className="wiring-table">
                <thead><tr><th>MCP2515</th><th>Arduino</th><th>Notes</th></tr></thead>
                <tbody>
                  <tr><td>VCC</td><td>5V</td><td></td></tr>
                  <tr><td>GND</td><td>GND</td><td></td></tr>
                  <tr><td>CS</td><td>D10</td><td>SPI chip select</td></tr>
                  <tr><td>INT</td><td>D2</td><td>Interrupt (INT0)</td></tr>
                  <tr><td>SCK</td><td>D13</td><td>SPI clock</td></tr>
                  <tr><td>MISO</td><td>D12</td><td>SPI data out</td></tr>
                  <tr><td>MOSI</td><td>D11</td><td>SPI data in</td></tr>
                </tbody>
              </table>
              <p className="note">This is the primary bus. Connect CAN-H/CAN-L to X179 pins 13/14 (VehicleBus).</p>
            </div>
          </div>
        </div>

        {/* Wiring: Second MCP2515 */}
        <div className="panel">
          <div className="panel-header"><h2>Wiring &mdash; MCP2515 #2 (Optional Dual CAN)</h2></div>
          <div className="panel-body">
            <p className="info-banner">
              A second MCP2515 lets you listen to another CAN bus simultaneously (e.g. PowertrainBus for pedal/regen discovery).
              The firmware detects it at boot &mdash; if it's not wired, everything works normally on a single bus.
            </p>
            <div className="wiring-group">
              <h4>MCP2515 #2 &rarr; Arduino Uno</h4>
              <table className="wiring-table">
                <thead><tr><th>MCP2515 #2</th><th>Arduino</th><th>Notes</th></tr></thead>
                <tbody>
                  <tr><td>VCC</td><td>5V</td><td>Shared rail</td></tr>
                  <tr><td>GND</td><td>GND</td><td>Shared rail</td></tr>
                  <tr><td>CS</td><td>D9</td><td>Unique chip select</td></tr>
                  <tr><td>INT</td><td>D3</td><td>Interrupt (INT1)</td></tr>
                  <tr><td>SCK</td><td>D13</td><td>Shared with #1</td></tr>
                  <tr><td>MISO</td><td>D12</td><td>Shared with #1</td></tr>
                  <tr><td>MOSI</td><td>D11</td><td>Shared with #1</td></tr>
                </tbody>
              </table>
              <p className="note">SPI lines (SCK/MISO/MOSI) are shared between both modules &mdash; only CS and INT differ.</p>
              <p className="warning">Arduino Uno has exactly 2 hardware interrupts: D2 (INT0) and D3 (INT1). Do not reuse D3 for anything else when using dual CAN.</p>
            </div>
            <div className="wiring-group">
              <h4>Firmware Build</h4>
              <p>Flash with a dual-CAN firmware variant from the Flasher page:</p>
              <ul className="setup-steps">
                <li><strong>USB + Dual CAN</strong> &mdash; no Bluetooth, with 2nd MCP2515</li>
                <li><strong>Full (USB + BT + Dual CAN)</strong> &mdash; Bluetooth + 2nd MCP2515</li>
              </ul>
              <p className="note">The <code>bus</code> field in frame JSON indicates which module received the frame (0 = primary, 1 = secondary).</p>
            </div>
          </div>
        </div>

        {/* Wiring: HC-05 */}
        <div className="panel">
          <div className="panel-header"><h2>Wiring &mdash; HC-05 Bluetooth (Optional)</h2></div>
          <div className="panel-body">
            <div className="wiring-group">
              <table className="wiring-table">
                <thead><tr><th>HC-05</th><th>Arduino</th><th>Notes</th></tr></thead>
                <tbody>
                  <tr><td>VCC</td><td>5V</td><td></td></tr>
                  <tr><td>GND</td><td>GND</td><td></td></tr>
                  <tr><td>RX</td><td>D4</td><td>Via voltage divider (5V &rarr; 3.3V)</td></tr>
                  <tr><td>TX</td><td>D5</td><td>Direct connection</td></tr>
                </tbody>
              </table>
              <p className="warning">HC-05 RX is 3.3V logic. Use a 1k&Omega; + 2k&Omega; voltage divider on the Arduino D4 &rarr; HC-05 RX line.</p>
            </div>
          </div>
        </div>

        {/* Wiring: X179 */}
        <div className="panel">
          <div className="panel-header"><h2>X179 Vehicle Connection</h2></div>
          <div className="panel-body">
            <div className="wiring-group">
              <img
                src="/reference/X179_Connector_Pinout_Colored.png"
                alt="Tesla X179 connector pinout"
                className="setup-img"
              />
              <table className="wiring-table">
                <thead><tr><th>X179 Pin</th><th>Connection</th><th>Purpose</th></tr></thead>
                <tbody>
                  <tr><td>Pin 1</td><td>Buck converter VIN+</td><td>12V power</td></tr>
                  <tr><td>Pin 20</td><td>Buck converter VIN&minus;</td><td>Ground</td></tr>
                  <tr><td>Pin 13</td><td>MCP2515 #1 CAN-H</td><td>VehicleBus high</td></tr>
                  <tr><td>Pin 14</td><td>MCP2515 #1 CAN-L</td><td>VehicleBus low</td></tr>
                </tbody>
              </table>
              <p className="note">Buck converter 5V USB output &rarr; Arduino USB port. All current features use VehicleBus (pins 13/14).</p>
              <p className="note">For MCP2515 #2, connect CAN-H/CAN-L to whichever secondary bus you want to monitor (tap from a different connector pair).</p>
            </div>
          </div>
        </div>

        {/* Firmware Variants */}
        <div className="panel">
          <div className="panel-header"><h2>Firmware Variants</h2></div>
          <div className="panel-body">
            <table className="wiring-table">
              <thead>
                <tr><th>Variant</th><th>Bluetooth</th><th>Dual CAN</th><th>Use Case</th></tr>
              </thead>
              <tbody>
                <tr><td><strong>USB Only</strong></td><td>No</td><td>No</td><td>Minimal &mdash; lightest firmware</td></tr>
                <tr><td><strong>USB + Bluetooth</strong></td><td>Yes</td><td>No</td><td>Wireless control via HC-05</td></tr>
                <tr><td><strong>USB + Dual CAN</strong></td><td>No</td><td>Yes</td><td>Monitor two CAN buses</td></tr>
                <tr><td><strong>Full</strong></td><td>Yes</td><td>Yes</td><td>Everything enabled</td></tr>
              </tbody>
            </table>
            <p className="note">All variants support all FSD/vehicle features. Bluetooth and Dual CAN only affect I/O capability.</p>
          </div>
        </div>

        {/* Vehicle Variants */}
        <div className="panel">
          <div className="panel-header"><h2>Vehicle Variant Selection</h2></div>
          <div className="panel-body">
            <table className="wiring-table">
              <thead><tr><th>Variant</th><th>Vehicles</th><th>Features</th></tr></thead>
              <tbody>
                <tr>
                  <td><strong>HW4</strong></td>
                  <td>2023+ with HW4 (FSD v14+)</td>
                  <td>FSD, Nag, Profile, ISA Chime, Summon</td>
                </tr>
                <tr>
                  <td><strong>HW3</strong></td>
                  <td>2019-2023 with HW3</td>
                  <td>FSD, Nag, Profile, Speed Offset, Summon</td>
                </tr>
                <tr>
                  <td><strong>Legacy</strong></td>
                  <td>Pre-HW3 vehicles</td>
                  <td>FSD, Nag, Profile (limited)</td>
                </tr>
              </tbody>
            </table>
            <p className="note">Select variant in the Dashboard connection bar. Setting is saved to EEPROM.</p>
          </div>
        </div>

        {/* CAN Bus & Feature Map */}
        <div className="panel">
          <div className="panel-header"><h2>CAN Bus &amp; Feature Map</h2></div>
          <div className="panel-body">
            <table className="wiring-table">
              <thead><tr><th>Feature</th><th>CAN ID</th><th>Bus</th><th>Variants</th></tr></thead>
              <tbody>
                <tr><td>FSD / Nag</td><td>1021 (0x3FD)</td><td>VehicleBus</td><td>HW4, HW3</td></tr>
                <tr><td>FSD / Nag (legacy)</td><td>1006 (0x3EE)</td><td>Chassis</td><td>Legacy</td></tr>
                <tr><td>Speed Profile</td><td>1016 (0x3F8)</td><td>VehicleBus</td><td>HW4, HW3</td></tr>
                <tr><td>Speed Profile (legacy)</td><td>69 (0x45)</td><td>Chassis</td><td>Legacy</td></tr>
                <tr><td>Speed Offset</td><td>1021 (0x3FD)</td><td>VehicleBus</td><td>HW3</td></tr>
                <tr><td>ISA Chime</td><td>921 (0x399)</td><td>VehicleBus</td><td>HW4</td></tr>
                <tr><td>Summon</td><td>627 (0x273)</td><td>VehicleBus</td><td>HW3, HW4</td></tr>
                <tr><td>Climate/Charge/Drive</td><td>755/819/820</td><td>VehicleBus</td><td>HW3, HW4</td></tr>
              </tbody>
            </table>
            <p className="note">Filters are set automatically per variant. Use <code>can:raw:on</code> in the console to see all bus traffic.</p>
          </div>
        </div>

        {/* Testing */}
        <div className="panel">
          <div className="panel-header"><h2>Testing Your Setup</h2></div>
          <div className="panel-body">
            <div className="setup-section">
              <h3>Bench Test (before vehicle install)</h3>
              <ol className="setup-steps">
                <li>Power Arduino via USB from PC</li>
                <li>Open Dashboard &rarr; Connect USB</li>
                <li>Verify boot messages in Console</li>
                <li>Check Board panel shows hardware info</li>
                <li>Send <code>ping</code> &mdash; should get Pong</li>
                <li>Toggle FSD on/off &mdash; check Console logs</li>
              </ol>
            </div>
            <div className="setup-section">
              <h3>Vehicle Test</h3>
              <ol className="setup-steps">
                <li>Install harness, power on vehicle</li>
                <li>Arduino LED should blink (CAN activity)</li>
                <li>Connect via USB or Bluetooth</li>
                <li>Start Stream &rarr; verify frame IDs appear</li>
                <li><strong>HW4:</strong> expect 921, 1016, 1021, 627</li>
                <li><strong>HW3:</strong> expect 1016, 1021, 627</li>
                <li><strong>Legacy:</strong> expect 69, 1006</li>
                <li>Enable features one at a time, check Console for confirmation logs</li>
              </ol>
              <p className="note">If 627 (0x273) is missing, summon won&apos;t work. Enable <code>can:raw:on</code> to verify the bus.</p>
            </div>
          </div>
        </div>

        {/* Vehicle Installation */}
        <div className="panel">
          <div className="panel-header"><h2>Vehicle Installation</h2></div>
          <div className="panel-body">
            <div className="setup-section">
              <h3>X179 Location</h3>
              <p>Behind the center screen in Tesla vehicles.</p>
              <ol className="setup-steps">
                <li>Remove center screen trim panel</li>
                <li>Locate X179 connector (white 20-pin)</li>
                <li>Connect your harness (see wiring above)</li>
                <li>Route cables to a hidden location</li>
                <li>Secure Arduino + modules with zip ties or velcro</li>
                <li>Reinstall trim</li>
              </ol>
              <p className="warning">Disconnect 12V battery before working on vehicle electrical systems.</p>
            </div>
          </div>
        </div>

        {/* Troubleshooting */}
        <div className="panel">
          <div className="panel-header"><h2>Troubleshooting</h2></div>
          <div className="panel-body">
            <div className="troubleshoot-item">
              <h4>Board not connecting</h4>
              <ul>
                <li>Use a data-capable USB cable (not charge-only)</li>
                <li>Install CH340 drivers for clone Arduinos</li>
                <li>Close Arduino IDE / other serial monitors</li>
                <li>Use Chrome or Edge (not Firefox/Safari)</li>
              </ul>
            </div>
            <div className="troubleshoot-item">
              <h4>No CAN frames</h4>
              <ul>
                <li>Check MCP2515 wiring: CS&rarr;D10, INT&rarr;D2, SPI pins</li>
                <li>Verify 8 MHz crystal on MCP2515 module</li>
                <li>Verify CAN-H/CAN-L to X179 pins 13/14</li>
                <li>Vehicle must be powered on (screen active)</li>
                <li>Console shows &ldquo;CAN bus silent&rdquo; &rarr; wiring issue</li>
              </ul>
            </div>
            <div className="troubleshoot-item">
              <h4>Dual CAN not detected</h4>
              <ul>
                <li>Must flash a dual-CAN firmware variant</li>
                <li>Check CS&rarr;D9 and INT&rarr;D3 wiring</li>
                <li>Board panel should show &ldquo;Dual&rdquo; under CAN</li>
                <li>If #2 fails init, firmware falls back to single-bus silently</li>
              </ul>
            </div>
            <div className="troubleshoot-item">
              <h4>Summon not working</h4>
              <ul>
                <li>Must be HW3 or HW4 variant</li>
                <li>Console error &ldquo;Waiting for 0x273&rdquo; = no cached frame yet</li>
                <li>Enable streaming, verify ID 627 in frame table</li>
                <li>If missing even in raw mode: X179 13/14 may not be VehicleBus</li>
              </ul>
            </div>
            <div className="troubleshoot-item">
              <h4>Board stops after car sleep</h4>
              <ul>
                <li>If using powerbank: board enters standby when CAN goes silent</li>
                <li>LED blinks slowly in standby mode</li>
                <li>Auto-recovers when car wakes and CAN resumes</li>
                <li>Check Console for &ldquo;Standby&rdquo; / &ldquo;Resuming&rdquo; logs</li>
              </ul>
            </div>
          </div>
        </div>

        {/* Safety */}
        <div className="panel">
          <div className="panel-header"><h2>Safety &amp; Legal</h2></div>
          <div className="panel-body">
            <p className="warning">
              <strong>WARNING:</strong> This device modifies vehicle CAN bus messages. Use at your own risk.
              Improper use may affect vehicle safety systems. Always test in safe environments.
            </p>
            <ul className="setup-steps">
              <li>Educational and research purposes only</li>
              <li>You are responsible for safe operation</li>
              <li>May void vehicle warranty</li>
              <li>Check local regulations on vehicle modifications</li>
              <li>Always maintain control of your vehicle</li>
              <li>Test features one at a time in safe locations</li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  );
}

