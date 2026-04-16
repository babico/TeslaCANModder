import { useState, useEffect, useMemo, useRef } from 'react';
import type { CanFrame } from '@teslacanmodder/protocol';
import './FrameTable.css';

interface CanDbEntry {
  name: string;
  signals?: string[];
}

export interface FilterPreset {
  label: string;
  bus?: number;
  dir?: string;
  idMin?: number;
  idMax?: number;
}

export const BUILT_IN_PRESETS: FilterPreset[] = [
  { label: 'All Frames' },
  { label: 'FSD Bus Only', bus: 0 },
  { label: 'Vehicle Bus Only', bus: 1 },
  { label: 'Body Bus Only', bus: 2 },
  { label: 'RX Only', dir: 'rx' },
  { label: 'TX Only', dir: 'tx' },
];

export function framesToCsv(frames: CanFrame[]): string {
  const header = 'Time,Bus,BusName,Dir,ID,DLC,Data';
  const rows = frames.map(f =>
    `${f.ts},${f.bus},${f.busName},${f.dir},0x${f.id.toString(16).toUpperCase()},${f.dlc},${f.data || ''}`
  );
  return [header, ...rows].join('\n');
}

export function framesToJson(frames: CanFrame[]): string {
  return JSON.stringify(frames, null, 2);
}

function downloadBlob(content: string, filename: string, mime: string) {
  const blob = new Blob([content], { type: mime });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

interface FrameTableProps {
  frames: CanFrame[];
  frameCount: number;
  onClear: () => void;
}

export default function FrameTable({ frames, frameCount, onClear }: FrameTableProps) {
  const [canDb, setCanDb] = useState<Record<string, CanDbEntry> | null>(null);
  const [expandedKey, setExpandedKey] = useState<string | null>(null);
  const [paused, setPaused] = useState(false);
  const [activePreset, setActivePreset] = useState(0);
  const [idFilter, setIdFilter] = useState('');
  const pausedFramesRef = useRef<CanFrame[]>([]);

  useEffect(() => {
    fetch('/can_frames_mcu2.json')
      .then(r => r.ok ? r.json() : null)
      .then((data: Record<string, CanDbEntry> | null) => setCanDb(data))
      .catch(() => {});
  }, []);

  // Snapshot frames when pausing
  useEffect(() => {
    if (paused) {
      pausedFramesRef.current = frames;
    }
  }, [paused]); // eslint-disable-line react-hooks/exhaustive-deps

  const displayFrames = paused ? pausedFramesRef.current : frames;

  const filteredFrames = useMemo(() => {
    const preset = BUILT_IN_PRESETS[activePreset];
    let result = displayFrames;
    if (preset.bus !== undefined) {
      result = result.filter(f => f.bus === preset.bus);
    }
    if (preset.dir !== undefined) {
      result = result.filter(f => f.dir === preset.dir);
    }
    if (idFilter.trim()) {
      const parsed = parseInt(idFilter.trim(), 16);
      if (!isNaN(parsed)) {
        result = result.filter(f => f.id === parsed);
      }
    }
    return result;
  }, [displayFrames, activePreset, idFilter]);

  const lookupFrame = (id: number): CanDbEntry | null => canDb ? canDb[String(id)] ?? null : null;

  const handleExportCsv = () => {
    const csv = framesToCsv(filteredFrames);
    downloadBlob(csv, `can-frames-${Date.now()}.csv`, 'text/csv');
  };

  const handleExportJson = () => {
    const json = framesToJson(filteredFrames);
    downloadBlob(json, `can-frames-${Date.now()}.json`, 'application/json');
  };

  return (
    <div className="panel frame-panel">
      <div className="panel-header">
        <h3>Live CAN Frames</h3>
        <div className="panel-actions">
          <button
            className={`btn btn-sm ${paused ? 'btn-primary' : 'btn-ghost'}`}
            onClick={() => setPaused(p => !p)}
          >
            {paused ? 'Resume' : 'Pause'}
          </button>
          <button className="btn btn-sm btn-ghost" onClick={onClear}>Clear</button>
          <span className="badge">{frameCount} frames</span>
        </div>
      </div>

      <div className="frame-toolbar">
        <div className="frame-presets">
          {BUILT_IN_PRESETS.map((preset, i) => (
            <button
              key={preset.label}
              className={`btn btn-sm ${activePreset === i ? 'btn-primary' : 'btn-ghost'}`}
              onClick={() => setActivePreset(i)}
            >
              {preset.label}
            </button>
          ))}
        </div>
        <div className="frame-toolbar-right">
          <input
            className="frame-id-filter"
            type="text"
            placeholder="Filter by hex ID…"
            value={idFilter}
            onChange={e => setIdFilter(e.target.value)}
          />
          <button className="btn btn-sm btn-ghost" disabled={filteredFrames.length === 0} onClick={handleExportCsv}>CSV</button>
          <button className="btn btn-sm btn-ghost" disabled={filteredFrames.length === 0} onClick={handleExportJson}>JSON</button>
        </div>
      </div>
      
      <div className="frame-table-wrap">
        {filteredFrames.length === 0 ? (
          <div className="empty-state">
            {frames.length === 0
              ? 'Connect and start streaming to see CAN frames'
              : 'No frames match the current filter'}
          </div>
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
              {filteredFrames.map((frame) => {
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
