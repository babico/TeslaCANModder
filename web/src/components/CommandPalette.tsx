import { useState, useEffect, useMemo, useRef, useCallback } from 'react';
import { commands } from '@teslacanmodder/protocol';
import './CommandPalette.css';

export interface PaletteEntry {
  id: string;
  label: string;
  category: string;
  command: () => string;
}

const PALETTE_ITEMS: PaletteEntry[] = [
  { id: 'ping', label: 'Ping', category: 'System', command: commands.ping },
  { id: 'status', label: 'Status', category: 'System', command: commands.status },
  { id: 'fsd-on', label: 'FSD Enable', category: 'FSD', command: () => commands.fsd(true) },
  { id: 'fsd-off', label: 'FSD Disable', category: 'FSD', command: () => commands.fsd(false) },
  { id: 'fsd-toggle', label: 'FSD Toggle', category: 'FSD', command: commands.fsdToggle },
  { id: 'nag-on', label: 'Nag Suppress Enable', category: 'Nag', command: () => commands.nag(true) },
  { id: 'nag-off', label: 'Nag Suppress Disable', category: 'Nag', command: () => commands.nag(false) },
  { id: 'nag-toggle', label: 'Nag Toggle', category: 'Nag', command: commands.nagToggle },
  { id: 'profile-0', label: 'Profile: Chill', category: 'Speed', command: () => commands.profile(0) },
  { id: 'profile-1', label: 'Profile: Normal', category: 'Speed', command: () => commands.profile(1) },
  { id: 'profile-2', label: 'Profile: Hurry', category: 'Speed', command: () => commands.profile(2) },
  { id: 'profile-3', label: 'Profile: Max', category: 'Speed', command: () => commands.profile(3) },
  { id: 'profile-4', label: 'Profile: Sloth', category: 'Speed', command: () => commands.profile(4) },
  { id: 'profile-auto', label: 'Profile: Auto', category: 'Speed', command: commands.profileAuto },
  { id: 'offset-auto', label: 'Offset: Auto', category: 'Speed', command: commands.offsetAuto },
  { id: 'stream-on', label: 'Stream On', category: 'Streaming', command: () => commands.stream(true) },
  { id: 'stream-off', label: 'Stream Off', category: 'Streaming', command: () => commands.stream(false) },
  { id: 'lock', label: 'Lock', category: 'Vehicle', command: commands.lock },
  { id: 'unlock', label: 'Unlock', category: 'Vehicle', command: commands.unlock },
  { id: 'horn', label: 'Horn', category: 'Vehicle', command: commands.horn },
  { id: 'frunk-open', label: 'Frunk Open', category: 'Vehicle', command: commands.frunkOpen },
  { id: 'frunk-close', label: 'Frunk Close', category: 'Vehicle', command: commands.frunkClose },
  { id: 'trunk-open', label: 'Trunk Open', category: 'Vehicle', command: commands.trunkOpen },
  { id: 'trunk-close', label: 'Trunk Close', category: 'Vehicle', command: commands.trunkClose },
  { id: 'mirror-fold', label: 'Mirror Fold', category: 'Vehicle', command: commands.mirrorFold },
  { id: 'mirror-unfold', label: 'Mirror Unfold', category: 'Vehicle', command: commands.mirrorUnfold },
  { id: 'vent-open', label: 'Vent Open', category: 'Vehicle', command: commands.ventOpen },
  { id: 'vent-close', label: 'Vent Close', category: 'Vehicle', command: commands.ventClose },
  { id: 'sentry-on', label: 'Sentry On', category: 'Vehicle', command: commands.sentryOn },
  { id: 'sentry-off', label: 'Sentry Off', category: 'Vehicle', command: commands.sentryOff },
  { id: 'climate-keep', label: 'Climate Keep', category: 'Climate', command: commands.climateKeep },
  { id: 'climate-off', label: 'Climate Off', category: 'Climate', command: commands.climateOff },
  { id: 'charge-start', label: 'Charge Start', category: 'Charge', command: commands.chargeStart },
  { id: 'charge-stop', label: 'Charge Stop', category: 'Charge', command: commands.chargeStop },
  { id: 'power-off', label: 'Power Off', category: 'Power', command: commands.powerOff },
  { id: 'power-ready', label: 'Power Ready', category: 'Power', command: commands.powerReady },
];

