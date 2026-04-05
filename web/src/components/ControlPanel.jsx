import { commands } from '../utils/commands';
import { useState } from 'react';

export default function ControlPanel({ state, connected, onCommand }) {
  const { hardware, driver, uptime, variant, fsd, nag, profile, profilePinned, offset, offsetPinned, isaChime, features, canOnline, standby, bus2 } = state;
  const [customOffset, setCustomOffset] = useState('');

  const formatUptime = (ms) => {
    if (!ms) return '\u2014';
    const s = Math.floor(ms / 1000);
    const m = Math.floor(s / 60);
    const h = Math.floor(m / 60);
    if (h > 0) return `${h}h ${m % 60}m`;
    if (m > 0) return `${m}m ${s % 60}s`;
    return `${s}s`;
  };

  const handleCustomOffset = () => {
    const val = parseInt(customOffset);
    if (!isNaN(val) && val >= 0 && val <= 100) {
      onCommand(commands.offset(val));
      setCustomOffset('');
    }
  };

  const busStatus = standby ? 'standby' : canOnline ? 'online' : 'offline';
  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Offline';

  return (
    <div className="control-panel">
      {/* Top row: Board + EEPROM side by side */}
      <div className="ctrl-top-row">
        <div className="panel ctrl-panel-half">
          <div className="panel-header">
            <h3>Board</h3>
            <span className="badge">{variant.toUpperCase()}</span>
          </div>
          <div className="panel-body">
            <div className={`health-banner health-${busStatus}`}>
              <div className={`health-dot health-dot-${busStatus}`}></div>
              <div className="health-info">
                <strong>{busLabel}</strong>
                {standby && <span>Auto-recover on wake</span>}
              </div>
            </div>
            <div className="stat-grid">
              <div className="stat"><span className="stat-label">Board</span><strong className="stat-value">{hardware}</strong></div>
              <div className="stat"><span className="stat-label">Driver</span><strong className="stat-value">{driver}</strong></div>
              <div className="stat"><span className="stat-label">Uptime</span><strong className="stat-value">{formatUptime(uptime)}</strong></div>
              <div className="stat"><span className="stat-label">CAN</span><strong className="stat-value">{bus2 ? 'Dual' : 'Single'}</strong></div>
            </div>
          </div>
        </div>

        <div className="panel ctrl-panel-half">
          <div className="panel-header">
            <h3>EEPROM</h3>
            <span className="badge-muted">Saved</span>
          </div>
          <div className="panel-body">
            <div className="settings-grid">
              <div><span className="text-muted">Variant:</span> <strong>{variant.toUpperCase()}</strong></div>
              <div><span className="text-muted">FSD:</span> <strong className={fsd ? 'text-success' : 'text-muted'}>{fsd ? 'ON' : 'OFF'}</strong></div>
              <div><span className="text-muted">Nag:</span> <strong className={nag ? 'text-success' : 'text-muted'}>{nag ? 'ON' : 'OFF'}</strong></div>
              <div><span className="text-muted">Profile:</span> <strong>{profile} {profilePinned ? '(pinned)' : '(auto)'}</strong></div>
              {features.speedOffset && (
                <div><span className="text-muted">Offset:</span> <strong>{offset}% {offsetPinned ? '(pinned)' : '(auto)'}</strong></div>
              )}
              {features.isaSpeedChime && (
                <div><span className="text-muted">ISA:</span> <strong className={isaChime ? 'text-success' : 'text-muted'}>{isaChime ? 'SUP' : 'ORI'}</strong></div>
              )}
            </div>
          </div>
        </div>
      </div>

      {/* Feature cards in a compact 2-col grid */}
      <div className="ctrl-features-grid">
        <div className="ctrl-feature-card">
          <div className="ctrl-feature-top">
            <span className="ctrl-feature-name">FSD</span>
            <span className={`feature-status ${fsd ? 'status-on' : 'status-off'}`}>{fsd ? 'ON' : 'OFF'}</span>
          </div>
          <div className="ctrl-feature-btns">
            <button className={`btn btn-sm ${fsd ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.fsd(true))}>Enable</button>
            <button className={`btn btn-sm ${!fsd ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.fsd(false))}>Disable</button>
          </div>
        </div>

        <div className="ctrl-feature-card">
          <div className="ctrl-feature-top">
            <span className="ctrl-feature-name">Nag Suppress</span>
            <span className={`feature-status ${nag ? 'status-on' : 'status-off'}`}>{nag ? 'ON' : 'OFF'}</span>
          </div>
          <div className="ctrl-feature-btns">
            <button className={`btn btn-sm ${nag ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.nag(true))}>Enable</button>
            <button className={`btn btn-sm ${!nag ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.nag(false))}>Disable</button>
          </div>
        </div>

        {features.isaSpeedChime && (
          <div className="ctrl-feature-card">
            <div className="ctrl-feature-top">
              <span className="ctrl-feature-name">ISA Chime</span>
              <span className={`feature-status ${isaChime ? 'status-on' : 'status-off'}`}>{isaChime ? 'SUP' : 'ORI'}</span>
            </div>
            <div className="ctrl-feature-btns">
              <button className={`btn btn-sm ${isaChime ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.isaChime(true))}>Suppress</button>
              <button className={`btn btn-sm ${!isaChime ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.isaChime(false))}>Original</button>
            </div>
          </div>
        )}

        {features.summon && (
          <div className="ctrl-feature-card">
            <div className="ctrl-feature-top">
              <span className="ctrl-feature-name">Summon</span>
            </div>
            <div className="ctrl-feature-btns">
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.summonForward())}>Fwd</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.summonReverse())}>Rev</button>
              <button className="btn btn-sm btn-danger" disabled={!connected} onClick={() => onCommand(commands.summonStop())}>Stop</button>
            </div>
          </div>
        )}
      </div>

      {/* Speed Profile */}
      <div className="panel">
        <div className="panel-header">
          <h3>Speed Profile</h3>
          <span className="feature-status">{profile} {profilePinned ? '(PINNED)' : '(AUTO)'}</span>
        </div>
        <div className="panel-body">
          <div className="feature-grid">
            {[{id:0,name:'Chill'},{id:1,name:'Normal'},{id:2,name:'Hurry'},{id:3,name:'Max'},{id:4,name:'Sloth'}].map((p) => (
              <button key={p.id} className={`btn btn-sm ${profilePinned && profile === p.id ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.profile(p.id))}>
                {p.name}
              </button>
            ))}
            <button className={`btn btn-sm ${!profilePinned ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.profileAuto())}>Auto</button>
          </div>
        </div>
      </div>

      {/* Speed Offset (HW3) */}
      {features.speedOffset && (
        <div className="panel">
          <div className="panel-header">
            <h3>Speed Offset</h3>
            <span className="feature-status">{offset}% {offsetPinned ? '(PINNED)' : '(AUTO)'}</span>
          </div>
          <div className="panel-body">
            <div className="feature-grid">
              {[0, 20, 40, 60, 80, 100].map((o) => (
                <button key={o} className={`btn btn-sm ${offsetPinned && offset === o ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.offset(o))}>
                  {o}%
                </button>
              ))}
              <button className={`btn btn-sm ${!offsetPinned ? 'btn-primary' : ''}`} disabled={!connected} onClick={() => onCommand(commands.offsetAuto())}>Auto</button>
            </div>
            <div className="feature-custom">
              <input type="number" min="0" max="100" placeholder="Custom %" value={customOffset} onChange={(e) => setCustomOffset(e.target.value)} disabled={!connected} />
              <button className="btn btn-sm" disabled={!connected || !customOffset} onClick={handleCustomOffset}>Set</button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
