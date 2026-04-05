/** Diagnosis engine — builds a health matrix and produces a human-readable verdict. */

export function buildMatrix(messages, variantRequested) {
  const parsed     = messages.filter(m => m.msg);
  const bootSeen   = parsed.some(m => m.msg.t === 'boot') || messages.some(m => m.raw.includes('"t":"boot"'));
  const pongSeen   = parsed.some(m => m.msg.t === 'pong');
  const statuses   = parsed.filter(m => m.msg.t === 'status');
  const latest     = statuses.length ? statuses[statuses.length - 1].msg : null;
  const frames     = parsed.filter(m => m.msg.t === 'frame');
  const errors     = parsed.filter(m => m.msg.t === 'error');
  const parseErrs  = messages.filter(m => m.type === 'parse-error');

  const rawCanAck      = parsed.some(m => m.msg.t === 'ack' && m.msg.cmd === 'can:raw:on');
  const rawCanInStatus = statuses.some(m => m.msg.rawCan !== undefined);
  const rawCanSupported = rawCanAck || rawCanInStatus;
  const rawCanEnabled   = statuses.some(m => Number(m.msg.rawCan) === 1);

  const filteredFrames = frames.filter(m => m._phase?.startsWith('filtered'));
  const rawFrames      = frames.filter(m => m._phase?.startsWith('raw'));

  const variantOk = !variantRequested || (latest && latest.variant === variantRequested);
  const unknownCmds = errors.filter(m => m.msg.msg === 'Unknown command').length;

  return {
    bootSeen,
    pongSeen,
    protocolOk: bootSeen && pongSeen,
    variantOk,
    latestStatus: latest,
    filteredFrameCount: filteredFrames.length,
    rawFrameCount: rawFrames.length,
    totalFrameCount: frames.length,
    rawCanSupported,
    rawCanEnabled,
    unknownCommandCount: unknownCmds,
    parseErrorCount: parseErrs.length,
    errorCount: errors.length,
    noiseLikely: unknownCmds > 0 || parseErrs.length > 0,
    filterMismatchLikely: rawCanSupported && rawCanEnabled && rawFrames.length > 0 && filteredFrames.length === 0,
    physicalCanIssueLikely: rawCanSupported && rawCanEnabled && rawFrames.length === 0,
    firmwareNeedsRawMode: !rawCanSupported,
  };
}

export function diagnose(matrix) {
  if (!matrix.protocolOk) {
    return 'Serial protocol handshake failed. Check the COM port, cable, and that the board is powered.';
  }
  if (matrix.firmwareNeedsRawMode) {
    return 'Board does not support can:raw command. Flash updated firmware to enable raw CAN diagnostics.';
  }
  if (matrix.filterMismatchLikely) {
    return 'Raw frames seen but no filtered frames. Likely a variant or filter-ID mismatch — switch variant and retry.';
  }
  if (matrix.physicalCanIssueLikely) {
    return 'No frames seen in raw CAN mode. Likely a wiring, termination, power, or vehicle CAN-bus access issue.';
  }
  if (matrix.filteredFrameCount > 0) {
    return 'Filtered CAN frames seen — basic CAN access is working.';
  }
  if (matrix.latestStatus?.ready === 'runtime-ready') {
    return 'Board previously saw CAN traffic but none arrived in this window. Try a longer watch duration.';
  }
  return 'Inconclusive — inspect raw messages for clues.';
}

export function tagPhase(session, phase) {
  session.messages().forEach(m => { if (!m._phase) m._phase = phase; });
}
