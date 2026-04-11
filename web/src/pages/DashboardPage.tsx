import { useState } from 'react';
import ConnectionBar from '../components/ConnectionBar';
import './DashboardPage.css';
import FrameTable from '../components/FrameTable';
import ControlPanel from '../components/ControlPanel';
import Console from '../components/Console';
import { commands } from '@teslacanmodder/protocol';
import type { BoardState } from '@teslacanmodder/protocol';
import type { UseSerialReturn } from '../hooks/useSerial';
import type { UseBoardStateReturn } from '../hooks/useBoardState';

const TABS = [
  { id: 'controls', label: 'Controls' },
  { id: 'vehicle', label: 'Vehicle' },
  { id: 'monitor', label: 'Monitor' },
];

interface DashboardPageProps {
  serial: UseSerialReturn;
  board: UseBoardStateReturn;
}

export default function DashboardPage({ serial, board }: DashboardPageProps) {
  const [activeTab, setActiveTab] = useState('controls');
  const { connected, transport, connect, disconnect, send, canUseSerial } = serial;
  const { state, clearFrames, clearMessages } = board;

  return (
    <div className="dashboard">
      <ConnectionBar
        connected={connected}
        transport={transport}
        variant={state.variant}
        rate={state.rate}
        streaming={state.streaming}
        canOnline={state.canOnline}
        standby={state.standby}
        bus1={state.bus1}
        bus2={state.bus2}
        bus3={state.bus3}
        busFsd={state.busFsd}
        busVehicle={state.busVehicle}
        busBody={state.busBody}
        onConnect={connect}
        onDisconnect={disconnect}
        onCommand={send}
        canUseSerial={canUseSerial}
      />

      <div className="dashboard-tabs">
        {TABS.map(tab => (
          <button
            key={tab.id}
            className={`dashboard-tab ${activeTab === tab.id ? 'active' : ''}`}
            onClick={() => setActiveTab(tab.id)}
          >
            {tab.label}
          </button>
        ))}
      </div>

      <div className="dashboard-body">
        {activeTab === 'controls' && (
          <div className="dashboard-controls">
            <ControlPanel
              state={state}
              connected={connected}
              onCommand={send}
            />
          </div>
        )}

        {activeTab === 'vehicle' && (
          <div className="dashboard-vehicle">
            <VehiclePanel
              state={state}
              connected={connected}
              onCommand={send}
            />
          </div>
        )}

        {activeTab === 'monitor' && (
          <div className="dashboard-monitor">
            <div className="dashboard-monitor-frames">
              <FrameTable
                frames={state.frames}
                frameCount={state.frameCount}
                onClear={clearFrames}
              />
            </div>
            <div className="dashboard-monitor-console">
              <Console
                messages={state.messages}
                connected={connected}
                onCommand={send}
                onClear={clearMessages}
              />
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

// Vehicle controls separated into its own panel

interface VehiclePanelProps {
  state: BoardState;
  connected: boolean;
  onCommand: (cmd: string) => void;
}

function VehiclePanel({ state, connected, onCommand }: VehiclePanelProps) {
  const { variant, busVehicle, busBody } = state;
  const isLegacy = variant === 'legacy';

  if (isLegacy) {
    return (
      <div className="panel">
        <div className="panel-header">
          <h3>Vehicle Controls</h3>
          <span className="badge-warning">Limited on Legacy variant</span>
        </div>
        <div className="panel-body">
          <div className="info-banner">Vehicle control commands are not available on Legacy variant. Switch to HW3 or HW4 for full control.</div>
        </div>
      </div>
    );
  }

  return (
    <div className="vehicle-grid">
      {!busVehicle && !busBody && (
        <div className="panel">
          <div className="panel-body">
            <div className="info-banner">No vehicle or body bus active. Enable Vehicle or Body bus in the Flasher to use these controls.</div>
          </div>
        </div>
      )}

      {/* Mirror — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Mirrors</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.mirrorFold())}>Fold</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.mirrorUnfold())}>Unfold</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.mirrorHeat())}>Heat</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.mirrorAutofold())}>Auto-fold</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.mirrorDip())}>Dip on Reverse</button>
          </div>
        </div>
      </div>
      )}

      {/* Lock — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Locks & Horn</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lock())}>Lock</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.unlock())}>Unlock</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lockChild())}>Child Lock</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.horn())}>Horn</button>
          </div>
        </div>
      </div>
      )}

      {/* Trunk/Frunk — Vehicle + Body bus */}
      {(busVehicle || busBody) && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Trunk & Frunk</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.frunkOpen())}>Frunk Open</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.frunkClose())}>Frunk Close</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.trunkOpen())}>Trunk Open</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.trunkClose())}>Trunk Close</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.glovebox())}>Glovebox</button>
          </div>
        </div>
      </div>
      )}

      {/* Lights — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Lights</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightFogFront())}>Front Fog</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightFogRear())}>Rear Fog</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightHighbeamAuto())}>Auto Highbeam</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightAmbient())}>Ambient</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightHome())}>Home Lighting</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightDomeOff())}>Dome Off</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightDomeOn())}>Dome On</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.lightDomeAuto())}>Dome Auto</button>
          </div>
        </div>
      </div>
      )}

      {/* Wiper — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Wipers</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.wiperOff())}>Off</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.wiper1())}>Speed 1</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.wiper2())}>Speed 2</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.wiper3())}>Speed 3</button>
          </div>
        </div>
      </div>
      )}

      {/* Seat Heating — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Seat Heating</h3></div>
        <div className="panel-body">
          <div className="seat-grid">
            {[
              { label: 'FL', fn: commands.seatFL },
              { label: 'FR', fn: commands.seatFR },
              { label: 'RL', fn: commands.seatRL },
              { label: 'RR', fn: commands.seatRR },
              { label: 'RC', fn: commands.seatRC },
            ].map(seat => (
              <div key={seat.label} className="seat-row">
                <span className="seat-label">{seat.label}</span>
                {[0, 1, 2, 3].map(l => (
                  <button key={l} className="btn btn-sm" disabled={!connected} onClick={() => onCommand(seat.fn(l))}>
                    {l === 0 ? 'Off' : l}
                  </button>
                ))}
              </div>
            ))}
          </div>
        </div>
      </div>
      )}

      {/* Window & Sentry — Body bus — Body bus */}
      {busBody && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Window & Sentry</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.ventOpen())}>Vent Open</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.ventClose())}>Vent Close</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.sentryOn())}>Sentry On</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.sentryOff())}>Sentry Off</button>
          </div>
        </div>
      </div>
      )}

      {/* Climate — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Climate</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.climateKeep())}>Keep On</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.climateOff())}>Off</button>
          </div>
        </div>
      </div>
      )}

      {/* Charge — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Charging</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.chargeStart())}>Start</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.chargeStop())}>Stop</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.chargePort())}>Open Port</button>
          </div>
        </div>
      </div>
      )}

      {/* Drive Config — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Drive Configuration</h3></div>
        <div className="panel-body">
          <div className="drive-section">
            <span className="drive-label">Pedal</span>
            <div className="feature-grid">
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.pedalStandard())}>Standard</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.pedalChill())}>Chill</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.pedalSport())}>Sport</button>
            </div>
          </div>
          <div className="drive-section">
            <span className="drive-label">Regen</span>
            <div className="feature-grid">
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.regenOff())}>Off</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.regenLow())}>Low</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.regenStd())}>Standard</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.regenMax())}>Max</button>
            </div>
          </div>
          <div className="drive-section">
            <span className="drive-label">Stop Mode</span>
            <div className="feature-grid">
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.stopCreep())}>Creep</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.stopRoll())}>Roll</button>
              <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.stopHold())}>Hold</button>
            </div>
          </div>
        </div>
      </div>
      )}

      {/* Display Brightness — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Display</h3></div>
        <div className="panel-body">
          <div className="brightness-control">
            <span className="drive-label">Brightness</span>
            <input type="range" min="0" max="127" defaultValue="64" disabled={!connected} onChange={(e) => onCommand(commands.mainDisplay(parseInt(e.target.value)))} />
          </div>
        </div>
      </div>
      )}

      {/* Power — Vehicle bus — Vehicle bus */}
      {busVehicle && (
      <div className="panel vehicle-card">
        <div className="panel-header"><h3>Power</h3></div>
        <div className="panel-body">
          <div className="feature-grid">
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.powerAccOn())}>Accessory On</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.powerAccOff())}>Accessory Off</button>
            <button className="btn btn-sm" disabled={!connected} onClick={() => onCommand(commands.powerReady())}>Drive Ready</button>
            <button className="btn btn-sm btn-danger" disabled={!connected} onClick={() => onCommand(commands.powerOff())}>Power Off</button>
          </div>
        </div>
      </div>
      )}
    </div>
  );
}
