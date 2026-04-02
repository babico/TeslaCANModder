import { useState } from 'react';

import Dashboard from './components/Dashboard';
import CanExplorer from './components/CanExplorer';
import Flasher from './components/Flasher';
import SetupGuide from './components/SetupGuide';
import { useBoardLink } from './hooks/useBoardLink';

function App() {
  const [activeTab, setActiveTab] = useState('setup');
  
  const board = useBoardLink();

  return (
    <div className="app-wrapper">
      <div className="bg-noise"></div>
      <div className="bg-orb bg-orb-a"></div>
      <div className="bg-orb bg-orb-b"></div>
      <div className="bg-orb bg-orb-c"></div>

      <header className="topbar">
        <div className="brand">
          <div className="brand-icon">
            <svg viewBox="0 0 32 32" width="28" height="28" fill="none">
              <path d="M16 4C9.4 4 4 6 4 6s2.6 2 12 2 12-2 12-2-5.4-2-12-2z" fill="#e82127"/>
              <path d="M16 10c-3.2 0-5.5-.3-7-.6V28l7-4 7 4V9.4c-1.5.3-3.8.6-7 .6z" fill="#e82127"/>
            </svg>
          </div>
          <div className="brand-text">
            <span className="brand-kicker">Open Source Toolkit</span>
            <h1>TeslaCANModder</h1>
          </div>
        </div>

        <nav className="tab-bar">
          <button 
            className={`tab ${activeTab === 'setup' ? 'active' : ''}`}
            onClick={() => setActiveTab('setup')}
          >
            Setup Guide
          </button>
          <button 
            className={`tab ${activeTab === 'dashboard' ? 'active' : ''}`}
            onClick={() => setActiveTab('dashboard')}
          >
            Dashboard
          </button>
          <button 
            className={`tab ${activeTab === 'explorer' ? 'active' : ''}`}
            onClick={() => setActiveTab('explorer')}
          >
            CAN Explorer
          </button>
          <button 
            className={`tab ${activeTab === 'flash' ? 'active' : ''}`}
            onClick={() => setActiveTab('flash')}
            style={{color: activeTab === 'flash' ? 'var(--red)' : ''}}
          >
            ⚡ Flash Arduino
          </button>
        </nav>
      </header>
      
      <main className="main-content">
        {activeTab === 'dashboard' && <Dashboard board={board} />}
        {activeTab === 'explorer' && <CanExplorer board={board} />}
        {activeTab === 'flash' && <Flasher board={board} />}
        {activeTab === 'setup' && <SetupGuide board={board} />}
      </main>

      <footer className="site-footer" style={{marginTop: 'auto', padding: '20px', textAlign: 'center', color: 'var(--text-muted)'}}>
        <span>TeslaCANModder Web Client · Modular Viewer + Packages · Not affiliated with Tesla, Inc.</span>
      </footer>
    </div>
  );
}

export default App;
