import { commands } from '@teslacanmodder/protocol';
import type { PendingCommand } from '../hooks/useSerial';
import './ConnectionBar.css';

interface ConnectionBarProps {
  connected: boolean;
  transport: string | null;
  variant: string;
  rate: number;
  streaming: boolean;
  canOnline: boolean;
  standby: boolean;
  bus1: boolean;
  bus2: boolean;
  bus3: boolean;
  busFsd: boolean;
  busVehicle: boolean;
  busBody: boolean;
  onConnect: (type: string) => void;
  onDisconnect: () => void;
  onCommand: (cmd: string) => void;
  canUseSerial: boolean;
  lastError?: string | null;
  pendingCommand?: PendingCommand | null;
  onClearError?: () => void;
}

export default function ConnectionBar({ connected, transport, variant, rate, streaming, canOnline, standby, bus1, bus2, bus3, busFsd, busVehicle, busBody, onConnect, onDisconnect, onCommand, canUseSerial, lastError, pendingCommand, onClearError }: ConnectionBarProps) {
  const busStatus = standby ? 'standby' : canOnline ? 'online' : 'waiting';
  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Waiting';

  const activeBuses = [];
  if (busFsd) activeBuses.push(bus1 ? 'FSD\u2713' : 'FSD\u2717');
  if (busVehicle) activeBuses.push(bus2 ? 'Veh\u2713' : 'Veh\u2717');
  if (busBody) activeBuses.push(bus3 ? 'Body\u2713' : 'Body\u2717');

  return (
    <div className={`connection-bar ${connected ? 'connected' : ''}`}>
      <div className="connection-status">
        <div className={`status-dot ${connected ? 'online' : 'offline'}`}></div>
        <div className="status-text">
          <strong>{connected ? 'Connected' : 'Not Connected'}</strong>
          {connected && (
            <span>
              {transport === 'usb' ? 'USB' : 'Bluetooth'} · {variant.toUpperCase()}
              {activeBuses.length > 0 && ` · ${activeBuses.join(' ')}`}
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

      {pendingCommand && (
        <div className="connection-pending">
          Waiting for ack: <strong>{pendingCommand.command}</strong>…
        </div>
      )}

      {lastError && (
        <div className="connection-error">
          <span>{lastError}</span>
          {onClearError && <button className="btn btn-sm" onClick={onClearError}>Dismiss</button>}
        </div>
      )}
    </div>
  );
}
