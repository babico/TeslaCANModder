import { useEffect, useMemo, useState } from 'react';
import { BoardCommands } from '../lib/board/commands';
import {
  CAN_DECODER_DATASETS,
  DEFAULT_CAN_DECODER_DATASET,
  describeDecodedFrame,
  getDecoderDatasetLabel,
  loadDecoderDataset,
} from '../lib/can/decoder';

function buildFrameSummary(frames, decoderIndex) {
  const grouped = new Map();

  for (const frame of frames) {
    const key = `${frame.dir}:${frame.id}`;
    const existing = grouped.get(key);

    if (existing) {
      existing.count += 1;
      if (frame.seenAt > existing.lastSeen) {
        existing.latest = frame;
        existing.lastSeen = frame.seenAt;
      }
      continue;
    }

    grouped.set(key, {
      key,
      id: frame.id,
      dir: frame.dir,
      meaning: describeDecodedFrame(decoderIndex, frame.id),
      count: 1,
      lastSeen: frame.seenAt,
      latest: frame,
    });
  }

  return [...grouped.values()].sort((left, right) => right.lastSeen - left.lastSeen);
}

function byteToBits(byteValue) {
  return byteValue.toString(2).padStart(8, '0').split('');
}

function MeaningSection({ entries }) {
  if (!entries.length) {
    return (
      <div className="explorer-meaning-empty">
        No known frame meaning was found in the selected legacy dataset for this CAN ID.
      </div>
    );
  }

  return (
    <div className="explorer-meaning-grid">
      {entries.map((entry, index) => (
        <div key={`${entry.frameName}-${entry.busName}-${entry.busId}-${index}`} className="explorer-meaning-card">
          <div className="explorer-meaning-head">
            <strong>{entry.frameName}</strong>
            <span className="badge mono">{entry.hex}</span>
          </div>
          <div className="explorer-meaning-meta">
            <span className="meta-pill">{entry.busName} ({entry.busId})</span>
            <span className="meta-pill">{entry.signalCount} signals</span>
          </div>
          <div className="explorer-signal-list">
            {entry.signals.length === 0 ? (
              <div className="explorer-signal-empty">No reduced signal metadata stored for this frame.</div>
            ) : (
              entry.signals.map((signal) => (
                <div key={`${entry.frameName}-${signal.name}`} className="explorer-signal-card">
                  <div className="explorer-signal-title">
                    <strong>{signal.name}</strong>
                    {signal.enumMap ? <span className="badge mono">{signal.enumMap}</span> : null}
                  </div>
                  {signal.values.length > 0 ? (
                    <div className="explorer-signal-values">
                      {signal.values.map((value) => (
                        <span key={`${signal.name}-${value.valueHex}-${value.label}`} className="chip">
                          <code>{value.valueHex || value.valueDec}</code> {value.label}
                        </span>
                      ))}
                    </div>
                  ) : (
                    <p className="status-note">{signal.note || 'No enumerated values in the legacy dataset for this signal.'}</p>
                  )}
                </div>
              ))
            )}
          </div>
        </div>
      ))}
    </div>
  );
}

