import assert from 'node:assert/strict';

import { buildDecoderIndex, describeDecodedFrame } from '../src/lib/can/decoder.ts';

function run(name, callback) {
  try {
    callback();
    console.log(`PASS ${name}`);
  } catch (error) {
    console.error(`FAIL ${name}`);
    throw error;
  }
}

run('buildDecoderIndex indexes frames by decimal CAN id', () => {
  const index = buildDecoderIndex({
    dataset_source: { vehicle: 'Model S/X' },
    frames: [
      {
        id: 1021,
        hex: '0x3FD',
        frame_name: 'APP_wheelButtons',
        bus_name: 'CH',
        bus_id: 0,
        signal_count: 2,
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
  assert.equal(entries.length, 1);
  assert.equal(entries[0].frameName, 'APP_wheelButtons');
  assert.equal(entries[0].signals[0].values[0].label, 'Enabled');
});

run('describeDecodedFrame returns empty array for unknown ids', () => {
  const index = buildDecoderIndex({ frames: [] });
  assert.deepEqual(describeDecodedFrame(index, 921), []);
});
