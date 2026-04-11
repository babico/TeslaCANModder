import {
  buildDecoderIndex,
  describeDecodedFrame,
  getCanIdLabel,
  KNOWN_CAN_IDS,
  type DecoderDataset,
} from '@teslacanmodder/protocol';

const sampleDataset: DecoderDataset = {
  dataset_source: { vehicle: 'Model 3' },
  frames: [
    {
      id: 69,
      hex: '0x045',
      frame_name: 'STW_ACTN_RQ',
      bus_name: 'Vehicle',
      signals: [
        {
          signal_name: 'TurnSignal',
          possible_values: [
            { value_dec: 1, value_hex: '0x01', label: 'Left' },
            { value_dec: 2, value_hex: '0x02', label: 'Right' },
          ],
        },
      ],
    },
    {
      id: 69,
      hex: '0x045',
      frame_name: 'STW_ACTN_RQ_2',
      bus_name: 'Vehicle',
      signals: [{ signal_name: 'HighBeam' }],
    },
    {
      id: 281,
      hex: '0x119',
      frame_name: 'WindowControl',
      signals: [],
    },
  ],
};

describe('buildDecoderIndex', () => {
  it('builds index keyed by CAN ID', () => {
    const index = buildDecoderIndex(sampleDataset);
    expect(index.byId.size).toBe(2);
    expect(index.byId.get(69)).toHaveLength(2);
    expect(index.byId.get(281)).toHaveLength(1);
  });

  it('handles empty dataset', () => {
    const index = buildDecoderIndex({ frames: [] });
    expect(index.byId.size).toBe(0);
  });

  it('coerces string IDs to numbers', () => {
    const ds: DecoderDataset = {
      frames: [{ id: '100' as any, hex: '0x064', frame_name: 'Test', signals: [] }],
    };
    const index = buildDecoderIndex(ds);
    expect(index.byId.has(100)).toBe(true);
  });
});

describe('describeDecodedFrame', () => {
  const index = buildDecoderIndex(sampleDataset);

  it('returns entries for known ID', () => {
    const entries = describeDecodedFrame(index, 69);
    expect(entries).toHaveLength(2);
    expect(entries[0].frameName).toBe('STW_ACTN_RQ');
    expect(entries[0].signals[0].name).toBe('TurnSignal');
    expect(entries[0].signals[0].values).toHaveLength(2);
  });

  it('maps possible_values correctly', () => {
    const entries = describeDecodedFrame(index, 69);
    expect(entries[0].signals[0].values[0]).toEqual({ value: 1, label: 'Left' });
  });

  it('returns empty for unknown ID', () => {
    expect(describeDecodedFrame(index, 9999)).toEqual([]);
  });

  it('handles frame with no signals', () => {
    const entries = describeDecodedFrame(index, 281);
    expect(entries).toHaveLength(1);
    expect(entries[0].signals).toEqual([]);
  });

  it('returns empty values for signal without possible_values', () => {
    const entries = describeDecodedFrame(index, 69);
    expect(entries[1].signals[0].values).toEqual([]);
  });
});

describe('getCanIdLabel', () => {
  it('returns label for known ID', () => {
    expect(getCanIdLabel(69)).toBe('STW_ACTN_RQ (Stalk)');
    expect(getCanIdLabel(1021)).toBe('FSD Control (HW3/HW4)');
  });

  it('returns null for unknown ID', () => {
    expect(getCanIdLabel(9999)).toBeNull();
  });
});

describe('KNOWN_CAN_IDS', () => {
  it('contains expected IDs', () => {
    expect(KNOWN_CAN_IDS).toHaveProperty('69');
    expect(KNOWN_CAN_IDS).toHaveProperty('281');
    expect(KNOWN_CAN_IDS).toHaveProperty('1021');
  });
});
