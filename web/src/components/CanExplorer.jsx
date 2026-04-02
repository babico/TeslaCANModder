import { useEffect, useMemo, useState } from 'react';
import { BoardCommands } from '../lib/board/commands';

function buildFrameSummary(frames) {
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

export default function CanExplorer({ board }) {
  const { isConnected, isStreaming, getFrames, sendCommand } = board;
  const [search, setSearch] = useState('');
  const [summaries, setSummaries] = useState([]);
  const [selectedKey, setSelectedKey] = useState(null);

  useEffect(() => {
    if (!isConnected) {
      return undefined;
    }

    const interval = setInterval(() => {
      setSummaries(buildFrameSummary(getFrames()));
    }, 180);

    return () => clearInterval(interval);
  }, [getFrames, isConnected]);

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
                <div style={{ fontSize: '0.85rem', color: 'var(--text)' }}>
                  {summary.count} recent frame{summary.count === 1 ? '' : 's'}
                </div>
                <div style={{ marginTop: '4px', color: 'var(--text-muted)', fontSize: '0.8rem' }}>
                  Last payload: {summary.latest.dataHex || '—'}
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
