/** JSON line parser — extracts JSON messages from noisy serial input. */

import type { ParsedEvent } from './types.js';

type JsonRecord = Record<string, unknown>;

function isJsonRecord(value: unknown): value is JsonRecord {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function assignIfDefined(target: JsonRecord, key: string, value: unknown): void {
  if (value !== undefined) {
    target[key] = value;
  }
}

function normalizeBootStatusMessage(message: JsonRecord): JsonRecord {
  const messageType = message.t;
  if (messageType !== 'boot' && messageType !== 'status') {
    return message;
  }

  const meta = isJsonRecord(message.meta) ? message.meta : undefined;
  const connectivity = isJsonRecord(message.connectivity) ? message.connectivity : undefined;
  const state = isJsonRecord(message.state) ? message.state : undefined;
  const driverAssist = isJsonRecord(message.driverAssist) ? message.driverAssist : undefined;
  const vehicle = isJsonRecord(message.vehicle) ? message.vehicle : undefined;
  const battery = isJsonRecord(message.battery) ? message.battery : undefined;
  const safety = isJsonRecord(message.safety) ? message.safety : undefined;
  const can = isJsonRecord(message.can) ? message.can : undefined;
  const features = isJsonRecord(message.features) ? message.features : undefined;

  if (!meta && !connectivity && !state && !driverAssist && !vehicle && !battery && !safety && !can && !features) {
    return message;
  }

  const normalized: JsonRecord = { ...message };

  assignIfDefined(normalized, 'variant', meta?.variant);
  assignIfDefined(normalized, 'hw', meta?.hw);
  assignIfDefined(normalized, 'drv', meta?.drv);
  assignIfDefined(normalized, 'up', meta?.up);

  assignIfDefined(normalized, 'vehicleOnline', connectivity?.vehicleOnline);
  assignIfDefined(normalized, 'bodyOnline', connectivity?.bodyOnline);
  assignIfDefined(normalized, 'chassisOnline', connectivity?.chassisOnline);
  assignIfDefined(normalized, 'standby', connectivity?.standby);

  assignIfDefined(normalized, 'fsd', state?.fsd);
  assignIfDefined(normalized, 'fsdForce', state?.fsdForce);
  assignIfDefined(normalized, 'nag', state?.nag);
  assignIfDefined(normalized, 'isaChime', state?.isaChime);
  assignIfDefined(normalized, 'summonInject', state?.summonInject);
  assignIfDefined(normalized, 'nagKiller', state?.nagKiller);
  assignIfDefined(normalized, 'nagKillerMode', state?.nagKillerMode);
  assignIfDefined(normalized, 'dasHandsOn', state?.dasHandsOn);
  assignIfDefined(normalized, 'precondition', state?.precondition);
  assignIfDefined(normalized, 'trackMode', state?.trackMode);
  assignIfDefined(normalized, 'otaInProgress', state?.otaInProgress);
  assignIfDefined(normalized, 'txPaused', state?.txPaused);
  assignIfDefined(normalized, 'detectedHW', state?.detectedHW);
  assignIfDefined(normalized, 'variantAutoDetect', state?.variantAutoDetect);
  assignIfDefined(normalized, 'gtwAutopilotTier', state?.gtwAutopilotTier);

  const profile = isJsonRecord(state?.profile) ? state.profile : undefined;
  assignIfDefined(normalized, 'sp', profile?.value);
  assignIfDefined(normalized, 'spPin', profile?.pinned);

  const offset = isJsonRecord(state?.offset) ? state.offset : undefined;
  assignIfDefined(normalized, 'offset', offset?.value);
  assignIfDefined(normalized, 'offsetPin', offset?.pinned);

  const stream = isJsonRecord(state?.stream) ? state.stream : undefined;
  if (stream) {
    normalized.stream = {
      ...(isJsonRecord(message.stream) ? message.stream : {}),
      ...stream,
    };
  }

  Object.entries(driverAssist ?? {}).forEach(([key, value]) => assignIfDefined(normalized, key, value));
  Object.entries(vehicle ?? {}).forEach(([key, value]) => assignIfDefined(normalized, key, value));

  assignIfDefined(normalized, 'bmsNomFullPack', battery?.nomFullPack);
  assignIfDefined(normalized, 'bmsNomRemain', battery?.nomRemain);
  assignIfDefined(normalized, 'bmsIdealRemain', battery?.idealRemain);
  assignIfDefined(normalized, 'bmsCellVMax', battery?.cellVMax);
  assignIfDefined(normalized, 'bmsCellVMin', battery?.cellVMin);
  assignIfDefined(normalized, 'bmsMaxRegen', battery?.maxRegen);
  assignIfDefined(normalized, 'bmsMaxDischarge', battery?.maxDischarge);
  assignIfDefined(normalized, 'hasEnhancedBms', battery?.hasEnhanced);

  assignIfDefined(normalized, 'banShield', safety?.banShield);
  assignIfDefined(normalized, 'banThreat', safety?.banThreat);
  assignIfDefined(normalized, 'banDetectCount', safety?.banDetectCount);

  assignIfDefined(normalized, 'canClockReqMHz', can?.clockReqMHz);
  assignIfDefined(normalized, 'canClockMHz', can?.clockMHz);

  if (features) {
    normalized.features = {
      ...(isJsonRecord(message.features) ? message.features : {}),
      ...features,
    };
  }

  return normalized;
}

/** Parse a single serial line, extracting JSON if present. */
export function parseSerialLine(line: string): ParsedEvent[] {
  const cleaned = line.replace(/\0/g, '').trim();
  if (!cleaned) return [];

  const start = cleaned.indexOf('{');
  const end = cleaned.lastIndexOf('}');

  if (start >= 0 && end > start) {
    try {
      const parsed = JSON.parse(cleaned.slice(start, end + 1));
      const msg = isJsonRecord(parsed) ? normalizeBootStatusMessage(parsed) : parsed;
      return [{ type: 'message', message: msg, raw: cleaned }];
    } catch (err) {
      return [{ type: 'parse-error', raw: cleaned, reason: err instanceof Error ? err.message : 'Invalid JSON' }];
    }
  }

  return [{ type: 'ignore', raw: cleaned }];
}

/** Parse a chunk of serial data, returning events and leftover buffer. */
export function parseSerialChunk(
  remainder: string,
  chunk: string,
): { remainder: string; events: ParsedEvent[] } {
  const combined = remainder + chunk;
  const lines = combined.split('\n');
  const newRemainder = lines.pop() || '';
  const events: ParsedEvent[] = [];

  for (const line of lines) {
    events.push(...parseSerialLine(line));
  }

  return { remainder: newRemainder, events };
}
