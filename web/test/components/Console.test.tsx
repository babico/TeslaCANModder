import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import Console from '../../src/components/Console';
import type { ConsoleMessage } from '@teslacanmodder/protocol';

const baseProps = {
  messages: [] as ConsoleMessage[],
  connected: false,
  onCommand: vi.fn(),
  onClear: vi.fn(),
};

describe('Console', () => {
  it('renders empty state when no messages', () => {
    render(<Console {...baseProps} />);
    expect(screen.getByText(/Board messages will appear here/)).toBeInTheDocument();
  });

  it('renders messages when provided', () => {
    const messages: ConsoleMessage[] = [
      { id: 1, ts: '12:00:00', type: 'info', text: 'Board connected: hw4' },
      { id: 2, ts: '12:00:01', type: 'error', text: 'Timeout' },
    ];
    render(<Console {...baseProps} messages={messages} />);
    expect(screen.getByText('Board connected: hw4')).toBeInTheDocument();
    expect(screen.getByText('Timeout')).toBeInTheDocument();
  });

  it('disables input and send when not connected', () => {
    render(<Console {...baseProps} connected={false} />);
    expect(screen.getByPlaceholderText('Type command...')).toBeDisabled();
    expect(screen.getByText('Send')).toBeDisabled();
  });

  it('enables input when connected', () => {
    render(<Console {...baseProps} connected />);
    expect(screen.getByPlaceholderText('Type command...')).not.toBeDisabled();
  });

  it('sends command on button click and clears input', async () => {
    const onCommand = vi.fn();
    render(<Console {...baseProps} connected onCommand={onCommand} />);
    const input = screen.getByPlaceholderText('Type command...');
    await userEvent.type(input, 'ping');
    await userEvent.click(screen.getByText('Send'));
    expect(onCommand).toHaveBeenCalledWith('ping');
    expect(input).toHaveValue('');
  });

  it('calls onClear when Clear is clicked', async () => {
    const onClear = vi.fn();
    render(<Console {...baseProps} onClear={onClear} />);
    await userEvent.click(screen.getByText('Clear'));
    expect(onClear).toHaveBeenCalled();
  });
});
