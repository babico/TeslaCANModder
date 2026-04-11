import { describe, it, expect } from 'vitest';
import { buildDecoderIndex, describeDecodedFrame } from '../src/lib/can/decoder.ts';

describe('buildDecoderIndex', () => {
  it('indexes frames by decimal CAN id', () => {
    const index = buildDecoderIndex({
      dataset_source: { vehicle: 'Model S/X' },
      frames: [
        {
          id: 1021, hex: '0x3FD', frame_name: 'APP_wheelButtons',
          bus_name: 'CH', bus_id: 0, signal_count: 2,
          signals: [
            {
              signal_name: 'buttonPressed',
              enum_map_symbol: 'Diag_button_map',
              possible_values_note: '',
              possible_values: [{ value_dec: 1, value_hex: '0x1', label: 'Enabled' }],
            },
          ],
        },
      ],
    });

    const entries = describeDecodedFrame(index, 1021);
    expect(entries.length).toBe(1);
    expect(entries[0].frameName).toBe('APP_wheelButtons');
    expect(entries[0].signals[0].values[0].label).toBe('Enabled');
  });
});

describe('describeDecodedFrame', () => {
  it('returns empty array for unknown ids', () => {
    const index = buildDecoderIndex({ frames: [] });
    expect(describeDecodedFrame(index, 921)).toEqual([]);
  });
});
