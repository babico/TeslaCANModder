import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { toolkitPackages } from '../packages';
import { BoardCommands, BOARD_VARIANTS } from '../lib/board/commands';

const DASHBOARD_LAYOUT_STORAGE_KEY = 'tesla-can-modder.dashboard-layout-v3';
const GRID_ROW_HEIGHT = 10;
const GRID_GAP = 14;
const MIN_TILE_COLS = 1;
const MAX_TILE_COLS = 12;
const MIN_TILE_HEIGHT = 96;

function readSavedDashboardLayout() {
  if (typeof window === 'undefined') {
    return { order: [], hidden: [], sizes: {} };
  }

  try {
    const raw = window.localStorage.getItem(DASHBOARD_LAYOUT_STORAGE_KEY);

    if (!raw) {
      return { order: [], hidden: [], sizes: {} };
    }

    const parsed = JSON.parse(raw);
    return {
      order: Array.isArray(parsed.order) ? parsed.order : [],
      hidden: Array.isArray(parsed.hidden) ? parsed.hidden : [],
      sizes: parsed.sizes && typeof parsed.sizes === 'object' ? parsed.sizes : {},
    };
  } catch {
    return { order: [], hidden: [], sizes: {} };
  }
}

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

function VisibilityIcon({ hidden }) {
  return <i className={`fa-solid ${hidden ? 'fa-eye' : 'fa-eye-slash'}`} aria-hidden="true" />;
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
        <div key={packageDefinition.id} className="dashboard-tile dashboard-tile--package">
          <DashboardPanel packageState={packages[packageDefinition.id]} board={board} />
        </div>
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

function StatusTile({ icon, label, value, note, tone, stateClass }) {
  return (
    <div className={`dash-card ${tone ? `dash-card--${tone}` : ''}`}>
      <div className="card-icon">{icon}</div>
      <div className="card-body">
        <span className="card-label">{label}</span>
        <span className={`card-value ${stateClass || ''}`}>{value}</span>
        {note ? <span className="card-footnote">{note}</span> : null}
      </div>
      {stateClass ? <div className={`card-indicator ${stateClass === 'val-on' ? 'on' : 'off'}`}></div> : null}
    </div>
  );
}

function VariantPanel({ activeVariant, activeVariantLabel, isConnected, sendCommand }) {
  return (
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
  );
}

function HealthPanel({ activeTransport, installReadiness }) {
  return (
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
  );
}

function AdvancedConsole({
  isCompact,
  consoleLines,
  clearConsole,
  commandInput,
  onCommandInputChange,
  onSend,
  isConnected,
}) {
  return (
    <div className="panel console-panel dashboard-advanced-panel">
      <div className="panel-head">
        <h2>Advanced Console</h2>
        <div className="log-actions">
          <span className="badge mono">JSON + TEXT</span>
          <button className="btn-sm btn-ghost" onClick={clearConsole}>Clear</button>
        </div>
      </div>

      <div className={`console-body dashboard-console-body ${isCompact ? 'compact' : ''}`}>
        {consoleLines.length === 0 ? (
          <div className="empty-state">Board messages will appear here after you connect.</div>
        ) : (
          consoleLines.map((line) => (
            <div key={line.id} className={`console-line dashboard-console-line ${line.type || 'info'}`}>
              <span className="console-ts">{line.timestamp}</span>
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
          onChange={(event) => onCommandInputChange(event.target.value)}
          disabled={!isConnected}
          onKeyDown={(event) => {
            if (event.key === 'Enter') {
              onSend();
            }
          }}
        />
        <button className="btn-sm" disabled={!isConnected} onClick={onSend}>Send</button>
      </div>
    </div>
  );
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

function normalizeTileSize(size, fallback) {
  const cols = clamp(Number(size?.cols) || fallback.cols, MIN_TILE_COLS, MAX_TILE_COLS);
  const fallbackHeight = Number(fallback.height) || null;
  const requestedHeight = size?.manualHeight ? Number(size?.height) : fallbackHeight;
  const height = requestedHeight ? Math.max(MIN_TILE_HEIGHT, requestedHeight) : null;

  return { cols, height };
}

function TileFrame({
  tile,
  isHidden,
  registerTileFrame,
  onToggleTile,
  onResizeStart,
  onDragStart,
  onDragEnd,
  onDragOver,
  onDrop,
  isDragTarget,
  isDragging,
}) {
  return (
    <div
      className={`dashboard-tile ${tile.size.cols <= 3 ? 'dashboard-tile--narrow' : ''} ${tile.size.cols <= 5 ? 'dashboard-tile--compact' : ''} ${isDragTarget ? 'is-drag-target' : ''} ${isDragging ? 'is-dragging' : ''}`}
      draggable
      onDragStart={(event) => onDragStart(tile.id, event)}
      onDragEnd={onDragEnd}
      onDragOver={(event) => onDragOver(tile.id, event)}
      onDrop={() => onDrop(tile.id)}
      style={{
        '--tile-cols': tile.size.cols,
        '--tile-min-height': tile.size.height ? `${tile.size.height}px` : '0px',
        '--tile-row-span': tile.rowSpan,
      }}
    >
      <div className="dashboard-tile-frame" ref={(node) => registerTileFrame(tile.id, node)}>
        <div className="dashboard-tile-toolbar">
          <div className="dashboard-tile-toolbar-actions">
            <button
              className="btn-sm btn-ghost dashboard-visibility-toggle"
              onClick={() => onToggleTile(tile.id)}
              title={isHidden ? `Show ${tile.label}` : `Hide ${tile.label}`}
              aria-label={isHidden ? `Show ${tile.label}` : `Hide ${tile.label}`}
            >
              <VisibilityIcon hidden={isHidden} />
            </button>
          </div>
        </div>
        <div className="dashboard-tile-content">{tile.node}</div>
        <div
          className="dashboard-resize-handle"
          onPointerDown={(event) => onResizeStart(tile.id, event)}
          title="Resize tile"
        />
      </div>
    </div>
  );
}

function HiddenTileStrip({ tiles, onToggleTile }) {
  if (tiles.length === 0) {
    return null;
  }

  return (
    <div className="panel dashboard-hidden-strip">
      <div className="panel-head">
        <h2>Hidden Tiles</h2>
        <span className="badge mono">{tiles.length}</span>
      </div>
      <div className="dashboard-hidden-actions">
        {tiles.map((tile) => (
          <button key={tile.id} className="btn-sm btn-ghost" onClick={() => onToggleTile(tile.id)}>
            <VisibilityIcon hidden size={14} />
            <span>{tile.label}</span>
          </button>
        ))}
      </div>
    </div>
  );
}

function DesktopDashboardGrid({
  tiles,
  hiddenTiles,
  registerTileFrame,
  onToggleTile,
  onResizeStart,
  onDragStart,
  onDragEnd,
  onDragOver,
  onDrop,
  dragTileId,
  dragOverTileId,
}) {
  return (
    <>
      <div className="dashboard-grid">
        {tiles.map((tile) => (
          <TileFrame
            key={tile.id}
            tile={tile}
            isHidden={false}
            registerTileFrame={registerTileFrame}
            onToggleTile={onToggleTile}
            onResizeStart={onResizeStart}
            onDragStart={onDragStart}
            onDragEnd={onDragEnd}
            onDragOver={onDragOver}
            onDrop={onDrop}
            isDragging={dragTileId === tile.id}
            isDragTarget={dragOverTileId === tile.id}
          />
        ))}
      </div>
      <HiddenTileStrip tiles={hiddenTiles} onToggleTile={onToggleTile} />
    </>
  );
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
  const [layoutOrder, setLayoutOrder] = useState(() => readSavedDashboardLayout().order);
  const [hiddenTileIds, setHiddenTileIds] = useState(() => readSavedDashboardLayout().hidden);
  const [tileSizes, setTileSizes] = useState(() => readSavedDashboardLayout().sizes);
  const [dragTileId, setDragTileId] = useState(null);
  const [dragOverTileId, setDragOverTileId] = useState(null);
  const [resizeState, setResizeState] = useState(null);
  const [tileMeasurements, setTileMeasurements] = useState({});
  const tileFrameRefs = useRef(new Map());

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
  const visibleFrames = useMemo(() => (isConnected ? frames : []), [frames, isConnected]);
  const activeVariant = telemetry.variant || deviceInfo?.variant || null;
  const activeVariantLabel = activeVariant ? (VARIANT_LABELS[activeVariant] || activeVariant.toUpperCase()) : '—';
  const activeDriver = telemetry.driver || deviceInfo?.drv || '—';
  const activeHardware = telemetry.hardware || deviceInfo?.hw || '—';
  const installReadiness = telemetry.installReadiness || deviceInfo?.ready || null;
  const transportCapability = telemetry.transportCapability || deviceInfo?.cap || null;

  const handleSend = useCallback(() => {
    const nextCommand = commandInput.trim();
    if (!nextCommand || !isConnected) {
      return;
    }

    void sendCommand(nextCommand);
    setCommandInput('');
  }, [commandInput, isConnected, sendCommand]);

  const quickActions = useMemo(() => ([
    { id: 'ping', label: 'Ping', run: () => sendCommand(BoardCommands.ping()) },
    { id: 'status', label: 'Status', run: () => sendCommand(BoardCommands.status()) },
    { id: 'stream', label: isStreaming ? 'Stop Stream' : 'Start Stream', run: () => sendCommand(BoardCommands.stream(!isStreaming)) },
  ]), [isStreaming, sendCommand]);

  const desktopTiles = useMemo(() => {
    const baseTiles = [
      {
        id: 'command',
        label: 'Command Center',
        defaultSize: { cols: 8 },
        node: (
          <div className="dashboard-sticky-strip panel">
            <div className="panel-head">
              <h2>Command Center</h2>
              <span className="badge mono">{getTransportLabel(activeTransport)}</span>
            </div>
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
        ),
      },
      {
        id: 'board-status',
        label: 'Board',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="🧰"
            label="Board"
            value={activeHardware}
            note="Shared Uno image, runtime-selected behavior"
          />
        ),
      },
      {
        id: 'variant-status',
        label: 'Variant Status',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="🚗"
            label="Vehicle Variant"
            value={activeVariantLabel}
          />
        ),
      },
      {
        id: 'driver-status',
        label: 'Driver Status',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="🔧"
            label="CAN Driver"
            value={activeDriver}
          />
        ),
      },
      {
        id: 'stream-status',
        label: 'Stream Status',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="🌐"
            label="Streaming"
            value={isStreaming ? 'LIVE' : 'IDLE'}
            tone="signal"
            stateClass={isStreaming ? 'val-on' : 'val-off'}
          />
        ),
      },
      {
        id: 'uptime-status',
        label: 'Uptime Status',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="⏱️"
            label="Board Uptime"
            value={formatUptime(telemetry.uptimeMs)}
          />
        ),
      },
      {
        id: 'rate-status',
        label: 'Rate Status',
        defaultSize: { cols: 4 },
        node: (
          <StatusTile
            icon="📡"
            label="Avg Msg / sec"
            value={telemetry.rate || '—'}
            tone="accent"
          />
        ),
      },
      {
        id: 'frames',
        label: 'Live CAN Frames',
        defaultSize: { cols: 8, height: 520 },
        node: (
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

            <div className="frame-table-wrap dashboard-frame-table">
              {visibleFrames.length === 0 ? (
                <div className="empty-state">Connect to the board and start frame streaming.</div>
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
        ),
      },
      {
        id: 'variant',
        label: 'Vehicle Variant',
        defaultSize: { cols: 4 },
        node: (
          <VariantPanel
            activeVariant={activeVariant}
            activeVariantLabel={activeVariantLabel}
            isConnected={isConnected}
            sendCommand={sendCommand}
          />
        ),
      },
      {
        id: 'health',
        label: 'Connection Health',
        defaultSize: { cols: 4 },
        node: <HealthPanel activeTransport={activeTransport} installReadiness={installReadiness} />,
      },
      {
        id: 'console',
        label: 'Advanced Console',
        defaultSize: { cols: 8, height: 260 },
        node: (
          <AdvancedConsole
            isCompact={false}
            consoleLines={consoleLines}
            clearConsole={clearConsole}
            commandInput={commandInput}
            onCommandInputChange={setCommandInput}
            onSend={handleSend}
            isConnected={isConnected}
          />
        ),
      },
    ];

    const packageTiles = toolkitPackages
      .filter((packageDefinition) => packageDefinition.DashboardPanel)
      .map((packageDefinition) => {
        const DashboardPanel = packageDefinition.DashboardPanel;

        return {
          id: `package-${packageDefinition.id}`,
          label: packageDefinition.label || packageDefinition.id,
          defaultSize: { cols: 4, height: 420 },
          node: <DashboardPanel packageState={packages[packageDefinition.id]} board={board} />,
        };
      });

    return [...baseTiles, ...packageTiles];
  }, [
    activeDriver,
    activeHardware,
    activeTransport,
    activeVariant,
    activeVariantLabel,
    board,
    clearConsole,
    clearFrames,
    commandInput,
    consoleLines,
    frameCount,
    handleSend,
    installReadiness,
    isConnected,
    isStreaming,
    packages,
    quickActions,
    sendCommand,
    telemetry,
    visibleFrames,
  ]);

  const tileIds = useMemo(() => desktopTiles.map((tile) => tile.id), [desktopTiles]);

  const normalizedLayoutOrder = useMemo(
    () => [...layoutOrder.filter((id) => tileIds.includes(id)), ...tileIds.filter((id) => !layoutOrder.includes(id))],
    [layoutOrder, tileIds],
  );

  const normalizedHiddenTileIds = useMemo(
    () => hiddenTileIds.filter((id) => tileIds.includes(id)),
    [hiddenTileIds, tileIds],
  );

  const normalizedTileSizes = useMemo(() => {
    const normalized = {};

    desktopTiles.forEach((tile) => {
      normalized[tile.id] = normalizeTileSize(tileSizes[tile.id], tile.defaultSize);
    });

    return normalized;
  }, [desktopTiles, tileSizes]);

  useEffect(() => {
    window.localStorage.setItem(
      DASHBOARD_LAYOUT_STORAGE_KEY,
      JSON.stringify({
        order: normalizedLayoutOrder,
        hidden: normalizedHiddenTileIds,
        sizes: normalizedTileSizes,
      }),
    );
  }, [normalizedHiddenTileIds, normalizedLayoutOrder, normalizedTileSizes]);

  const orderedDesktopTiles = useMemo(() => {
    const tileMap = new Map(desktopTiles.map((tile) => [tile.id, tile]));
    return normalizedLayoutOrder
      .map((id) => tileMap.get(id))
      .filter(Boolean)
      .map((tile) => ({
        ...tile,
        size: normalizedTileSizes[tile.id] || normalizeTileSize(undefined, tile.defaultSize),
      }));
  }, [desktopTiles, normalizedLayoutOrder, normalizedTileSizes]);

  const registerTileFrame = useCallback((tileId, node) => {
    if (node) {
      tileFrameRefs.current.set(tileId, node);
      return;
    }

    tileFrameRefs.current.delete(tileId);
  }, []);

  const visibleDesktopTiles = useMemo(
    () => orderedDesktopTiles.filter((tile) => !normalizedHiddenTileIds.includes(tile.id)),
    [normalizedHiddenTileIds, orderedDesktopTiles],
  );

  const hiddenDesktopTiles = useMemo(
    () => orderedDesktopTiles.filter((tile) => normalizedHiddenTileIds.includes(tile.id)),
    [normalizedHiddenTileIds, orderedDesktopTiles],
  );

  useEffect(() => {
    if (typeof window === 'undefined') {
      return undefined;
    }

    const observer = new ResizeObserver((entries) => {
      setTileMeasurements((current) => {
        const next = { ...current };
        let changed = false;

        entries.forEach((entry) => {
          const tileId = [...tileFrameRefs.current.entries()].find(([, node]) => node === entry.target)?.[0];

          if (!tileId) {
            return;
          }

          const measuredHeight = Math.max(MIN_TILE_HEIGHT, Math.ceil(entry.contentRect.height));

          if (next[tileId] !== measuredHeight) {
            next[tileId] = measuredHeight;
            changed = true;
          }
        });

        return changed ? next : current;
      });
    });

    visibleDesktopTiles.forEach((tile) => {
      const node = tileFrameRefs.current.get(tile.id);

      if (node) {
        observer.observe(node);
      }
    });

    return () => observer.disconnect();
  }, [visibleDesktopTiles]);

  const toggleTile = (tileId) => {
    setHiddenTileIds((current) => (
      current.includes(tileId)
        ? current.filter((id) => id !== tileId)
        : [...current, tileId]
    ));
  };

  useEffect(() => {
    if (!resizeState) {
      return undefined;
    }

    const handlePointerMove = (event) => {
      const nextCols = clamp(
        Math.round(resizeState.initialSize.cols + (event.clientX - resizeState.startX) / 64),
        MIN_TILE_COLS,
        MAX_TILE_COLS,
      );
      const nextHeight = Math.max(MIN_TILE_HEIGHT, Math.round(resizeState.initialHeight + (event.clientY - resizeState.startY)));

      setTileSizes((current) => ({
        ...current,
        [resizeState.tileId]: {
          cols: nextCols,
          height: nextHeight,
          manualHeight: true,
        },
      }));
    };

    const handlePointerUp = () => {
      setResizeState(null);
    };

    window.addEventListener('pointermove', handlePointerMove);
    window.addEventListener('pointerup', handlePointerUp, { once: true });

    return () => {
      window.removeEventListener('pointermove', handlePointerMove);
      window.removeEventListener('pointerup', handlePointerUp);
    };
  }, [resizeState]);

  const startResizeTile = (tileId, event) => {
    event.preventDefault();
    event.stopPropagation();
    setResizeState({
      tileId,
      startX: event.clientX,
      startY: event.clientY,
      initialSize: normalizedTileSizes[tileId],
      initialHeight: normalizedTileSizes[tileId]?.height || tileMeasurements[tileId] || MIN_TILE_HEIGHT,
    });
  };

  const sizedVisibleDesktopTiles = useMemo(
    () => visibleDesktopTiles.map((tile) => {
      const measuredHeight = tileMeasurements[tile.id] || tile.size.height || tile.defaultSize.height || MIN_TILE_HEIGHT;
      const rowSpan = Math.max(8, Math.ceil((measuredHeight + GRID_GAP) / (GRID_ROW_HEIGHT + GRID_GAP)));

      return {
        ...tile,
        rowSpan,
      };
    }),
    [tileMeasurements, visibleDesktopTiles],
  );

  const moveTileToTarget = (sourceId, targetId) => {
    if (!sourceId || !targetId || sourceId === targetId) {
      return;
    }

    setLayoutOrder((current) => {
      const sourceIndex = current.indexOf(sourceId);
      const targetIndex = current.indexOf(targetId);

      if (sourceIndex === -1 || targetIndex === -1) {
        return current;
      }

      const next = [...current];
      const [tile] = next.splice(sourceIndex, 1);
      next.splice(targetIndex, 0, tile);
      return next;
    });
  };

  const handleTileDragStart = (tileId, event) => {
    setDragTileId(tileId);
    event.dataTransfer.effectAllowed = 'move';
    event.dataTransfer.setData('text/plain', tileId);
  };

  const handleTileDragOver = (tileId, event) => {
    event.preventDefault();
    if (dragTileId && dragTileId !== tileId) {
      setDragOverTileId(tileId);
    }
  };

  const handleTileDrop = (tileId) => {
    moveTileToTarget(dragTileId, tileId);
    setDragTileId(null);
    setDragOverTileId(null);
  };

  const handleTileDragEnd = () => {
    setDragTileId(null);
    setDragOverTileId(null);
  };

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

      {!isCompact ? (
        <DesktopDashboardGrid
          tiles={sizedVisibleDesktopTiles}
          hiddenTiles={hiddenDesktopTiles}
          registerTileFrame={registerTileFrame}
          onToggleTile={toggleTile}
          onResizeStart={startResizeTile}
          onDragStart={handleTileDragStart}
          onDragEnd={handleTileDragEnd}
          onDragOver={handleTileDragOver}
          onDrop={handleTileDrop}
          dragTileId={dragTileId}
          dragOverTileId={dragOverTileId}
        />
      ) : (
        <>
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

              <VariantPanel
                activeVariant={activeVariant}
                activeVariantLabel={activeVariantLabel}
                isConnected={isConnected}
                sendCommand={sendCommand}
              />

              <PackagePanels board={board} packages={packages} isCompact />
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

                <div className="frame-table-wrap dashboard-frame-table compact">
                  {visibleFrames.length === 0 ? (
                    <div className="empty-state">Connect to the board and start frame streaming.</div>
                  ) : (
                    <FrameList frames={visibleFrames.slice(0, 24)} />
                  )}
                </div>
              </div>

              <HealthPanel activeTransport={activeTransport} installReadiness={installReadiness} />
            </>
          )}

          {showAdvanced && (
            <AdvancedConsole
              isCompact
              consoleLines={consoleLines}
              clearConsole={clearConsole}
              commandInput={commandInput}
              onCommandInputChange={setCommandInput}
              onSend={handleSend}
              isConnected={isConnected}
            />
          )}
        </>
      )}
    </div>
  );
}