export default function CanExplorer({ board }) {
  const { isConnected, isStreaming, monitor, sendCommand } = board;
  const [search, setSearch] = useState('');
  const [datasetKey, setDatasetKey] = useState(DEFAULT_CAN_DECODER_DATASET);
  const [selectedKey, setSelectedKey] = useState(null);
  const [decoderIndex, setDecoderIndex] = useState(null);
  const [decoderError, setDecoderError] = useState(null);
  const [decoderLoading, setDecoderLoading] = useState(true);

  useEffect(() => {
    const ac = new AbortController();
    setDecoderLoading(true);
    setDecoderError(null);

    loadDecoderDataset(datasetKey, ac.signal)
      .then((index) => {
        if (ac.signal.aborted) {
          return;
        }
        setDecoderIndex(index);
        setDecoderLoading(false);
      })
      .catch((error) => {
        if (ac.signal.aborted) {
          return;
        }
        setDecoderError(error.message || String(error));
        setDecoderIndex(null);
        setDecoderLoading(false);
      });

    return () => {
      ac.abort();
    };
  }, [datasetKey]);

  const summaries = useMemo(
    () => buildFrameSummary(monitor.frames, decoderIndex),
    [decoderIndex, monitor.frames],
  );

  const visibleSummaries = useMemo(() => {
    if (!isConnected) {
      return [];
    }

    const query = search.trim().toLowerCase();
    if (!query) {
      return summaries;
    }

    return summaries.filter((summary) => {
      const frame = summary.latest;
      return [
        summary.dir,
        summary.id.toString(),
        summary.id.toString(16),
        summary.meaning.map((entry) => `${entry.frameName} ${entry.busName} ${entry.hex}`).join(' '),
        frame.dataHex,
        frame.bytes.map((byte) => byte.toString(16).padStart(2, '0')).join(' '),
      ].some((value) => value.toLowerCase().includes(query));
    });
  }, [isConnected, search, summaries]);

  const selectedFrame = useMemo(() => {
    if (!isConnected || visibleSummaries.length === 0) {
      return null;
    }

    return visibleSummaries.find((summary) => summary.key === selectedKey) || visibleSummaries[0];
  }, [isConnected, selectedKey, visibleSummaries]);

  return (
    <div className="explorer-layout" style={{ padding: '24px', height: '100%', boxSizing: 'border-box' }}>
      <aside className="panel panel-left" style={{ height: '100%', overflowY: 'auto' }}>
        <div className="panel-head">
          <h2>Observed Frames</h2>
          <span className="badge">{visibleSummaries.length} IDs</span>
        </div>

        <div className="controls">
          <label className="field">
            <span>Search</span>
            <input
              type="search"
              placeholder="ID, hex bytes, direction..."
              value={search}
              onChange={(event) => setSearch(event.target.value)}
              disabled={!isConnected}
            />
          </label>
          <label className="field">
            <span>Meaning Dataset</span>
            <select
              value={datasetKey}
              onChange={(event) => {
                setDatasetKey(event.target.value);
                setDecoderLoading(true);
                setDecoderError(null);
              }}
            >
              {CAN_DECODER_DATASETS.map((dataset) => (
                <option key={dataset.key} value={dataset.key}>{dataset.label}</option>
              ))}
            </select>
          </label>
          <div className="explorer-dataset-status">
            <span className="badge mono">{getDecoderDatasetLabel(datasetKey)}</span>
            {decoderLoading ? <span className="status-note">Loading meaning dataset…</span> : null}
            {decoderError ? <span className="status-note warn">{decoderError}</span> : null}
          </div>
        </div>

        {!isConnected ? (
          <div className="empty-state" style={{ padding: '16px' }}>
            Connect to the board to inspect live CAN traffic.
          </div>
        ) : !isStreaming ? (
          <div className="empty-state" style={{ padding: '16px' }}>
            Frame streaming is off. Start it to populate the explorer.
            <div style={{ marginTop: '12px' }}>
              <button className="btn-sm" onClick={() => sendCommand(BoardCommands.stream(true))}>▶ Start Stream</button>
            </div>
          </div>
        ) : visibleSummaries.length === 0 ? (
          <div className="empty-state" style={{ padding: '16px' }}>
            Waiting for matching CAN frames.
          </div>
        ) : (
          <div className="frame-list">
            {visibleSummaries.map((summary) => (
              <button
                key={summary.key}
                type="button"
                className={`frame-item ${selectedFrame?.key === summary.key ? 'active' : ''}`}
                onClick={() => setSelectedKey(summary.key)}
                style={{
                  padding: '12px',
                  borderBottom: '1px solid var(--border)',
                  cursor: 'pointer',
                  width: '100%',
                  textAlign: 'left',
                  background: 'transparent',
                  borderLeft: 0,
                  borderRight: 0,
                  borderTop: 0,
                }}
              >
                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '4px' }}>
                  <strong style={{ color: 'var(--blue)' }}>0x{summary.id.toString(16).toUpperCase()}</strong>
                  <span className="badge mono">{summary.dir.toUpperCase()}</span>
                </div>
                <div className="frame-name">
                  {summary.meaning[0]?.frameName || 'Unknown frame'}
                </div>
                <div style={{ fontSize: '0.85rem', color: 'var(--text)' }}>
                  {summary.count} recent frame{summary.count === 1 ? '' : 's'}
                </div>
                <div style={{ marginTop: '4px', color: 'var(--text-muted)', fontSize: '0.8rem' }}>
                  {summary.meaning[0] ? `${summary.meaning[0].busName} · ` : ''}Last payload: {summary.latest.dataHex || '—'}
                </div>
              </button>
            ))}
          </div>
        )}
      </aside>

      <section className="panel panel-right" style={{ height: '100%', overflowY: 'auto' }}>
        {selectedFrame ? (
          <>
            <div className="panel-head">
              <div>
                <h2>Frame 0x{selectedFrame.id.toString(16).toUpperCase()}</h2>
                <p>
                  {selectedFrame.dir.toUpperCase()} · {selectedFrame.count} sample{selectedFrame.count === 1 ? '' : 's'} in recent buffer
                </p>
              </div>
              <span className="badge mono">{selectedFrame.latest.dlc} bytes</span>
            </div>

            <div style={{ padding: '16px 0' }}>
              <div className="panel" style={{ padding: '16px', marginBottom: '16px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', gap: '12px', flexWrap: 'wrap' }}>
                  <div>
                    <div style={{ color: 'var(--text-muted)', fontSize: '0.8rem', marginBottom: '4px' }}>Last Seen</div>
                    <div className="mono">{selectedFrame.latest.ts}</div>
                  </div>
                  <div>
                    <div style={{ color: 'var(--text-muted)', fontSize: '0.8rem', marginBottom: '4px' }}>Raw Hex</div>
                    <div className="mono">{selectedFrame.latest.dataHex || '—'}</div>
                  </div>
                </div>
              </div>

              <div className="panel explorer-meaning-panel" style={{ padding: '16px', marginBottom: '16px' }}>
                <div className="panel-head" style={{ padding: 0, borderBottom: 0, marginBottom: '12px' }}>
                  <div>
                    <h2>Known Meaning</h2>
                    <p>Frame names and signal hints from the selected legacy Tesla CAN Explorer dataset.</p>
                  </div>
                  <span className="badge mono">{selectedFrame.meaning.length} matches</span>
                </div>
                <MeaningSection entries={selectedFrame.meaning} />
              </div>

              <div className="signal-table-wrap">
                <table style={{ width: '100%', borderCollapse: 'collapse' }}>
                  <thead>
                    <tr style={{ textAlign: 'left', borderBottom: '1px solid var(--border)' }}>
                      <th style={{ padding: '8px' }}>Byte</th>
                      <th style={{ padding: '8px' }}>Hex</th>
                      <th style={{ padding: '8px' }}>Binary</th>
                      <th style={{ padding: '8px' }}>Decimal</th>
                    </tr>
                  </thead>
                  <tbody>
                    {selectedFrame.latest.bytes.map((byte, index) => (
                      <tr key={`${selectedFrame.key}-${index}`} style={{ borderBottom: '1px solid var(--bg-hover)' }}>
                        <td style={{ padding: '8px' }} className="mono">[{index}]</td>
                        <td style={{ padding: '8px' }} className="mono">{byte.toString(16).padStart(2, '0').toUpperCase()}</td>
                        <td style={{ padding: '8px' }} className="mono">{byteToBits(byte).join(' ')}</td>
                        <td style={{ padding: '8px' }} className="mono">{byte}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          </>
        ) : (
          <div
            className="empty-state"
            style={{ flex: 1, display: 'flex', alignItems: 'center', justifyContent: 'center', color: 'var(--text-muted)' }}
          >
            Select a frame from the left panel to inspect its latest payload.
          </div>
        )}
      </section>
    </div>
  );
}
