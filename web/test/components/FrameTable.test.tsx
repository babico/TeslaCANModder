import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import FrameTable from '../../src/components/FrameTable';
import { framesToCsv, framesToJson, BUILT_IN_PRESETS } from '../../src/components/frameUtils';
import type { CanFrame } from '@teslacanmodder/protocol';

// Mock fetch for CAN DB
globalThis.fetch = vi.fn().mockResolvedValue({
  ok: true,
  json: () => Promise.resolve({ frames: [] }),
});

const sampleFrames: CanFrame[] = [
  { key: 'f1', ts: '12:00:00', bus: 0, busName: 'FSD', dir: 'rx', id: 1021, dlc: 3, data: 'AABBCC' },
  { key: 'f2', ts: '12:00:01', bus: 1, busName: 'Vehicle', dir: 'tx', id: 255, dlc: 8, data: '1122334455667788' },
];

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
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
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
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    expect(screen.getByText('Time')).toBeInTheDocument();
    expect(screen.getByText('Bus')).toBeInTheDocument();
    expect(screen.getByText('Dir')).toBeInTheDocument();
    expect(screen.getByText('ID')).toBeInTheDocument();
    expect(screen.getByText('DLC')).toBeInTheDocument();
    expect(screen.getByText('Data')).toBeInTheDocument();
  });

  it('renders filter preset buttons', () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    BUILT_IN_PRESETS.forEach(p => {
      expect(screen.getByText(p.label)).toBeInTheDocument();
    });
  });

  it('filters by bus when preset is selected', async () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    await userEvent.click(screen.getByText('FSD Bus Only'));
    expect(screen.getByText('FSD')).toBeInTheDocument();
    expect(screen.queryByText('Vehicle')).not.toBeInTheDocument();
  });

  it('filters by direction when RX Only preset is selected', async () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    await userEvent.click(screen.getByText('RX Only'));
    expect(screen.getByText('RX')).toBeInTheDocument();
    expect(screen.queryByText('TX')).not.toBeInTheDocument();
  });

  it('filters by hex ID input', async () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    const input = screen.getByPlaceholderText('Filter by hex ID…');
    await userEvent.type(input, '3FD');
    expect(screen.getByText('0x3FD')).toBeInTheDocument();
    expect(screen.queryByText('0xFF')).not.toBeInTheDocument();
  });

  it('shows "No frames match" when filter eliminates all', async () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    await userEvent.click(screen.getByText('Body Bus Only'));
    expect(screen.getByText(/No frames match/)).toBeInTheDocument();
  });

  it('pause button freezes displayed frames', async () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    const pauseBtn = screen.getByText('Pause');
    await userEvent.click(pauseBtn);
    expect(screen.getByText('Resume')).toBeInTheDocument();
  });

  it('renders export buttons', () => {
    render(<FrameTable {...baseProps} frames={sampleFrames} frameCount={2} />);
    expect(screen.getByText('CSV')).toBeInTheDocument();
    expect(screen.getByText('JSON')).toBeInTheDocument();
  });

  it('export buttons disabled when no frames match', () => {
    render(<FrameTable {...baseProps} />);
    expect(screen.getByText('CSV').closest('button')).toBeDisabled();
    expect(screen.getByText('JSON').closest('button')).toBeDisabled();
  });
});

describe('framesToCsv', () => {
  it('produces CSV header and rows', () => {
    const csv = framesToCsv(sampleFrames);
    const lines = csv.split('\n');
    expect(lines[0]).toBe('Time,Bus,BusName,Dir,ID,DLC,Data');
    expect(lines[1]).toContain('0x3FD');
    expect(lines[2]).toContain('0xFF');
    expect(lines.length).toBe(3);
  });

  it('handles empty array', () => {
    const csv = framesToCsv([]);
    expect(csv).toBe('Time,Bus,BusName,Dir,ID,DLC,Data');
  });
});

describe('framesToJson', () => {
  it('produces valid JSON', () => {
    const json = framesToJson(sampleFrames);
    const parsed = JSON.parse(json);
    expect(parsed).toHaveLength(2);
    expect(parsed[0].id).toBe(1021);
  });
});
