import { FSD_COMMANDS, FSD_PROFILE_OPTIONS } from './package';

export default function FsdPackagePanel({ packageState, board }) {
  const { isConnected, sendCommand } = board;

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
            Package commands: <code>fsd:on</code>, <code>fsd:off</code>, <code>profile:&lt;0-4&gt;</code>, <code>status</code>
          </p>
          <p>
            These overrides are session-scoped runtime values. Rebooting the board resets them back to firmware defaults.
          </p>
        </div>
      </div>
    </section>
  );
}
