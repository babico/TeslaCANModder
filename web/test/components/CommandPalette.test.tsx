import { describe, it, expect, vi, beforeEach } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import CommandPalette from '../../src/components/CommandPalette';

const baseProps = {
  open: true,
  onClose: vi.fn(),
  onCommand: vi.fn(),
  connected: true,
};

beforeEach(() => {
  vi.clearAllMocks();
  localStorage.clear();
});

describe('CommandPalette', () => {
  it('renders nothing when closed', () => {
    const { container } = render(<CommandPalette {...baseProps} open={false} />);
    expect(container.innerHTML).toBe('');
  });

  it('renders input and items when open', () => {
    render(<CommandPalette {...baseProps} />);
    expect(screen.getByPlaceholderText('Type a command…')).toBeInTheDocument();
    expect(screen.getByText('FSD Enable')).toBeInTheDocument();
    expect(screen.getByText('Lock')).toBeInTheDocument();
  });

  it('filters items by query', async () => {
    render(<CommandPalette {...baseProps} />);
    await userEvent.type(screen.getByPlaceholderText('Type a command…'), 'fsd');
    expect(screen.getByText('FSD Enable')).toBeInTheDocument();
    expect(screen.getByText('FSD Disable')).toBeInTheDocument();
    expect(screen.queryByText('Lock')).not.toBeInTheDocument();
  });

  it('shows no match message for bad query', async () => {
    render(<CommandPalette {...baseProps} />);
    await userEvent.type(screen.getByPlaceholderText('Type a command…'), 'zzzznotfound');
    expect(screen.getByText(/No commands match/)).toBeInTheDocument();
  });

  it('executes command on item click', async () => {
    const onCommand = vi.fn();
    render(<CommandPalette {...baseProps} onCommand={onCommand} />);
    await userEvent.click(screen.getByText('Ping'));
    expect(onCommand).toHaveBeenCalledWith('ping');
  });

  it('calls onClose on Escape', async () => {
    const onClose = vi.fn();
    render(<CommandPalette {...baseProps} onClose={onClose} />);
    await userEvent.keyboard('{Escape}');
    expect(onClose).toHaveBeenCalled();
  });

  it('calls onClose on overlay click', async () => {
    const onClose = vi.fn();
    const { container } = render(<CommandPalette {...baseProps} onClose={onClose} />);
    const overlay = container.querySelector('.palette-overlay')!;
    await userEvent.click(overlay);
    expect(onClose).toHaveBeenCalled();
  });

  it('shows warning when not connected', () => {
    render(<CommandPalette {...baseProps} connected={false} />);
    expect(screen.getByText(/Not connected/)).toBeInTheDocument();
  });

  it('shows keyboard hints in footer', () => {
    render(<CommandPalette {...baseProps} />);
    expect(screen.getByText('navigate')).toBeInTheDocument();
    expect(screen.getByText('execute')).toBeInTheDocument();
    expect(screen.getByText('close')).toBeInTheDocument();
  });

  it('executes selected item on Enter', async () => {
    const onCommand = vi.fn();
    render(<CommandPalette {...baseProps} onCommand={onCommand} />);
    // First item is Ping by default
    await userEvent.keyboard('{Enter}');
    expect(onCommand).toHaveBeenCalledWith('ping');
  });

  it('navigates with arrow keys', async () => {
    const onCommand = vi.fn();
    render(<CommandPalette {...baseProps} onCommand={onCommand} />);
    await userEvent.keyboard('{ArrowDown}');
    await userEvent.keyboard('{Enter}');
    // Second item is Status
    expect(onCommand).toHaveBeenCalledWith('status');
  });

  it('saves recent commands to localStorage', async () => {
    const onCommand = vi.fn();
    render(<CommandPalette {...baseProps} onCommand={onCommand} />);
    await userEvent.click(screen.getByText('Ping'));
    const stored = JSON.parse(localStorage.getItem('tcm-recent-commands') || '[]');
    expect(stored).toContain('ping');
  });
});
