import { useState } from 'react';
import { STK500, parseHex } from '../utils/stk500';

const DIRECT_FLASH_AVAILABLE = import.meta.env.DEV;
const HARDWARE_BUILD_COMMAND = `cd hardware
pio run -e uno`;
const HARDWARE_HEX_PATH = 'hardware/.pio/build/uno/firmware.hex';

function CodePanel({ title, code }) {
  return (
    <div className="config-block">
      <div className="config-block-header">
        <span>{title}</span>
      </div>
      <pre><code>{code}</code></pre>
    </div>
  );
}

export default function Flasher({ board }) {
  const capabilities = board?.capabilities;
  const [firmwareFile, setFirmwareFile] = useState(null);
  const [flashState, setFlashState] = useState({
    status: 'Ready',
    percent: 0,
    color: 'var(--text-muted)',
    isFlashing: false,
  });

  const updateProgress = (status, percent, color = 'var(--blue)') => {
    setFlashState((previous) => ({ ...previous, status, percent, color }));
  };

  const handleFileChange = async (event) => {
    const file = event.target.files[0];
    if (!file) {
      setFirmwareFile(null);
      updateProgress('Ready', 0, 'var(--text-muted)');
      return;
    }

    try {
      const text = await file.text();
      const parsed = parseHex(text);
      setFirmwareFile(parsed);
      updateProgress(`Ready to flash custom file: ${parsed.length} bytes`, 0, 'var(--text-muted)');
    } catch (error) {
      console.error('HEX parsing failed:', error);
      alert('Invalid .hex file format.');
      setFirmwareFile(null);
      updateProgress('Ready', 0, 'var(--text-muted)');
    }
  };

  const executeFlash = async (firmwareData) => {
    if (!firmwareData || firmwareData.length === 0) {
      alert('Choose a valid firmware image first.');
      return;
    }

    if (!('serial' in navigator)) {
      alert('Web Serial API is not supported in this browser. Please use Chrome, Edge, or Opera.');
      return;
    }

    let port;
    const flasherDisconnect = async (flasher) => {
      try {
        await flasher.disconnect();
      } catch (disconnectError) {
        console.warn('Bootloader disconnect failed:', disconnectError);
      }
    };

    try {
      updateProgress('Requesting Port...', 0);
      port = await navigator.serial.requestPort();
    } catch (requestError) {
      console.info('Port request cancelled:', requestError);
      updateProgress('Flash canceled.', 0, 'var(--text-muted)');
      return;
    }

    setFlashState((previous) => ({ ...previous, isFlashing: true }));
    updateProgress('Connecting to Bootloader...', 5);

    const flasher = new STK500(port);
    try {
      await flasher.connect();
      updateProgress('Syncing with STK500...', 10);

      await flasher.sync();
      updateProgress('Entering Programming Mode...', 15);

      await flasher.enterProgMode();

      const totalBytes = firmwareData.length;
      let writtenBytes = 0;
      updateProgress('Flashing Firmware...', 20);

      // Optiboot expects writes aligned to the Uno page size.
      for (let offset = 0; offset < totalBytes; offset += 128) {
        const chunkLength = Math.min(128, totalBytes - offset);
        const chunk = new Uint8Array(128).fill(0xff);
        chunk.set(firmwareData.slice(offset, offset + chunkLength));

        await flasher.loadAddress(offset / 2);
        await flasher.writePage(chunk);

        writtenBytes += chunkLength;
        updateProgress(`Writing page at 0x${offset.toString(16)}...`, 20 + (writtenBytes / totalBytes) * 75);
      }

      updateProgress('Leaving Programming Mode...', 96);
      await flasher.leaveProgMode();
      updateProgress('Success! Board restarting.', 100, 'var(--green)');
    } catch (error) {
      console.error('Flash error', error);
      updateProgress(`Flashing Failed: ${error.message}`, 100, 'var(--danger)');
    } finally {
      await flasherDisconnect(flasher);
      setTimeout(() => {
        setFlashState((previous) => ({ ...previous, isFlashing: false }));
      }, 1200);
    }
  };

  const handleFetchAndFlash = async (envName) => {
    if (!DIRECT_FLASH_AVAILABLE) {
      alert('Direct project flashing is only available while the Vite dev server is running locally.');
      return;
    }

    updateProgress(`Fetching ${envName} firmware...`, 0);

    try {
      const response = await fetch(`/firmware-builds/${envName}/firmware.hex`);
      if (!response.ok) {
        throw new Error(`Firmware not found for ${envName}. Build the hardware project first.`);
      }

      const text = await response.text();
      const parsed = parseHex(text);
      updateProgress(`Fetched ${parsed.length} bytes from ${envName}.`, 0);
      await executeFlash(parsed);
    } catch (error) {
      console.error('Direct flash fetch failed:', error);
      alert(error.message);
      updateProgress('Ready', 0, 'var(--text-muted)');
      setFlashState((previous) => ({ ...previous, isFlashing: false }));
    }
  };

  return (
    <div className="page-shell">
      <div className="page-hero" style={{ borderBottom: '1px solid var(--border)' }}>
        <h2>⚡ Arduino Optiboot Web Flasher</h2>
        <p>Flash an Arduino Uno / CH340 board directly from the browser over Web Serial.</p>
      </div>

      <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
        <p style={{ margin: 0, color: 'var(--text-muted)' }}>
          This browser flasher is only for the Arduino Uno Optiboot path used by this project. It does not replace the normal Arduino IDE or PlatformIO upload flow.
        </p>
      </div>

      {capabilities && (
        <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
          <p style={{ margin: 0, color: 'var(--text-muted)' }}>
            {capabilities.supportsFirstFlash
              ? 'This browser supports the desktop-first flash flow. Keep the first flash on USB, then move the board to X179-powered operation later.'
              : 'First flash stays desktop-first. Use desktop Chrome or Edge over USB for the first firmware upload, then use the phone UI later for guide and runtime control where supported.'}
          </p>
        </div>
      )}

      <div className="page-section" style={{ marginTop: '30px' }}>
        <div className="section-header">
          <span className="step-number" style={{ background: 'var(--red)' }}>1</span>
          <div>
            <h3>First Flash: Build the hardware project</h3>
            <p>The first flash should use the shared Uno firmware generated by the `hardware` PlatformIO project, then runtime vehicle selection happens later in the dashboard.</p>
          </div>
        </div>

        <div className="config-grid" style={{ marginBottom: '20px' }}>
          <CodePanel title="Build the firmware from hardware/" code={HARDWARE_BUILD_COMMAND} />
          <CodePanel title="Expected HEX output" code={HARDWARE_HEX_PATH} />
        </div>

        <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
          <p style={{ margin: 0, color: 'var(--text-muted)' }}>
            Build `hardware` first. The browser button below reads the generated `firmware.hex` from that build output, and the manual file picker should point to the same file when you are not using local dev mode.
          </p>
        </div>

        {DIRECT_FLASH_AVAILABLE ? (
          <div style={{ display: 'flex', gap: '12px', flexWrap: 'wrap', marginBottom: '20px' }}>
            <button
              className="btn-primary"
              onClick={() => handleFetchAndFlash('uno')}
              disabled={flashState.isFlashing}
              style={{ flex: 1, padding: '12px', background: 'var(--blue)', border: 'none' }}
            >
              ⚡ Flash hardware/uno firmware
            </button>
          </div>
        ) : (
          <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
            <p style={{ margin: 0, color: 'var(--text-muted)' }}>
              Direct project flashing is hidden in production builds because it depends on the local Vite middleware that serves `.hex` files from the `hardware/.pio/build` folder. In that case, build `hardware` yourself and use the custom file picker with `hardware/.pio/build/uno/firmware.hex`.
            </p>
          </div>
        )}

        <div className="section-header" style={{ marginTop: '30px' }}>
          <span className="step-number" style={{ background: 'var(--text-muted)' }}>2</span>
          <div>
            <h3>Upload the built hardware HEX manually</h3>
            <p>Select the `.hex` produced by the `hardware` project, or another manually compiled Uno-compatible Optiboot image.</p>
          </div>
        </div>

        <div className="config-output" style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px', flexWrap: 'wrap' }}>
            <label className="field" style={{ flex: 1 }}>
              <span>Select Firmware (.hex)</span>
              <input
                type="file"
                accept=".hex"
                onChange={handleFileChange}
                disabled={flashState.isFlashing}
                style={{
                  width: '100%',
                  padding: '8px',
                  border: '1px solid var(--border)',
                  borderRadius: '4px',
                  background: 'var(--bg-elevated)',
                  color: 'var(--text)',
                  cursor: 'pointer',
                }}
              />
            </label>
            <button
              className="btn-primary"
              onClick={() => executeFlash(firmwareFile)}
              disabled={!firmwareFile || flashState.isFlashing}
              style={{ padding: '10px 24px', background: 'var(--red)', border: 'none' }}
            >
              {flashState.isFlashing ? 'Flashing...' : '⚡ Flash Custom File'}
            </button>
          </div>
          <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.85rem' }}>
            Recommended first-time file: <code>{HARDWARE_HEX_PATH}</code>
          </p>

          {(flashState.percent > 0 || flashState.status !== 'Ready') && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '8px', marginTop: '10px' }}>
              <div
                style={{
                  display: 'flex',
                  justifyContent: 'space-between',
                  fontSize: '0.85rem',
                  fontFamily: '"JetBrains Mono", monospace',
                  color: flashState.color,
                }}
              >
                <span>{flashState.status}</span>
                <span>{Math.round(flashState.percent)}%</span>
              </div>
              <div style={{ width: '100%', height: '8px', background: 'var(--bg-dark)', borderRadius: '4px', overflow: 'hidden' }}>
                <div style={{ width: `${flashState.percent}%`, height: '100%', background: flashState.color, transition: 'width 0.1s linear' }}></div>
              </div>
            </div>
          )}
        </div>
      </div>

      <div className="panel log-panel" style={{ marginTop: '20px', padding: '20px' }}>
        <h3>How it works</h3>
        <p style={{ fontSize: '0.9rem', color: 'var(--text-muted)', marginBottom: '10px' }}>
          This tool speaks STK500v1 directly from the browser, toggles the serial control lines to reset the Uno into Optiboot, then writes the firmware a page at a time.
        </p>
        <p style={{ fontSize: '0.9rem', color: 'var(--text-muted)' }}>
          The canonical first image is the shared Uno build from `hardware`. After flashing that image, choose `HW4`, `HW3`, or `Legacy` from the dashboard instead of flashing a separate per-variant build.
        </p>
      </div>
    </div>
  );
}
