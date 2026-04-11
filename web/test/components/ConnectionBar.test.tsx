import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import ConnectionBar from '../../src/components/ConnectionBar';

const baseProps = {
  connected: false,
  transport: null,
  variant: 'hw4',
  rate: 0,
  streaming: false,
  canOnline: false,
  standby: false,
  bus1: true, bus2: false, bus3: false,
  busFsd: true, busVehicle: false, busBody: false,
  onConnect: vi.fn(),
  onDisconnect: vi.fn(),
  onCommand: vi.fn(),
  canUseSerial: true,
};

describe('ConnectionBar', () => {
  it('renders disconnected state with connect buttons', () => {
    render(<ConnectionBar {...baseProps} />);
    expect(screen.getByText('Not Connected')).toBeInTheDocument();
    expect(screen.getByText('Connect USB')).toBeInTheDocument();
  });

  it('renders connected state with disconnect button', () => {
    render(<ConnectionBar {...baseProps} connected transport="usb" canOnline />);
    expect(screen.getByText('Connected')).toBeInTheDocument();
    expect(screen.getByText('Disconnect')).toBeInTheDocument();
  });

  it('calls onConnect when Connect USB is clicked', async () => {
    const onConnect = vi.fn();
    render(<ConnectionBar {...baseProps} onConnect={onConnect} />);
    await userEvent.click(screen.getByText('Connect USB'));
    expect(onConnect).toHaveBeenCalledWith('usb');
  });

  it('calls onDisconnect when Disconnect is clicked', async () => {
    const onDisconnect = vi.fn();
    render(<ConnectionBar {...baseProps} connected transport="usb" onDisconnect={onDisconnect} />);
    await userEvent.click(screen.getByText('Disconnect'));
    expect(onDisconnect).toHaveBeenCalled();
  });

  it('shows CAN bus health indicators when connected', () => {
    render(<ConnectionBar {...baseProps} connected transport="usb" canOnline busFsd busVehicle={false} />);
    expect(screen.getByText(/FSD/)).toBeInTheDocument();
  });

  it('shows rate when connected and streaming', () => {
    render(<ConnectionBar {...baseProps} connected transport="usb" rate={42} streaming />);
    expect(screen.getByText(/42 msg\/s/)).toBeInTheDocument();
  });

  it('shows fallback when Web Serial is not supported', () => {
    render(<ConnectionBar {...baseProps} canUseSerial={false} />);
    expect(screen.getByText(/Web Serial not supported/)).toBeInTheDocument();
  });
});
