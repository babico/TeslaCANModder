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
    speedOffset: 0,
    isaChimeEnabled: false,
    streamEnabled: false,
    streamedFrameCount: 0,
    features: {
      fsd: true,
      profile: true,
      nag: true,
      speedOffset: false,
      isaSpeedChime: false,
      forceFsd: false,
    },
    rawStatus: null,
  };
}

export function createFrameMonitorState() {
  return {
    frames: [],
    groups: [],
    totalReceived: 0,
    trimmedCount: 0,
    parseErrors: 0,
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

function frameGroupKey(frame) {
  return `${frame.extended ? 'ext' : 'std'}:${frame.id}`;
}

export function computeByteDiff(previousBytes = [], nextBytes = []) {
  const limit = Math.max(previousBytes.length, nextBytes.length);
  const changedIndexes = [];

  for (let index = 0; index < limit; index += 1) {
    if ((previousBytes[index] ?? null) !== (nextBytes[index] ?? null)) {
      changedIndexes.push(index);
    }
  }

  return changedIndexes;
}

export function buildFrameGroups(frames) {
  const groups = new Map();

  frames.forEach((frame) => {
    const key = frameGroupKey(frame);
    const existing = groups.get(key);

    if (existing) {
      existing.count += 1;
      if (frame.boardMs >= existing.lastBoardMs) {
        existing.lastBoardMs = frame.boardMs;
        existing.lastSequence = frame.sequence;
        existing.lastDir = frame.dir;
        existing.lastDataHex = frame.dataHex;
        existing.dlc = frame.dlc;
        existing.ts = frame.ts;
      }
      return;
    }

    groups.set(key, {
      key,
      id: frame.id,
      extended: frame.extended,
      count: 1,
      dlc: frame.dlc,
      lastDir: frame.dir,
      lastDataHex: frame.dataHex,
      lastBoardMs: frame.boardMs,
      lastSequence: frame.sequence,
      ts: frame.ts,
    });
  });

  return [...groups.values()].sort((left, right) => (
    right.lastBoardMs - left.lastBoardMs || right.count - left.count || right.id - left.id
  ));
}

export function normalizeStatusMessage(message, rate) {
  const rawFeatures = message.features && typeof message.features === 'object' ? message.features : {};
  const rawStream = message.stream && typeof message.stream === 'object' ? message.stream : {};

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
    speedOffset: Number(message.offset) || 0,
    isaChimeEnabled: Boolean(Number(message.isaChime)),
    streamEnabled: Number(rawStream.on ?? 0) === 1,
    streamedFrameCount: Number(rawStream.emitted ?? 0) || 0,
    features: {
      fsd: Number(rawFeatures.fsd ?? 1) === 1,
      profile: Number(rawFeatures.profile ?? 1) === 1,
      nag: Number(rawFeatures.nag ?? 1) === 1,
      speedOffset: Number(rawFeatures.speedOffset ?? 0) === 1,
      isaSpeedChime: Number(rawFeatures.isaSpeedChime ?? 0) === 1,
      forceFsd: Number(rawFeatures.forceFsd ?? 0) === 1,
    },
    rawStatus: message,
  };
}

export function normalizeFrameMessage(message) {
  const timestamp = new Date();
  const dataHex = String(message.d || '').toUpperCase();
  const bytes = parseFrameBytes(dataHex);
  const boardMs = Number(message.ms);
  const sequence = Number(message.seq);
  const frameId = Number(message.id);
  const parsedDlc = Number(message.dlc);

  if (!Number.isFinite(frameId)) {
    return null;
  }

  if (dataHex && bytes.length === 0) {
    return null;
  }

  return {
    id: frameId,
    dlc: Number.isFinite(parsedDlc) ? parsedDlc : bytes.length || 8,
    dir: message.dir || 'rx',
    ts: Number.isFinite(boardMs) ? `${boardMs} ms` : formatClockTime(timestamp),
    seenAt: timestamp.getTime(),
    boardMs: Number.isFinite(boardMs) ? boardMs : timestamp.getTime(),
    receivedAt: timestamp.getTime(),
    sequence: Number.isFinite(sequence) ? sequence : null,
    extended: Number(message.ext) === 1,
    dataHex,
    bytes,
    changedByteIndexes: [],
  };
}

export function ingestFrameMonitorMessage(previousState, message) {
  return ingestFrameMonitorBatch(previousState, [message]);
}

export function ingestFrameMonitorBatch(previousState, messages = [], parseErrorCount = 0) {
  if (!messages.length && !parseErrorCount) {
    return previousState;
  }

  const frames = [...previousState.frames];
  const latestRxByKey = new Map();

  frames.forEach((existingFrame) => {
    if (existingFrame.dir === 'rx') {
      latestRxByKey.set(frameGroupKey(existingFrame), existingFrame);
    }
  });

  let nextParseErrors = previousState.parseErrors + parseErrorCount;
  let acceptedFrames = 0;

  messages.forEach((message) => {
    const frame = normalizeFrameMessage(message);
    if (!frame) {
      nextParseErrors += 1;
      return;
    }

    if (frame.dir === 'tx') {
      const referenceFrame = latestRxByKey.get(frameGroupKey(frame));
      frame.changedByteIndexes = computeByteDiff(referenceFrame?.bytes || [], frame.bytes);
    }

    if (frame.dir === 'rx') {
      latestRxByKey.set(frameGroupKey(frame), frame);
    }

    frames.unshift(frame);
    acceptedFrames += 1;
  });

  let trimmedCount = previousState.trimmedCount;
  if (frames.length > MAX_FRAME_BUFFER) {
    trimmedCount += frames.length - MAX_FRAME_BUFFER;
    trimList(frames, MAX_FRAME_BUFFER);
  }

  return {
    frames,
    groups: buildFrameGroups(frames),
    totalReceived: previousState.totalReceived + acceptedFrames,
    trimmedCount,
    parseErrors: nextParseErrors,
  };
}
