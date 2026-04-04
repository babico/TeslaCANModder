import { useState } from 'react';
import { STK500, parseHex } from '../utils/stk500';

const DIRECT_FLASH_AVAILABLE = import.meta.env.DEV;
const FIRMWARE_OPTIONS = [
  {
    envName: 'uno_usb',
    title: 'USB-only firmware',
    badge: 'Recommended without HC-05',
    buildCommand: `cd hardware
pio run -e uno_usb`,
    hexPath: 'hardware/.pio/build/uno_usb/firmware.hex',
    description: 'Use this when the HC-05 module is not physically installed. Bluetooth support stays disabled so the board runs on the cleaner USB-only path.',
    buttonLabel: 'Flash USB-only firmware',
    buttonColor: 'var(--red)',
  },
  {
    envName: 'uno',
    title: 'USB + HC-05 firmware',
    badge: 'Only when HC-05 is installed',
    buildCommand: `cd hardware
pio run -e uno`,
    hexPath: 'hardware/.pio/build/uno/firmware.hex',
    description: 'Use this only after the HC-05 is actually wired, powered, and ready to use as an optional secondary transport.',
    buttonLabel: 'Flash USB + HC-05 firmware',
    buttonColor: 'var(--blue)',
  },
];

const RECOMMENDED_FIRMWARE = FIRMWARE_OPTIONS[0];

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
  const [recoveryEraseEnabled, setRecoveryEraseEnabled] = useState(false);
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

  const executeFlash = async (firmwareData, options = {}) => {
    const { fullErase = false } = options;

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

      if (fullErase) {
        updateProgress('Erasing Entire Chip...', 20);
        await flasher.chipErase();
      }

      const totalBytes = firmwareData.length;
      let writtenBytes = 0;
      updateProgress('Flashing Firmware...', fullErase ? 28 : 20);

      // Optiboot expects writes aligned to the Uno page size.
      for (let offset = 0; offset < totalBytes; offset += 128) {
        const chunkLength = Math.min(128, totalBytes - offset);
        const chunk = new Uint8Array(128).fill(0xff);
        chunk.set(firmwareData.slice(offset, offset + chunkLength));

        await flasher.loadAddress(offset / 2);
        await flasher.writePage(chunk);

        writtenBytes += chunkLength;
        updateProgress(
          `Writing page at 0x${offset.toString(16)}...`,
          (fullErase ? 28 : 20) + (writtenBytes / totalBytes) * (fullErase ? 67 : 75),
        );
      }

      updateProgress('Leaving Programming Mode...', 96);
      await flasher.leaveProgMode();
      updateProgress(fullErase ? 'Success! Full erase + flash complete.' : 'Success! Board restarting.', 100, 'var(--green)');
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
      await executeFlash(parsed, { fullErase: recoveryEraseEnabled });
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

      <div
        className="panel"
        style={{
          padding: '16px',
          marginBottom: '20px',
          border: '1px solid rgba(248, 113, 113, 0.45)',
          background: 'rgba(127, 29, 29, 0.14)',
        }}
      >
        <p style={{ margin: 0, color: 'var(--text)' }}>
          If the HC-05 is not physically installed, do not flash the Bluetooth build. Use <code>uno_usb</code> and keep the board on the USB-only firmware path.
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
            <p>The first flash should use a firmware image generated by the `hardware` PlatformIO project. Pick the USB-only image unless the HC-05 is really installed.</p>
          </div>
        </div>

        <div style={{ display: 'grid', gap: '20px', marginBottom: '20px' }}>
          {FIRMWARE_OPTIONS.map((option) => (
            <div key={option.envName} className="panel" style={{ padding: '16px' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', gap: '12px', alignItems: 'center', flexWrap: 'wrap', marginBottom: '12px' }}>
                <div>
                  <h4 style={{ margin: 0 }}>{option.title}</h4>
                  <p style={{ margin: '6px 0 0', color: 'var(--text-muted)' }}>{option.description}</p>
                </div>
                <span className="badge mono">{option.badge}</span>
              </div>

              <div className="config-grid">
                <CodePanel title="Build command" code={option.buildCommand} />
                <CodePanel title="Expected HEX output" code={option.hexPath} />
              </div>
            </div>
          ))}
        </div>

        <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
          <label style={{ display: 'flex', gap: '10px', alignItems: 'flex-start', cursor: flashState.isFlashing ? 'not-allowed' : 'pointer' }}>
            <input
              type="checkbox"
              checked={recoveryEraseEnabled}
              onChange={(event) => setRecoveryEraseEnabled(event.target.checked)}
              disabled={flashState.isFlashing}
              style={{ marginTop: '3px' }}
            />
            <span style={{ color: 'var(--text-muted)' }}>
              <strong style={{ color: 'var(--text)' }}>Recovery mode: fully erase the chip before flashing.</strong> Use this when a normal upload verifies incorrectly, old firmware behaves strangely, or you want the same recovery flow as a manual erase + reflash.
            </span>
          </label>
        </div>

        {DIRECT_FLASH_AVAILABLE ? (
          <div style={{ display: 'flex', gap: '12px', flexWrap: 'wrap', marginBottom: '20px' }}>
            {FIRMWARE_OPTIONS.map((option) => (
              <button
                key={option.envName}
                className="btn-primary"
                onClick={() => handleFetchAndFlash(option.envName)}
                disabled={flashState.isFlashing}
                style={{ flex: 1, minWidth: '240px', padding: '12px', background: option.buttonColor, border: 'none' }}
              >
                ⚡ {recoveryEraseEnabled ? `Full Erase + ${option.buttonLabel}` : option.buttonLabel}
              </button>
            ))}
          </div>
        ) : (
          <div className="panel" style={{ padding: '16px', marginBottom: '20px' }}>
            <p style={{ margin: 0, color: 'var(--text-muted)' }}>
              Direct project flashing is hidden in production builds because it depends on the local Vite middleware that serves `.hex` files from the `hardware/.pio/build` folder. In that case, build `hardware` yourself and use the custom file picker with <code>{RECOMMENDED_FIRMWARE.hexPath}</code> when HC-05 is absent, or <code>hardware/.pio/build/uno/firmware.hex</code> only when the Bluetooth hardware is actually installed.
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
              onClick={() => executeFlash(firmwareFile, { fullErase: recoveryEraseEnabled })}
              disabled={!firmwareFile || flashState.isFlashing}
              style={{ padding: '10px 24px', background: 'var(--red)', border: 'none' }}
            >
              {flashState.isFlashing ? 'Flashing...' : `⚡ ${recoveryEraseEnabled ? 'Full Erase + Flash Custom File' : 'Flash Custom File'}`}
            </button>
          </div>
          <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.85rem' }}>
            Recommended first-time file without HC-05: <code>{RECOMMENDED_FIRMWARE.hexPath}</code>
          </p>
          <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.85rem' }}>
            Use <code>hardware/.pio/build/uno/firmware.hex</code> only when HC-05 is physically installed and you want the optional Bluetooth transport enabled in firmware.
          </p>
          <div
            className="panel"
            style={{
              padding: '14px 16px',
              border: '1px solid var(--border)',
              background: 'var(--bg-elevated)',
            }}
          >
            <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.85rem' }}>
              Troubleshoot: if upload verify mismatches or the board behaves like an older build is still present, enable recovery mode and run a full erase + flash once over USB with the Uno connected by itself.
            </p>
          </div>

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
          This tool speaks STK500v1 directly from the browser, toggles the serial control lines to reset the Uno into Optiboot, and can optionally issue a full chip erase before writing the firmware a page at a time.
        </p>
        <p style={{ fontSize: '0.9rem', color: 'var(--text-muted)' }}>
          The canonical first image is the shared `hardware` build. Start with `uno_usb` when HC-05 is absent, and only use `uno` after the HC-05 hardware is really part of the install. After flashing, choose `HW4`, `HW3`, or `Legacy` from the dashboard instead of flashing a separate per-variant build.
        </p>
      </div>
    </div>
  );
}
