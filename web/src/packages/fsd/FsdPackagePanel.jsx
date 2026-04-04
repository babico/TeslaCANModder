import { useState } from 'react';
import { FSD_COMMANDS, FSD_PROFILE_OPTIONS } from './package';

export default function FsdPackagePanel({ packageState, board }) {
  const { isConnected, sendCommand } = board;
  const [expertOpen, setExpertOpen] = useState(false);

  const handleCommand = (command) => {
    if (!isConnected) {
      return;
    }

    void sendCommand(command);
  };

  return (
    <section className="panel package-panel">
      <div className="panel-head">
        <div>
          <h2>FSD Package</h2>
          <p>Feature-specific overrides live here instead of the core CAN viewer.</p>
        </div>
        <span className="badge mono">Runtime Override Pack</span>
      </div>

      <div className="package-body">
        <div className="package-status-grid">
          <div className="package-stat-card">
            <span className="package-stat-label">FSD State</span>
            <strong className={`package-stat-value ${packageState.enabled ? 'val-on' : 'val-off'}`}>
              {packageState.enabledLabel}
            </strong>
          </div>
          <div className="package-stat-card">
            <span className="package-stat-label">Speed Profile</span>
            <strong className="package-stat-value">{packageState.speedProfileLabel}</strong>
          </div>
          <div className="package-stat-card">
            <span className="package-stat-label">HW3 Speed Offset</span>
            <strong className="package-stat-value">
              {packageState.features.speedOffset ? `${packageState.speedOffset}` : 'N/A'}
            </strong>
          </div>
          <div className="package-stat-card">
            <span className="package-stat-label">HW4 ISA Chime</span>
            <strong className={`package-stat-value ${packageState.isaChimeEnabled ? 'val-on' : 'val-off'}`}>
              {packageState.features.isaSpeedChime ? (packageState.isaChimeEnabled ? 'SUPPRESSED' : 'ORIGINAL') : 'N/A'}
            </strong>
          </div>
        </div>

        <div className="page-section">
          <div className="section-header">
            <span className="step-number">1</span>
            <div>
              <h3>FSD Enable Override</h3>
              <p>These commands update the firmware runtime state through the dedicated FSD package API.</p>
            </div>
          </div>

          <div className="package-action-row">
            <button
              className={`preset-button ${packageState.enabled ? 'active' : ''}`}
              disabled={!isConnected}
              onClick={() => handleCommand(FSD_COMMANDS.enable)}
            >
              Enable FSD
            </button>
            <button
              className={`preset-button ${!packageState.enabled ? 'active' : ''}`}
              disabled={!isConnected}
              onClick={() => handleCommand(FSD_COMMANDS.disable)}
            >
              Disable FSD
            </button>
            <button
              className="btn-sm btn-ghost"
              disabled={!isConnected}
              onClick={() => handleCommand(FSD_COMMANDS.refresh)}
            >
              Refresh Status
            </button>
          </div>
        </div>

        <div className="page-section">
          <div className="section-header">
            <span className="step-number">2</span>
            <div>
              <h3>Profile Presets</h3>
              <p>The base transport stays generic. The FSD package owns the profile preset commands.</p>
            </div>
          </div>

          <div className="preset-grid">
            {FSD_PROFILE_OPTIONS.map((profile) => (
              <button
                key={profile.value}
                className={`preset-button ${packageState.speedProfile === profile.value ? 'active' : ''}`}
                disabled={!isConnected}
                onClick={() => handleCommand(FSD_COMMANDS.setSpeedProfile(profile.value))}
              >
                {profile.label}
              </button>
            ))}
          </div>
        </div>

        <div className="panel package-note">
          <p>
            Package commands: <code>fsd:on</code>, <code>fsd:off</code>, <code>profile:&lt;0-4&gt;</code>, <code>offset:&lt;0-100&gt;</code>, <code>isa-chime:on|off</code>, <code>status</code>
          </p>
          <p>
            These overrides are session-scoped runtime values. Rebooting the board resets them back to firmware defaults.
          </p>
        </div>

        {(packageState.features.speedOffset || packageState.features.isaSpeedChime) ? (
          <details className="dashboard-collapsible" open={expertOpen} onToggle={(event) => setExpertOpen(event.currentTarget.open)}>
            <summary>Expert Controls</summary>
            <div className="package-body" style={{ paddingTop: 0 }}>
              {packageState.features.speedOffset ? (
                <div className="page-section">
                  <div className="section-header">
                    <span className="step-number">3</span>
                    <div>
                      <h3>HW3 Speed Offset</h3>
                      <p>Use this only on the HW3 runtime variant. The offset is injected on the mux 2 autopilot frame.</p>
                    </div>
                  </div>
                  <div className="preset-grid">
                    {[0, 10, 20, 30, 40, 50].map((offset) => (
                      <button
                        key={offset}
                        className={`preset-button ${packageState.speedOffset === offset ? 'active' : ''}`}
                        disabled={!isConnected}
                        onClick={() => handleCommand(FSD_COMMANDS.setOffset(offset))}
                      >
                        {offset}
                      </button>
                    ))}
                  </div>
                </div>
              ) : null}

              {packageState.features.isaSpeedChime ? (
                <div className="page-section">
                  <div className="section-header">
                    <span className="step-number">{packageState.features.speedOffset ? '4' : '3'}</span>
                    <div>
                      <h3>HW4 ISA Speed-Chime</h3>
                      <p>HW4 can suppress the speed warning chime on the dedicated DAS status frame.</p>
                    </div>
                  </div>
                  <div className="package-action-row">
                    <button
                      className={`preset-button ${packageState.isaChimeEnabled ? 'active' : ''}`}
                      disabled={!isConnected}
                      onClick={() => handleCommand(FSD_COMMANDS.setIsaChime(true))}
                    >
                      Suppress Chime
                    </button>
                    <button
                      className={`preset-button ${!packageState.isaChimeEnabled ? 'active' : ''}`}
                      disabled={!isConnected}
                      onClick={() => handleCommand(FSD_COMMANDS.setIsaChime(false))}
                    >
                      Keep Original
                    </button>
                  </div>
                </div>
              ) : null}
            </div>
          </details>
        ) : null}
      </div>
    </section>
  );
}
