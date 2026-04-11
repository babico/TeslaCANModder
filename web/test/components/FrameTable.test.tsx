import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import FrameTable from '../../src/components/FrameTable';
import type { CanFrame } from '@teslacanmodder/protocol';

// Mock fetch for CAN DB
globalThis.fetch = vi.fn().mockResolvedValue({
  ok: true,
  json: () => Promise.resolve({ frames: [] }),
});

const baseProps = {
  frames: [] as CanFrame[],
  frameCount: 0,
  onClear: vi.fn(),
};

describe('FrameTable', () => {
  it('renders empty state when no frames', () => {
    render(<FrameTable {...baseProps} />);
    expect(screen.getByText(/Connect and start streaming/)).toBeInTheDocument();
  });

  it('renders frame count badge', () => {
    render(<FrameTable {...baseProps} frameCount={42} />);
    expect(screen.getByText('42 frames')).toBeInTheDocument();
  });

  it('renders frame rows when frames provided', () => {
    const frames: CanFrame[] = [
      { key: 'f1', ts: '12:00:00', bus: 0, busName: 'FSD', dir: 'rx', id: 1021, dlc: 3, data: 'AABBCC' },
    ];
    render(<FrameTable {...baseProps} frames={frames} frameCount={1} />);
    expect(screen.getByText('0x3FD')).toBeInTheDocument();
    expect(screen.getByText('FSD')).toBeInTheDocument();
    expect(screen.getByText('RX')).toBeInTheDocument();
    expect(screen.getByText('AABBCC')).toBeInTheDocument();
  });

  it('calls onClear when Clear is clicked', async () => {
    const onClear = vi.fn();
    render(<FrameTable {...baseProps} onClear={onClear} />);
    await userEvent.click(screen.getByText('Clear'));
    expect(onClear).toHaveBeenCalled();
  });

  it('renders column headers when frames exist', () => {
    const frames: CanFrame[] = [
      { key: 'f1', ts: '12:00:00', bus: 0, busName: 'FSD', dir: 'rx', id: 1021, dlc: 3, data: 'AABBCC' },
    ];
    render(<FrameTable {...baseProps} frames={frames} frameCount={1} />);
    expect(screen.getByText('Time')).toBeInTheDocument();
    expect(screen.getByText('Bus')).toBeInTheDocument();
    expect(screen.getByText('Dir')).toBeInTheDocument();
    expect(screen.getByText('ID')).toBeInTheDocument();
    expect(screen.getByText('DLC')).toBeInTheDocument();
    expect(screen.getByText('Data')).toBeInTheDocument();
  });
});
