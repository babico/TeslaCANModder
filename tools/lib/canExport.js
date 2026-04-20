/**
 * canExport — Convert Tesla CAN decoder JSON datasets to DBC, CSV, or summary JSON.
 *
 * All functions accept a `DecoderDataset`-shaped plain object and return a string.
 *
 * Schema:
 *   dataset.dataset_source   { vehicle, firmware, mcu, soc }
 *   dataset.frames[]         { id, hex, frame_name, bus_name?, signals[] }
 *   signals[]                { signal_name, enum_map_symbol?, possible_values_note?, possible_values[] }
 *   possible_values[]        { value_dec, value_hex, label }
 */

// ── DBC Export ────────────────────────────────────────────────────────────────

/**
 * Convert a decoder dataset to a partial DBC string.
 *
 * Because the decoder JSON carries label/enum data only (no bit positions or
 * signal widths), signal SG_ entries use placeholder bit layout `0|8@1+` and
 * are intentionally marked with a comment.  VAL_ lines are fully populated.
 *
 * @param {object} dataset
 * @returns {string}
 */
export function toDbcString(dataset) {
  const src = dataset.dataset_source ?? {};
  const frames = dataset.frames ?? [];
  const lines = [];

  const comment = `; Tesla CAN decoder export`
    + (src.vehicle  ? `  vehicle=${src.vehicle}`  : '')
    + (src.firmware ? `  firmware=${src.firmware}` : '')
    + (src.mcu      ? `  mcu=${src.mcu}`           : '')
    + (src.soc      ? `  soc=${src.soc}`           : '');

  lines.push(comment);
  lines.push('; NOTE: Signal bit positions and widths are UNKNOWN (placeholder 0|8@1+).');
  lines.push('; Only enum value labels (VAL_) are accurate.');
  lines.push('');
  lines.push('VERSION ""');
  lines.push('');
  lines.push('NS_ :');
  lines.push('');
  lines.push('BS_:');
  lines.push('');
  lines.push('BU_:');
  lines.push('');

  const valLines = [];

  for (const frame of frames) {
    const msgId   = Number(frame.id);
    const msgName = sanitizeDbcId(frame.frame_name ?? `MSG_${msgId}`);
    const busNode = sanitizeDbcId(frame.bus_name ?? 'Vector__XXX');

    lines.push(`BO_ ${msgId} ${msgName}: 8 ${busNode}`);

    for (const sig of (frame.signals ?? [])) {
      const sigName = sanitizeDbcId(sig.signal_name ?? 'SIG');
      // Placeholder layout: start=0, len=8, little-endian, unsigned
      lines.push(` SG_ ${sigName} : 0|8@1+ (1,0) [0|0] "" Vector__XXX`);

      const pairs = (sig.possible_values ?? [])
        .filter(v => v.label)
        .map(v => `${v.value_dec} "${escapeDbcString(v.label)}"`)
        .join(' ');

      if (pairs) {
        valLines.push(`VAL_ ${msgId} ${sigName} ${pairs} ;`);
      }
    }

    lines.push('');
  }

  if (valLines.length > 0) {
    lines.push('');
    for (const vl of valLines) lines.push(vl);
  }

  return lines.join('\n');
}

// ── CSV Export ────────────────────────────────────────────────────────────────

const CSV_HEADER =
  'frame_id_dec,frame_id_hex,frame_name,bus_name,signal_name,enum_symbol,possible_values_note,value_dec,value_hex,value_label';

/**
 * Convert a decoder dataset to a flat CSV string.
 * One row per enum value; signals with no enum values emit one row with empty value columns.
 *
 * @param {object} dataset
 * @returns {string}
 */
export function toCsvString(dataset) {
  const frames = dataset.frames ?? [];
  const rows = [CSV_HEADER];

  for (const frame of frames) {
    const frameId   = Number(frame.id);
    const frameHex  = frame.hex  ?? `0x${frameId.toString(16).toUpperCase()}`;
    const frameName = frame.frame_name ?? '';
    const busName   = frame.bus_name   ?? '';

    for (const sig of (frame.signals ?? [])) {
      const sigName    = sig.signal_name          ?? '';
      const enumSym    = sig.enum_map_symbol       ?? '';
      const pvNote     = sig.possible_values_note  ?? '';
      const values     = sig.possible_values        ?? [];

      if (values.length === 0) {
        rows.push([
          frameId, frameHex, csvField(frameName), csvField(busName),
          csvField(sigName), csvField(enumSym), csvField(pvNote),
          '', '', '',
        ].join(','));
      } else {
        for (const pv of values) {
          rows.push([
            frameId, frameHex, csvField(frameName), csvField(busName),
            csvField(sigName), csvField(enumSym), csvField(pvNote),
            pv.value_dec, csvField(pv.value_hex ?? ''), csvField(pv.label ?? ''),
          ].join(','));
        }
      }
    }
  }

  return rows.join('\n');
}

// ── Summary JSON Export ───────────────────────────────────────────────────────

/**
 * Convert a decoder dataset to a compact summary JSON object (not stringified).
 * Strips bulk fields; retains only the information needed for human review.
 *
 * Shape:
 *   { source, counts, frames: [{ id, hex, name, bus, signals: [{ name, values: {dec: label} }] }] }
 *
 * @param {object} dataset
 * @returns {object}
 */
export function toSummaryJson(dataset) {
  const src    = dataset.dataset_source ?? {};
  const frames = dataset.frames ?? [];

  const summaryFrames = frames.map((frame) => {
    const signals = (frame.signals ?? []).map((sig) => {
      const values = {};
      for (const pv of (sig.possible_values ?? [])) {
        if (pv.label) values[pv.value_dec] = pv.label;
      }
      const entry = { name: sig.signal_name ?? '' };
      if (Object.keys(values).length > 0) entry.values = values;
      if (sig.possible_values_note && !sig.possible_values_note.startsWith('No enum'))
        entry.note = sig.possible_values_note;
      return entry;
    });

    return {
      id:      Number(frame.id),
      hex:     frame.hex ?? '',
      name:    frame.frame_name ?? '',
      bus:     frame.bus_name   ?? '',
      signals,
    };
  });

  return {
    source: {
      vehicle:  src.vehicle  ?? null,
      firmware: src.firmware ?? null,
      mcu:      src.mcu      ?? null,
      soc:      src.soc      ?? null,
    },
    counts: {
      frames:  frames.length,
      signals: frames.reduce((n, f) => n + (f.signals?.length ?? 0), 0),
    },
    frames: summaryFrames,
  };
}

// ── Helpers ───────────────────────────────────────────────────────────────────

/** Replace characters illegal in DBC identifiers with underscores. */
function sanitizeDbcId(str) {
  return String(str).replace(/[^A-Za-z0-9_]/g, '_');
}

/** Escape double-quotes inside a DBC string literal. */
function escapeDbcString(str) {
  return String(str).replace(/"/g, '\\"');
}

/** Wrap a CSV field in quotes if it contains commas, quotes, or newlines. */
function csvField(val) {
  const s = String(val ?? '');
  if (s.includes(',') || s.includes('"') || s.includes('\n')) {
    return `"${s.replace(/"/g, '""')}"`;
  }
  return s;
}
