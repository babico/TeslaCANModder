import { describe, it, expect } from '@jest/globals';
import { toDbcString, toCsvString, toSummaryJson } from '../lib/canExport.js';

// ── Sample dataset fixture ────────────────────────────────────────────────────

const MINIMAL = {
  dataset_source: { vehicle: 'Model 3', firmware: '2026.2', mcu: 'MCU2', soc: 'Intel' },
  frames: [
    {
      id: 280,
      hex: '0x118',
      frame_name: 'DI_systemStatus',
      bus_name: 'ETH',
      bus_id: 3,
      signal_count: 2,
      signals: [
        {
          signal_name: 'DI_gear',
          enum_map_symbol: 'Diag_DI_gear_map',
          possible_values_note: '',
          possible_values: [
            { value_dec: 0, value_hex: '0x0', label: 'INVALID' },
            { value_dec: 1, value_hex: '0x1', label: 'P' },
            { value_dec: 4, value_hex: '0x4', label: 'D' },
          ],
        },
        {
          signal_name: 'DI_brakePedalState',
          enum_map_symbol: 'Diag_DI_brakePedalState_map',
          possible_values_note: '',
          possible_values: [
            { value_dec: 0, value_hex: '0x0', label: 'OFF' },
            { value_dec: 1, value_hex: '0x1', label: 'ON' },
          ],
        },
      ],
    },
    {
      id: 640,
      hex: '0x280',
      frame_name: 'APB_status',
      bus_name: 'ETH',
      bus_id: 3,
      signal_count: 1,
      signals: [
        {
          signal_name: 'APB_numeric_only',
          enum_map_symbol: '',
          possible_values_note: 'No enum map in this binary (numeric/bitfield signal or unresolved metadata).',
          possible_values: [],
        },
      ],
    },
  ],
};

const EMPTY = { dataset_source: {}, frames: [] };

// ── toDbcString ───────────────────────────────────────────────────────────────

describe('toDbcString', () => {
  it('produces a VERSION header', () => {
    expect(toDbcString(MINIMAL)).toContain('VERSION ""');
  });

  it('includes BO_ block for each frame', () => {
    const dbc = toDbcString(MINIMAL);
    expect(dbc).toContain('BO_ 280 DI_systemStatus:');
    expect(dbc).toContain('BO_ 640 APB_status:');
  });

  it('includes SG_ entry for each signal', () => {
    const dbc = toDbcString(MINIMAL);
    expect(dbc).toContain('SG_ DI_gear');
    expect(dbc).toContain('SG_ DI_brakePedalState');
    expect(dbc).toContain('SG_ APB_numeric_only');
  });

  it('includes VAL_ lines for signals with enum values', () => {
    const dbc = toDbcString(MINIMAL);
    expect(dbc).toContain('VAL_ 280 DI_gear');
    expect(dbc).toContain('0 "INVALID"');
    expect(dbc).toContain('1 "P"');
    expect(dbc).toContain('4 "D"');
  });

  it('does NOT emit VAL_ for signals with no possible_values', () => {
    const dbc = toDbcString(MINIMAL);
    expect(dbc).not.toMatch(/VAL_ 640 APB_numeric_only/);
  });

  it('embeds dataset_source info in a comment', () => {
    const dbc = toDbcString(MINIMAL);
    expect(dbc).toContain('vehicle=Model 3');
    expect(dbc).toContain('firmware=2026.2');
  });

  it('handles empty dataset without error', () => {
    const dbc = toDbcString(EMPTY);
    expect(dbc).toContain('VERSION ""');
    expect(dbc).not.toContain('BO_');
  });

  it('sanitizes non-identifier characters in frame/signal names', () => {
    const weird = {
      frames: [{
        id: 1, hex: '0x1', frame_name: 'My Frame!', bus_name: 'CAN-0',
        signals: [{ signal_name: 'val/raw', possible_values: [] }],
      }],
    };
    const dbc = toDbcString(weird);
    expect(dbc).toContain('BO_ 1 My_Frame_:');
    expect(dbc).toContain('SG_ val_raw');
  });
});

// ── toCsvString ───────────────────────────────────────────────────────────────

