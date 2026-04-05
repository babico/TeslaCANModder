import { commands } from '../utils/commands';

export default function ConnectionBar({ connected, transport, variant, rate, streaming, canOnline, standby, bus2, onConnect, onDisconnect, onCommand, canUseSerial }) {
  const busStatus = standby ? 'standby' : canOnline ? 'online' : 'waiting';
  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Waiting';

  return (
    <div className={`connection-bar ${connected ? 'connected' : ''}`}>
      <div className="connection-status">
        <div className={`status-dot ${connected ? 'online' : 'offline'}`}></div>
        <div className="status-text">
          <strong>{connected ? 'Connected' : 'Not Connected'}</strong>
          {connected && (
            <span>
              {transport === 'usb' ? 'USB' : 'Bluetooth'} · {variant.toUpperCase()}
              {bus2 ? ' · Dual CAN' : ''}
              {' · '}{rate} msg/s
              {' · '}<span className={`can-status can-status-${busStatus}`}>{busLabel}</span>
            </span>
          )}
        </div>
      </div>

      <div className="connection-actions">
        {canUseSerial ? (
          connected ? (
            <>
              {['hw4', 'hw3', 'legacy'].map(v => (
                <button key={v} className={`btn btn-sm ${variant === v ? 'btn-primary' : ''}`} onClick={() => onCommand(commands.variant(v))}>
                  {v === 'legacy' ? 'Legacy' : v.toUpperCase()}
                </button>
              ))}
              <button className={`btn btn-sm ${streaming ? 'btn-primary' : ''}`} onClick={() => onCommand(commands.stream(!streaming))}>
                {streaming ? 'Stop Stream' : 'Start Stream'}
              </button>
              <button className="btn btn-danger" onClick={onDisconnect}>Disconnect</button>
            </>
          ) : (
            <>
              <button className="btn btn-primary" onClick={() => onConnect('usb')}>Connect USB</button>
              <button className="btn btn-ghost" onClick={() => onConnect('bluetooth')}>Connect HC-05</button>
            </>
          )
        ) : (
          <span className="text-muted">Web Serial not supported</span>
        )}
      </div>
    </div>
  );
}
