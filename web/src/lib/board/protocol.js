export const MAX_FRAME_BUFFER = 200;
export const MAX_CONSOLE_LINES = 200;

export function createBaseTelemetry() {
  return {
    uptimeMs: 0,
    rate: 0,
    variant: null,
    driver: null,
    hardware: null,
    canBus: null,
    installReadiness: null,
    transportCapability: null,
    bluetoothEnabled: false,
    rawStatus: null,
  };
}

export function formatClockTime(date) {
  return `${date.toLocaleTimeString('en-GB', { hour12: false })}.${String(date.getMilliseconds()).padStart(3, '0')}`;
}

export function parseFrameBytes(hexPayload = '') {
  const clean = String(hexPayload).replace(/\s+/g, '').toUpperCase();
  if (!clean || clean.length % 2 !== 0) {
    return [];
  }

  const bytes = [];
  for (let index = 0; index < clean.length; index += 2) {
    const value = Number.parseInt(clean.slice(index, index + 2), 16);
    if (Number.isNaN(value)) {
      return [];
    }
    bytes.push(value);
  }

  return bytes;
}

export function trimList(list, limit) {
  if (list.length > limit) {
    list.splice(limit);
  }
}

export function normalizeStatusMessage(message, rate) {
  return {
    uptimeMs: Number(message.up) || 0,
    rate,
    variant: message.variant || null,
    driver: message.drv || null,
    hardware: message.hw || null,
    canBus: message.can || null,
    installReadiness: message.ready || null,
    transportCapability: message.cap || null,
    bluetoothEnabled: Boolean(Number(message.bt)),
    rawStatus: message,
  };
}

export function normalizeFrameMessage(message) {
  const timestamp = new Date();
  const dataHex = String(message.d || '').toUpperCase();
  const bytes = parseFrameBytes(dataHex);

  return {
    id: Number(message.id),
    dlc: Number(message.dlc) || bytes.length || 8,
    dir: message.dir || 'rx',
    ts: formatClockTime(timestamp),
    seenAt: timestamp.getTime(),
    dataHex,
    bytes,
  };
}
