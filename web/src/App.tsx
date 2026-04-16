import { useEffect, useState } from 'react';
import { useSerial } from './hooks/useSerial';
import { useBoardState } from './hooks/useBoardState';
import DashboardPage from './pages/DashboardPage';
import FlasherPage from './pages/FlasherPage';
import SetupGuidePage from './pages/SetupGuidePage';
import DocsPage from './pages/DocsPage';
import CommandPalette from './components/CommandPalette';
import './styles/reset.css';
import './styles/lib.css';
import './styles/layout.css';

export default function App() {
  const [page, setPage] = useState<'dashboard' | 'flasher' | 'setup' | 'docs'>('dashboard');
  const [paletteOpen, setPaletteOpen] = useState(false);
  const serial = useSerial();
  const board = useBoardState();

  useEffect(() => {
    serial.setOnMessage((msg: Record<string, unknown>) => {
      // Clear pending command on ack
      if (msg.t === 'ack' && typeof msg.cmd === 'string') {
        serial.ackReceived(msg.cmd);
      }
      board.handleMessage(msg);
    });
  }, [serial.setOnMessage, board.handleMessage, serial.ackReceived]);

  useEffect(() => {
    if (!serial.connected) {
      board.reset();
    }
  }, [serial.connected, board.reset]);

  // Ctrl+K / Cmd+K to open command palette
  useEffect(() => {
    const handler = (e: KeyboardEvent) => {
      if ((e.metaKey || e.ctrlKey) && e.key === 'k') {
        e.preventDefault();
        setPaletteOpen(prev => !prev);
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, []);

  return (
    <div className="app">
      <CommandPalette
        open={paletteOpen}
        onClose={() => setPaletteOpen(false)}
        onCommand={serial.send}
        connected={serial.connected}
      />
      <nav className="app-nav">
        <div className="app-logo">TeslaCANModder</div>
        <div className="app-nav-links">
          <button className={page === 'dashboard' ? 'active' : ''} onClick={() => setPage('dashboard')}>
            Dashboard
          </button>
          <button className={page === 'flasher' ? 'active' : ''} onClick={() => setPage('flasher')}>
            Flasher
          </button>
          <button className={page === 'setup' ? 'active' : ''} onClick={() => setPage('setup')}>
            Setup Guide
          </button>
          <button className={page === 'docs' ? 'active' : ''} onClick={() => setPage('docs')}>
            Docs
          </button>
          <button className="btn btn-sm btn-ghost palette-trigger" onClick={() => setPaletteOpen(true)} title="Command Palette (Ctrl+K)">
            ⌘K
          </button>
        </div>
      </nav>

      <div className="app-content">
        {page === 'dashboard' && <DashboardPage serial={serial} board={board} />}
        {page === 'flasher' && <FlasherPage />}
        {page === 'setup' && <SetupGuidePage />}
        {page === 'docs' && <DocsPage />}
      </div>
    </div>
  );
}
