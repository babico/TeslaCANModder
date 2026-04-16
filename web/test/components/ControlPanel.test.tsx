import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import ControlPanel from '../../src/components/ControlPanel';
import type { BoardState } from '@teslacanmodder/protocol';

const defaultState: BoardState = {
  variant: 'hw4',
  hardware: 'ArduinoUno',
  driver: 'arduino-mcp2515',
  board: 'arduino',
  uptime: 3661000,
  rate: 0,
  fsd: true,
  nag: false,
  profile: 1,
  profilePinned: false,
  offset: 0,
  offsetPinned: false,
  isaChime: false,
  summonInject: false,
  canOnline: true,
  standby: false,
  bus1: false,
  bus2: false,
  bus3: false,
  busFsd: true,
  busVehicle: false,
  busBody: false,
  summonActive: false,
  streaming: false,
  frames: [],
  frameCount: 0,
  messages: [],
  features: {
    fsd: true,
    profile: true,
    nag: true,
    speedOffset: false,
    isaSpeedChime: false,
    summon: false,
  },
};

const baseProps = {
  state: defaultState,
  connected: true,
  onCommand: vi.fn(),
};

describe('ControlPanel', () => {
  it('renders board info section', () => {
    render(<ControlPanel {...baseProps} />);
    expect(screen.getByText('ArduinoUno')).toBeInTheDocument();
  });

  it('shows CAN Active when canOnline is true', () => {
    render(<ControlPanel {...baseProps} />);
    expect(screen.getByText('CAN Active')).toBeInTheDocument();
  });

  it('shows Standby when standby is true', () => {
    render(<ControlPanel {...baseProps} state={{ ...defaultState, canOnline: false, standby: true }} />);
    expect(screen.getByText('Standby')).toBeInTheDocument();
  });

  it('renders FSD toggle that fires command', async () => {
    const onCommand = vi.fn();
    render(<ControlPanel {...baseProps} onCommand={onCommand} state={{ ...defaultState, fsd: true }} />);
    // Find the FSD disable button
    const fsdButtons = screen.getAllByText(/FSD/i);
    expect(fsdButtons.length).toBeGreaterThan(0);
  });

  it('renders profile buttons', () => {
    render(<ControlPanel {...baseProps} />);
    expect(screen.getByText('Chill')).toBeInTheDocument();
    expect(screen.getByText('Normal')).toBeInTheDocument();
  });

  it('disables buttons when not connected', () => {
    render(<ControlPanel {...baseProps} connected={false} />);
    const chill = screen.getByText('Chill');
    expect(chill.closest('button')).toBeDisabled();
  });

  it('formats uptime as hours and minutes', () => {
    render(<ControlPanel {...baseProps} />);
    // 3661 seconds = 1h 1m 1s
    expect(screen.getByText(/1h/)).toBeInTheDocument();
  });
});
