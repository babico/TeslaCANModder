/** CAN frame decoder — maps CAN IDs to known signal descriptions. */

export interface DecoderSignal {
  signal_name: string;
  enum_map_symbol?: string;
  possible_values_note?: string;
  possible_values?: Array<{ value_dec: number; value_hex: string; label: string }>;
}

export interface DecoderFrame {
  id: number;
  hex: string;
  frame_name: string;
  bus_name?: string;
  bus_id?: number;
  signal_count?: number;
  signals: DecoderSignal[];
}

export interface DecoderDataset {
  dataset_source?: { vehicle?: string };
  frames: DecoderFrame[];
}

export interface DecoderIndex {
  byId: Map<number, DecoderFrame[]>;
}

export interface DecodedEntry {
  frameName: string;
  busName?: string;
  signals: Array<{
    name: string;
    values: Array<{ value: number; label: string }>;
  }>;
}

/** Build an index keyed by decimal CAN ID. */
export function buildDecoderIndex(data: DecoderDataset): DecoderIndex {
  const byId = new Map<number, DecoderFrame[]>();
  for (const frame of data.frames) {
    const id = Number(frame.id);
    if (!byId.has(id)) byId.set(id, []);
    byId.get(id)!.push(frame);
  }
  return { byId };
}

/** Look up decoded info for a CAN ID. */
export function describeDecodedFrame(index: DecoderIndex, canId: number): DecodedEntry[] {
  const frames = index.byId.get(canId);
  if (!frames) return [];
  return frames.map((f) => ({
    frameName: f.frame_name,
    busName: f.bus_name,
    signals: f.signals.map((s) => ({
      name: s.signal_name,
      values: (s.possible_values || []).map((v) => ({
        value: v.value_dec,
        label: v.label,
      })),
    })),
  }));
}

/** Built-in known CAN IDs used by TeslaCANModder firmware. */
export const KNOWN_CAN_IDS: Record<number, string> = {
  69:   'STW_ACTN_RQ (Stalk)',
  281:  'Window Control',
  627:  'UI_vehicleControl',
  644:  'Sentry Mode',
  755:  'UI_hvacRequest',
  819:  'UI_chargeRequest',
  820:  'UI_powertrainControl',
  921:  'ISA Chime',
  947:  'Trunk/Glovebox',
  1006: 'FSD Control (Legacy)',
  1016: 'Follow Distance',
  1021: 'FSD Control (HW3/HW4)',
};

/** Get a human-readable label for a CAN ID. */
export function getCanIdLabel(id: number): string | null {
  return KNOWN_CAN_IDS[id] || null;
}