describe('toCsvString', () => {
  it('starts with the expected header row', () => {
    const csv = toCsvString(MINIMAL);
    expect(csv.split('\n')[0]).toBe(
      'frame_id_dec,frame_id_hex,frame_name,bus_name,signal_name,enum_symbol,possible_values_note,value_dec,value_hex,value_label',
    );
  });

  it('emits one row per enum value', () => {
    const csv = toCsvString(MINIMAL);
    const rows = csv.split('\n').slice(1).filter(Boolean);
    // DI_gear: 3 values, DI_brakePedalState: 2 values, APB_numeric_only: 1 empty row
    expect(rows).toHaveLength(6);
  });

  it('includes frame id and hex in each row', () => {
    const csv = toCsvString(MINIMAL);
    expect(csv).toContain('280,0x118,DI_systemStatus');
  });

  it('emits a row with empty value columns for no-enum signals', () => {
    const csv = toCsvString(MINIMAL);
    const rows = csv.split('\n').slice(1).filter(Boolean);
    const noEnumRow = rows.find(r => r.includes('APB_numeric_only'));
    expect(noEnumRow).toBeDefined();
    // last three columns (value_dec, value_hex, value_label) should be empty
    expect(noEnumRow).toMatch(/,,,$/);
  });

  it('quotes fields containing commas', () => {
    const dataset = {
      frames: [{
        id: 1, hex: '0x1', frame_name: 'A,B', bus_name: 'bus',
        signals: [{
          signal_name: 'sig', enum_map_symbol: '', possible_values_note: '',
          possible_values: [{ value_dec: 0, value_hex: '0x0', label: 'label,with,commas' }],
        }],
      }],
    };
    const csv = toCsvString(dataset);
    expect(csv).toContain('"A,B"');
    expect(csv).toContain('"label,with,commas"');
  });

  it('handles empty dataset without error', () => {
    const csv = toCsvString(EMPTY);
    const rows = csv.split('\n').filter(Boolean);
    expect(rows).toHaveLength(1); // header only
  });
});

// ── toSummaryJson ─────────────────────────────────────────────────────────────

describe('toSummaryJson', () => {
  it('returns source metadata', () => {
    const result = toSummaryJson(MINIMAL);
    expect(result.source.vehicle).toBe('Model 3');
    expect(result.source.firmware).toBe('2026.2');
    expect(result.source.mcu).toBe('MCU2');
  });

  it('returns accurate counts', () => {
    const result = toSummaryJson(MINIMAL);
    expect(result.counts.frames).toBe(2);
    expect(result.counts.signals).toBe(3);
  });

  it('maps frames with id, hex, name, bus', () => {
    const result = toSummaryJson(MINIMAL);
    expect(result.frames[0].id).toBe(280);
    expect(result.frames[0].hex).toBe('0x118');
    expect(result.frames[0].name).toBe('DI_systemStatus');
    expect(result.frames[0].bus).toBe('ETH');
  });

  it('maps enum values as a { dec: label } dictionary', () => {
    const result = toSummaryJson(MINIMAL);
    const gearSig = result.frames[0].signals.find(s => s.name === 'DI_gear');
    expect(gearSig).toBeDefined();
    expect(gearSig.values[0]).toBe('INVALID');
    expect(gearSig.values[1]).toBe('P');
    expect(gearSig.values[4]).toBe('D');
  });

  it('omits values key for numeric-only signals', () => {
    const result = toSummaryJson(MINIMAL);
    const numSig = result.frames[1].signals[0];
    expect(numSig.values).toBeUndefined();
  });

  it('omits boilerplate "No enum map" note from summary', () => {
    const result = toSummaryJson(MINIMAL);
    const numSig = result.frames[1].signals[0];
    expect(numSig.note).toBeUndefined();
  });

  it('handles empty dataset without error', () => {
    const result = toSummaryJson(EMPTY);
    expect(result.counts.frames).toBe(0);
    expect(result.frames).toHaveLength(0);
  });

  it('is JSON-serializable', () => {
    expect(() => JSON.stringify(toSummaryJson(MINIMAL))).not.toThrow();
  });
});
