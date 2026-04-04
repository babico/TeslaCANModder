const DATASET_OPTIONS = {
  mcu2: {
    key: 'mcu2',
    label: 'Model 3 MCU2 (Intel)',
  },
  mcu3: {
    key: 'mcu3',
    label: 'Model 3 MCU3 (AMD)',
  },
  modelsx_amd: {
    key: 'modelsx_amd',
    label: 'Model S/X MCU3 (AMD)',
  },
  modelsx_intel: {
    key: 'modelsx_intel',
    label: 'Model S/X MCU2 (Intel)',
  },
};

export const CAN_DECODER_DATASETS = Object.values(DATASET_OPTIONS);
export const DEFAULT_CAN_DECODER_DATASET = 'modelsx_intel';

const datasetCache = new Map();

function normalizeSignal(signal) {
  const values = Array.isArray(signal?.possible_values) ? signal.possible_values : [];

  return {
    name: signal?.signal_name || 'Unnamed signal',
    enumMap: signal?.enum_map_symbol || '',
    note: signal?.possible_values_note || '',
    values: values.map((value) => ({
      valueDec: value?.value_dec,
      valueHex: value?.value_hex,
      label: value?.label || '',
    })),
  };
}

export function buildDecoderIndex(payload) {
  const frames = Array.isArray(payload?.frames) ? payload.frames : [];
  const byId = new Map();

  frames.forEach((frame) => {
    const id = Number(frame?.id);
    if (!Number.isFinite(id)) {
      return;
    }

    const entry = {
      id,
      frameName: frame?.frame_name || `Frame ${id}`,
      hex: frame?.hex || `0x${id.toString(16).toUpperCase()}`,
      busName: frame?.bus_name || 'Unknown',
      busId: Number(frame?.bus_id),
      signalCount: Number(frame?.signal_count) || 0,
      signals: Array.isArray(frame?.signals) ? frame.signals.map(normalizeSignal) : [],
    };

    const existing = byId.get(id);
    if (existing) {
      existing.push(entry);
    } else {
      byId.set(id, [entry]);
    }
  });

  return {
    dataset: payload?.dataset_source || null,
    generatedUtc: payload?.generated_utc || null,
    counts: payload?.counts || null,
    byId,
  };
}

export function getDecoderDatasetLabel(datasetKey) {
  return DATASET_OPTIONS[datasetKey]?.label || datasetKey;
}

export async function loadDecoderDataset(datasetKey, signal) {
  const key = DATASET_OPTIONS[datasetKey] ? datasetKey : DEFAULT_CAN_DECODER_DATASET;

  if (datasetCache.has(key)) {
    return datasetCache.get(key);
  }

  const response = await fetch(`/can-decoder/${key}.json`, signal ? { signal } : undefined);
  if (!response.ok) {
    throw new Error(`Failed to load decoder dataset ${key}: HTTP ${response.status}`);
  }

  const payload = await response.json();
  const index = buildDecoderIndex(payload);
  datasetCache.set(key, index);
  return index;
}

export function describeDecodedFrame(index, frameId) {
  if (!index?.byId || !Number.isFinite(Number(frameId))) {
    return [];
  }

  return index.byId.get(Number(frameId)) || [];
}
