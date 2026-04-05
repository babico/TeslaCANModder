import { useEffect, useState } from 'react';
import { useSerial } from './hooks/useSerial';
import { useBoardState } from './hooks/useBoardState';
import DashboardPage from './pages/DashboardPage';
import FlasherPage from './pages/FlasherPage';
import SetupGuidePage from './pages/SetupGuidePage';
import './styles/reset.css';
import './styles/variables.css';
import './styles/layout.css';
import './styles/components.css';

export default function App() {
  const [page, setPage] = useState('dashboard');
  const serial = useSerial();
  const board = useBoardState();

  useEffect(() => {
    serial.setOnMessage(board.handleMessage);
  }, [serial.setOnMessage, board.handleMessage]);

  useEffect(() => {
    if (!serial.connected) {
      board.reset();
    }
  }, [serial.connected, board.reset]);

  return (
    <div className="app">
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
        </div>
      </nav>

      <div className="app-content">
        {page === 'dashboard' && <DashboardPage serial={serial} board={board} />}
        {page === 'flasher' && <FlasherPage />}
        {page === 'setup' && <SetupGuidePage />}
      </div>
    </div>
  );
}