const RECENT_KEY = 'tcm-recent-commands';
const MAX_RECENT = 5;

function loadRecent(): string[] {
  try {
    const raw = localStorage.getItem(RECENT_KEY);
    return raw ? JSON.parse(raw) : [];
  } catch {
    return [];
  }
}

function saveRecent(ids: string[]) {
  localStorage.setItem(RECENT_KEY, JSON.stringify(ids.slice(0, MAX_RECENT)));
}

interface CommandPaletteProps {
  open: boolean;
  onClose: () => void;
  onCommand: (cmd: string) => void;
  connected: boolean;
}

export default function CommandPalette({ open, onClose, onCommand, connected }: CommandPaletteProps) {
  const [query, setQuery] = useState('');
  const [recentIds, setRecentIds] = useState<string[]>(loadRecent);
  const [selectedIndex, setSelectedIndex] = useState(0);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (open) {
      setQuery('');
      setSelectedIndex(0);
      setTimeout(() => inputRef.current?.focus(), 0);
    }
  }, [open]);

  const filtered = useMemo(() => {
    const q = query.toLowerCase();
    if (!q) return PALETTE_ITEMS;
    return PALETTE_ITEMS.filter(
      item => item.label.toLowerCase().includes(q) || item.category.toLowerCase().includes(q) || item.id.includes(q)
    );
  }, [query]);

  const recentItems = useMemo(() => {
    return recentIds
      .map(id => PALETTE_ITEMS.find(item => item.id === id))
      .filter((item): item is PaletteEntry => !!item);
  }, [recentIds]);

  const executeItem = useCallback((item: PaletteEntry) => {
    const cmd = item.command();
    onCommand(cmd);
    const updated = [item.id, ...recentIds.filter(id => id !== item.id)].slice(0, MAX_RECENT);
    setRecentIds(updated);
    saveRecent(updated);
    onClose();
  }, [onCommand, onClose, recentIds]);

  const handleKeyDown = useCallback((e: React.KeyboardEvent) => {
    if (e.key === 'Escape') {
      onClose();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex(i => Math.min(i + 1, filtered.length - 1));
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex(i => Math.max(i - 1, 0));
    } else if (e.key === 'Enter') {
      e.preventDefault();
      if (filtered[selectedIndex]) {
        executeItem(filtered[selectedIndex]);
      }
    }
  }, [onClose, filtered, selectedIndex, executeItem]);

  // Reset selection when filter changes
  useEffect(() => {
    setSelectedIndex(0);
  }, [query]);

  if (!open) return null;

  return (
    <div className="palette-overlay" onClick={onClose}>
      <div className="palette" onClick={e => e.stopPropagation()} onKeyDown={handleKeyDown}>
        <input
          ref={inputRef}
          className="palette-input"
          type="text"
          placeholder="Type a command…"
          value={query}
          onChange={e => setQuery(e.target.value)}
        />
        {!connected && (
          <div className="palette-warning">Not connected — commands will not be sent</div>
        )}
        <div className="palette-list" role="listbox">
          {!query && recentItems.length > 0 && (
            <>
              <div className="palette-category">Recent</div>
              {recentItems.map(item => (
                <button
                  key={`recent-${item.id}`}
                  className="palette-item"
                  onClick={() => executeItem(item)}
                >
                  <span className="palette-item-label">{item.label}</span>
                  <span className="palette-item-cat">{item.category}</span>
                </button>
              ))}
              <div className="palette-divider" />
            </>
          )}
          {filtered.length === 0 ? (
            <div className="palette-empty">No commands match "{query}"</div>
          ) : (
            filtered.map((item, i) => (
              <button
                key={item.id}
                className={`palette-item ${i === selectedIndex ? 'palette-item--selected' : ''}`}
                role="option"
                aria-selected={i === selectedIndex}
                onClick={() => executeItem(item)}
                onMouseEnter={() => setSelectedIndex(i)}
              >
                <span className="palette-item-label">{item.label}</span>
                <span className="palette-item-cat">{item.category}</span>
              </button>
            ))
          )}
        </div>
        <div className="palette-footer">
          <span><kbd>↑↓</kbd> navigate</span>
          <span><kbd>Enter</kbd> execute</span>
          <span><kbd>Esc</kbd> close</span>
        </div>
      </div>
    </div>
  );
}
