import { useEffect, useMemo, useState } from 'react';
import { toolkitPackages } from '../packages';
import { BoardCommands, BOARD_VARIANTS } from '../lib/board/commands';

const VARIANT_LABELS = {
  hw4: 'HW4',
  hw3: 'HW3',
  legacy: 'Legacy',
};

const INSTALL_LABELS = {
  'bench-ready': 'Bench Ready',
  'installed-power-ready': 'Installed Power Ready',
  'runtime-ready': 'Runtime Ready',
};

function formatUptime(uptimeMs) {
  if (!uptimeMs) {
    return '—';
  }

  const totalSeconds = Math.floor(uptimeMs / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;

  if (hours > 0) {
    return `${hours}h ${minutes}m ${seconds}s`;
  }

  if (minutes > 0) {
    return `${minutes}m ${seconds}s`;
  }

  return `${seconds}s`;
}

function getTransportLabel(activeTransport) {
  if (activeTransport === 'bluetooth') {
    return 'HC-05';
  }

  if (activeTransport === 'usb') {
    return 'USB';
  }

  return '—';
}

function getInstallLabel(readiness) {
  return INSTALL_LABELS[readiness] || '—';
}

function MobilePanelTabs({ activePanel, onChange }) {
  return (
    <div className="dashboard-panel-tabs">
      {[
        ['control', 'Control'],
        ['monitor', 'Monitor'],
        ['advanced', 'Advanced'],
      ].map(([id, label]) => (
        <button
          key={id}
          type="button"
          className={`dashboard-panel-tab ${activePanel === id ? 'active' : ''}`}
          onClick={() => onChange(id)}
        >
          {label}
        </button>
      ))}
    </div>
  );
}

function FrameList({ frames }) {
  const [selectedKey, setSelectedKey] = useState(null);

  const selectedFrame = frames.find((frame) => `${frame.seenAt}-${frame.id}` === selectedKey) || frames[0] || null;

  return (
    <div className="mobile-frame-viewer">
      <div className="mobile-frame-list">
        {frames.map((frame, index) => {
          const key = `${frame.seenAt}-${frame.id}-${index}`;
          const isActive = selectedFrame && selectedFrame.seenAt === frame.seenAt && selectedFrame.id === frame.id;

          return (
            <button
              key={key}
              type="button"
              className={`mobile-frame-row ${isActive ? 'active' : ''}`}
              onClick={() => setSelectedKey(`${frame.seenAt}-${frame.id}`)}
            >
              <div className="mobile-frame-row-top">
                <strong>0x{frame.id.toString(16).toUpperCase()}</strong>
                <span>{frame.dir.toUpperCase()}</span>
              </div>
              <div className="mobile-frame-row-meta">
                <span>{frame.ts}</span>
                <span>{frame.dataHex || '—'}</span>
              </div>
            </button>
          );
        })}
      </div>

      {selectedFrame ? (
        <div className="mobile-frame-detail">
          <div className="mobile-frame-detail-head">
            <strong>Latest Frame Detail</strong>
            <span className="badge mono">{selectedFrame.dlc} bytes</span>
          </div>
          <div className="mobile-frame-detail-grid">
            <div className="mobile-frame-detail-card">
              <span>ID</span>
              <strong>0x{selectedFrame.id.toString(16).toUpperCase()}</strong>
            </div>
            <div className="mobile-frame-detail-card">
              <span>Direction</span>
              <strong>{selectedFrame.dir.toUpperCase()}</strong>
            </div>
            <div className="mobile-frame-detail-card">
              <span>Time</span>
              <strong>{selectedFrame.ts}</strong>
            </div>
            <div className="mobile-frame-detail-card">
              <span>Data</span>
              <strong className="mono">{selectedFrame.dataHex || '—'}</strong>
            </div>
          </div>
        </div>
      ) : null}
    </div>
  );
}

function PackagePanels({ board, packages, isCompact }) {
  const activePackagePanels = toolkitPackages.filter((packageDefinition) => packageDefinition.DashboardPanel);

  return activePackagePanels.map((packageDefinition) => {
    const DashboardPanel = packageDefinition.DashboardPanel;

    if (!isCompact) {
      return (
        <DashboardPanel
          key={packageDefinition.id}
          packageState={packages[packageDefinition.id]}
          board={board}
        />
      );
    }

    return (
      <details key={packageDefinition.id} className="dashboard-collapsible" open>
        <summary>{packageDefinition.label || packageDefinition.id}</summary>
        <DashboardPanel packageState={packages[packageDefinition.id]} board={board} />
      </details>
    );
  });
}

export default function Dashboard({ board }) {
  const {
    isConnected,
    isStreaming,
    status,
    connect,
    disconnect,
    sendCommand,
    telemetry,
    packages,
    frameCount,
    getFrames,
    clearFrames,
    consoleLines,
    clearConsole,
    deviceInfo,
    activeTransport,
    capabilities,
  } = board;

  const [commandInput, setCommandInput] = useState('');
  const [frames, setFrames] = useState([]);
  const [mobilePanel, setMobilePanel] = useState('control');

  useEffect(() => {
    if (!isConnected) {
      return undefined;
    }

    const interval = setInterval(() => {
      setFrames([...getFrames()]);
    }, 120);

    return () => clearInterval(interval);
  }, [getFrames, isConnected]);

  const isCompact = capabilities.isMobile;
  const visibleFrames = isConnected ? frames : [];
  const activeVariant = telemetry.variant || deviceInfo?.variant || null;
  const activeVariantLabel = activeVariant ? (VARIANT_LABELS[activeVariant] || activeVariant.toUpperCase()) : '—';
  const activeDriver = telemetry.driver || deviceInfo?.drv || '—';
  const activeHardware = telemetry.hardware || deviceInfo?.hw || '—';
  const installReadiness = telemetry.installReadiness || deviceInfo?.ready || null;
  const transportCapability = telemetry.transportCapability || deviceInfo?.cap || null;

  const handleSend = () => {
    const nextCommand = commandInput.trim();
    if (!nextCommand || !isConnected) {
      return;
    }

    void sendCommand(nextCommand);
    setCommandInput('');
  };

  const quickActions = useMemo(() => ([
    { id: 'ping', label: 'Ping', run: () => sendCommand(BoardCommands.ping()) },
    { id: 'status', label: 'Status', run: () => sendCommand(BoardCommands.status()) },
    { id: 'stream', label: isStreaming ? 'Stop Stream' : 'Start Stream', run: () => sendCommand(BoardCommands.stream(!isStreaming)) },
  ]), [isStreaming, sendCommand]);

  const showControl = !isCompact || mobilePanel === 'control';
  const showMonitor = !isCompact || mobilePanel === 'monitor';
  const showAdvanced = !isCompact || mobilePanel === 'advanced';

  return (
    <div className="dash-container">
      <div className={`connection-banner ${isConnected ? 'connected' : ''}`}>
        <div className="conn-left">
          <span className={`status-dot ${isConnected ? 'online' : 'offline'}`}></span>
          <div className="conn-text">
            <strong>{status}</strong>
            <span>
              {isConnected
                ? `${activeHardware} · ${getTransportLabel(activeTransport)} · ${activeVariantLabel}`
                : 'Use USB for first flash and recovery. Add HC-05 only after the wired install is stable.'}
            </span>
            <div className="connection-meta-row">
              <span className="badge mono">{transportCapability || 'usb'}</span>
              <span className="badge mono">{getInstallLabel(installReadiness)}</span>
              <span className="badge mono">Service path: USB</span>
            </div>
          </div>
        </div>

        <div className="conn-right dashboard-connect-actions">
          {capabilities.canUseWebSerial ? (
            !isConnected ? (
              <>
                <button className="btn-primary dashboard-connect-btn" onClick={() => connect('usb')}>
                  Connect USB
                </button>
                <button className="btn-ghost dashboard-connect-btn dashboard-secondary-btn" onClick={() => connect('bluetooth')}>
                  Connect HC-05
                </button>
              </>
            ) : (
              <button className="btn-danger dashboard-connect-btn" onClick={disconnect}>Disconnect</button>
            )
          ) : (
            <div className="dashboard-compat-inline">
              <strong>Serial control unavailable here</strong>
              <span>{capabilities.compatibilitySummary}</span>
            </div>
          )}
        </div>
      </div>

      {isCompact ? <MobilePanelTabs activePanel={mobilePanel} onChange={setMobilePanel} /> : null}

      {!capabilities.supportsRuntimeControl && (
        <div className="panel dashboard-compat-panel">
          <div className="panel-head">
            <h2>Phone Compatibility</h2>
            <span className="badge mono">{capabilities.canUseWebSerial ? 'Limited' : 'Guide only'}</span>
          </div>
          <div className="dashboard-compat-copy">
            <p>{capabilities.compatibilitySummary}</p>
            <p>Use desktop Chrome or Edge for the first flash over USB. The preferred phone runtime path is Android Chrome with the paired HC-05 serial port after the wired install is already stable.</p>
          </div>
        </div>
      )}

      {showControl && (
        <>
          <div className="dashboard-sticky-strip panel">
            <div className="dashboard-sticky-grid">
              <div className="dashboard-sticky-stat">
                <span>Transport</span>
                <strong>{getTransportLabel(activeTransport)}</strong>
              </div>
              <div className="dashboard-sticky-stat">
                <span>Variant</span>
                <strong>{activeVariantLabel}</strong>
              </div>
              <div className="dashboard-sticky-stat">
                <span>Readiness</span>
                <strong>{getInstallLabel(installReadiness)}</strong>
              </div>
            </div>
            <div className="dashboard-quick-actions">
              {quickActions.map((action) => (
                <button
                  key={action.id}
                  className="btn-sm"
                  disabled={!isConnected}
                  onClick={action.run}
                >
                  {action.label}
                </button>
              ))}
            </div>
          </div>

          <div className="status-grid">
            <div className="dash-card">
              <div className="card-icon">🧰</div>
              <div className="card-body">
                <span className="card-label">Board</span>
                <span className="card-value">{activeHardware}</span>
              </div>
            </div>

            <div className="dash-card">
              <div className="card-icon">🚗</div>
              <div className="card-body">
                <span className="card-label">Vehicle Variant</span>
                <span className="card-value">{activeVariantLabel}</span>
              </div>
            </div>

            <div className="dash-card">
              <div className="card-icon">🔧</div>
              <div className="card-body">
                <span className="card-label">CAN Driver</span>
                <span className="card-value">{activeDriver}</span>
              </div>
            </div>

            <div className="dash-card">
              <div className="card-icon">🌐</div>
              <div className="card-body">
                <span className="card-label">Streaming</span>
                <span className={`card-value ${isStreaming ? 'val-on' : 'val-off'}`}>
                  {isStreaming ? 'LIVE' : 'IDLE'}
                </span>
              </div>
              <div className={`card-indicator ${isStreaming ? 'on' : 'off'}`}></div>
            </div>

            <div className="dash-card">
              <div className="card-icon">⏱️</div>
              <div className="card-body">
                <span className="card-label">Board Uptime</span>
                <span className="card-value">{formatUptime(telemetry.uptimeMs)}</span>
              </div>
            </div>

            <div className="dash-card">
              <div className="card-icon">📡</div>
              <div className="card-body">
                <span className="card-label">Avg Msg / sec</span>
                <span className="card-value">{telemetry.rate || '—'}</span>
              </div>
            </div>
          </div>

          <div className="panel dashboard-variant-panel">
            <div className="panel-head">
              <h2>Vehicle Variant</h2>
              <span className="badge mono">{activeVariantLabel}</span>
            </div>

            <div className="dashboard-variant-copy">
              <p>One shared Uno firmware image includes every handler. Keep USB as the recovery path, and switch `HW4`, `HW3`, or `Legacy` at runtime instead of reflashing.</p>
            </div>

            <div className="dashboard-variant-actions">
              {BOARD_VARIANTS.map((variantOption) => (
                <button
                  key={variantOption.id}
                  className={activeVariant === variantOption.id ? 'btn-primary' : 'btn-ghost'}
                  disabled={!isConnected}
                  onClick={() => sendCommand(BoardCommands.variant(variantOption.id))}
                >
                  {variantOption.label}
                </button>
              ))}
            </div>
          </div>

          <PackagePanels board={board} packages={packages} isCompact={isCompact} />
        </>
      )}

      {showMonitor && (
        <>
          <div className="panel log-panel">
            <div className="panel-head">
              <h2>Live CAN Frames</h2>
              <div className="log-actions">
                <button
                  className="btn-sm"
                  disabled={!isConnected}
                  onClick={() => sendCommand(BoardCommands.stream(!isStreaming))}
                >
                  {isStreaming ? 'Stop Stream' : 'Start Stream'}
                </button>
                <button className="btn-sm btn-ghost" onClick={clearFrames}>Clear</button>
                <span className="badge">{frameCount} frames received</span>
              </div>
            </div>

            <div className="frame-table-wrap" style={{ height: isCompact ? 'auto' : '300px', overflowY: 'auto' }}>
              {visibleFrames.length === 0 ? (
                <div className="empty-state">Connect to the board and start frame streaming.</div>
              ) : isCompact ? (
                <FrameList frames={visibleFrames.slice(0, 24)} />
              ) : (
                <>
                  <div className="frame-table-header">
                    <span className="col-time">Time</span>
                    <span className="col-dir">Dir</span>
                    <span className="col-id">ID</span>
                    <span className="col-hex">Hex</span>
                    <span className="col-dlc">DLC</span>
                    <span className="col-data">Data</span>
                  </div>
                  <div className="frame-table-body">
                    {visibleFrames.map((frame, index) => (
                      <div className="frame-row" key={`${frame.seenAt}-${frame.id}-${index}`}>
                        <span className="col-time">{frame.ts}</span>
                        <span className="col-dir" style={{ color: frame.dir === 'tx' ? 'var(--amber)' : 'var(--blue)' }}>
                          {frame.dir.toUpperCase()}
                        </span>
                        <span className="col-id">{frame.id}</span>
                        <span className="col-hex">0x{frame.id.toString(16).toUpperCase()}</span>
                        <span className="col-dlc">{frame.dlc}</span>
                        <span className="col-data mono">
                          {frame.bytes.length > 0
                            ? frame.bytes.map((byte) => byte.toString(16).padStart(2, '0').toUpperCase()).join(' ')
                            : '—'}
                        </span>
                      </div>
                    ))}
                  </div>
                </>
              )}
            </div>
          </div>

          <div className="panel dashboard-health-panel">
            <div className="panel-head">
              <h2>Connection Health</h2>
              <span className="badge mono">{getTransportLabel(activeTransport)}</span>
            </div>
            <div className="dashboard-health-grid">
              <div className="dashboard-health-card">
                <span>Service path</span>
                <strong>USB first</strong>
                <p>Use USB for the first flash, first diagnostics, and recovery even if HC-05 runtime control is available later.</p>
              </div>
              <div className="dashboard-health-card">
                <span>Install state</span>
                <strong>{getInstallLabel(installReadiness)}</strong>
                <p>{installReadiness === 'runtime-ready'
                  ? 'CAN traffic has been observed and the board is in live runtime operation.'
                  : installReadiness === 'installed-power-ready'
                    ? 'The CAN driver is healthy. Finish X179 CAN validation to confirm live runtime traffic.'
                    : 'Keep the board on the desk and finish the USB-first bring-up before moving to the vehicle harness.'}</p>
              </div>
            </div>
          </div>
        </>
      )}

      {showAdvanced && (
        <div className="panel console-panel dashboard-advanced-panel">
          <div className="panel-head">
            <h2>Advanced Console</h2>
            <div className="log-actions">
              <span className="badge mono">JSON + TEXT</span>
              <button className="btn-sm btn-ghost" onClick={clearConsole}>Clear</button>
            </div>
          </div>

          <div
            className="frame-table-wrap"
            style={{
              height: isCompact ? '220px' : '180px',
              overflowY: 'auto',
              padding: '12px',
              display: 'flex',
              flexDirection: 'column',
              gap: '8px',
            }}
          >
            {consoleLines.length === 0 ? (
              <div className="empty-state">Board messages will appear here after you connect.</div>
            ) : (
              consoleLines.map((line) => (
                <div
                  key={line.id}
                  className="mono"
                  style={{
                    color: line.type === 'error' ? 'var(--danger)' : line.type === 'tx' ? 'var(--amber)' : 'var(--text)',
                    fontSize: '0.85rem',
                    display: 'flex',
                    gap: '10px',
                  }}
                >
                  <span style={{ color: 'var(--text-muted)', minWidth: '86px' }}>{line.timestamp}</span>
                  <span>{line.text}</span>
                </div>
              ))
            )}
          </div>

          <div className="console-input">
            <input
              type="text"
              placeholder="Type command: ping, status, stream:on, fsd:on, profile:2, variant:hw4"
              value={commandInput}
              onChange={(event) => setCommandInput(event.target.value)}
              disabled={!isConnected}
              onKeyDown={(event) => {
                if (event.key === 'Enter') {
                  handleSend();
                }
              }}
            />
            <button className="btn-sm" disabled={!isConnected} onClick={handleSend}>Send</button>
          </div>
        </div>
      )}
    </div>
  );
}
