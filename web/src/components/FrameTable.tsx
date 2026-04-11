import { useState, useEffect } from 'react';
import type { CanFrame } from '@teslacanmodder/protocol';
import './FrameTable.css';

interface CanDbEntry {
  name: string;
  signals?: string[];
}

interface FrameTableProps {
  frames: CanFrame[];
  frameCount: number;
  onClear: () => void;
}

export default function FrameTable({ frames, frameCount, onClear }: FrameTableProps) {
  const [canDb, setCanDb] = useState<Record<string, CanDbEntry> | null>(null);
  const [expandedKey, setExpandedKey] = useState<string | null>(null);

  useEffect(() => {
    fetch('/can_frames_mcu2.json')
      .then(r => r.ok ? r.json() : null)
      .then((data: Record<string, CanDbEntry> | null) => setCanDb(data))
      .catch(() => {});
  }, []);

  const lookupFrame = (id: number): CanDbEntry | null => canDb ? canDb[String(id)] ?? null : null;

  return (
    <div className="panel frame-panel">
      <div className="panel-header">
        <h3>Live CAN Frames</h3>
        <div className="panel-actions">
          <button className="btn btn-sm btn-ghost" onClick={onClear}>Clear</button>
          <span className="badge">{frameCount} frames</span>
        </div>
      </div>
      
      <div className="frame-table-wrap">
        {frames.length === 0 ? (
          <div className="empty-state">Connect and start streaming to see CAN frames</div>
        ) : (
          <>
            <div className="frame-table-header">
              <span>Time</span>
              <span>Bus</span>
              <span>Dir</span>
              <span>ID</span>
              <span>Name</span>
              <span>DLC</span>
              <span>Data</span>
            </div>
            <div className="frame-table-body">
              {frames.map((frame) => {
                const decoded = lookupFrame(frame.id);
                return (
                  <div key={frame.key}>
                    <div
                      className={`frame-row ${decoded ? 'frame-row--decoded' : ''} ${expandedKey === frame.key ? 'frame-row--expanded' : ''}`}
                      onClick={() => decoded && setExpandedKey(expandedKey === frame.key ? null : frame.key)}
                    >
                      <span className="frame-time">{frame.ts}</span>
                      <span className={`frame-bus bus-${frame.bus}`}>{frame.busName}</span>
                      <span className={`frame-dir ${frame.dir}`}>{frame.dir.toUpperCase()}</span>
                      <span className="frame-id mono">0x{frame.id.toString(16).toUpperCase()}</span>
                      <span className="frame-name">{decoded ? decoded.name : '—'}</span>
                      <span className="frame-dlc">{frame.dlc}</span>
                      <span className="frame-data mono">{frame.data || '—'}</span>
                    </div>
                    {expandedKey === frame.key && decoded?.signals && (
                      <div className="frame-signals">
                        <span className="frame-signals__title">Signals ({decoded.signals.length}):</span>
                        <span className="frame-signals__list">{decoded.signals.join(', ')}</span>
                      </div>
                    )}
                  </div>
                );
              })}
            </div>
          </>
        )}
      </div>
    </div>
  );
}
