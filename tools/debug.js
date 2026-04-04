#!/usr/bin/env node
/**
 * TeslaCANModder — Unified Board Debug Tool
 *
 * Cross-platform serial debug tool for the Arduino Uno firmware.
 * Replaces serial-debug.ps1 (PowerShell, Windows-only) and board-smoke.js.
 *
 * Requires: cd web && npm install   (serialport is a devDependency in web/package.json)
 *
 * Usage:
 *   node tools/debug.js <command> [options]
 *
 * Commands:
 *   smoke   Protocol health-check  (default when no command given)
 *   watch   Live frame / state monitor
 *   test    FSD / profile functional round-trip test
 *
 * Global options:
 *   --port    <path>   Serial port path (required)  e.g. COM3 /dev/ttyUSB0
 *   --baud    <n>      Baud rate                    [default: 115200]
 *   --timeout <ms>     Per-step response timeout    [default: 3000]
 *   --variant <name>   Set variant first: hw4 | hw3 | legacy
 *   --warmup  <ms>     Settle time after port open  [default: 2000]
 *   --json             Print JSON summary at the end
 *   --no-color         Disable ANSI colour
 *
 * `watch` extras:
 *   --filter   <ids>   CAN IDs to display, comma-separated (e.g. 921,1016)
 *   --raw              Enable can:raw mode while watching
 *   --duration <ms>    How long to watch             [default: 10000]
 *   --watch-fsd        Print FSD state changes
 *   --watch-profile    Print profile state changes
 *
 * `test` extras:
 *   --test-fsd         Run FSD on → verify → off round-trip
 *   --test-profile <n> Run profile:N → verify round-trip (0–4)
 *   --restore          Restore original FSD / profile after test
 *
 * Examples:
 *   node tools/debug.js smoke --port COM3
 *   node tools/debug.js watch --port COM3 --filter 921,1016 --raw --duration 15000
 *   node tools/debug.js test  --port COM3 --variant hw4 --test-fsd --restore
 *   node tools/debug.js smoke --port COM3 --json > report.json
 */

import { createInterface } from 'node:readline';
import { createRequire } from 'node:module';
import { setTimeout as delay } from 'node:timers/promises';

// ── Argument parsing ─────────────────────────────────────────────────────────

function parseArgs(argv) {
  const result = { _: [] };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    if (arg.startsWith('--')) {
      const key = arg.slice(2);
      const next = argv[i + 1];
      if (next !== undefined && !next.startsWith('--')) {
        result[key] = next;
        i++;
      } else {
        result[key] = true;
      }
    } else {
      result._.push(arg);
    }
  }
  return result;
}

const args = parseArgs(process.argv.slice(2));
const COMMAND = args._[0] || 'smoke';

if (!args.port) {
  console.error('ERROR: --port is required');
  console.error('Usage: node tools/debug.js <smoke|watch|test> --port COM3 [options]');
  console.error('Run with --help for full option list.');
  process.exit(1);
}

const PORT          = String(args.port);
const BAUD          = Number(args.baud)    || 115200;
const TIMEOUT_MS    = Number(args.timeout) || 3000;
const WARMUP_MS     = Number(args.warmup)  || 2000;
const VARIANT       = args.variant ? String(args.variant) : null;
const JSON_OUTPUT   = Boolean(args.json);
const NO_COLOR      = Boolean(args['no-color']);
const WATCH_DUR_MS  = Number(args.duration) || 10000;
const WATCH_RAW     = Boolean(args.raw);
const WATCH_FSd     = Boolean(args['watch-fsd']);
const WATCH_PROFILE = Boolean(args['watch-profile']);
const FILTER_IDS    = (args.filter ? String(args.filter) : '')
  .split(',').map((s) => Number(s.trim())).filter((n) => Number.isFinite(n) && n > 0);

const TEST_FSd      = Boolean(args['test-fsd']);
const TEST_PROFILE  = args['test-profile'] !== undefined ? Number(args['test-profile']) : -1;
const RESTORE       = Boolean(args.restore);

// ── ANSI colour ──────────────────────────────────────────────────────────────

const C = NO_COLOR ? {
  green: '', red: '', yellow: '', cyan: '', dim: '', bold: '', reset: '',
} : {
  green:  '\x1b[32m',
  red:    '\x1b[31m',
  yellow: '\x1b[33m',
  cyan:   '\x1b[36m',
  dim:    '\x1b[2m',
  bold:   '\x1b[1m',
  reset:  '\x1b[0m',
};

function ts() {
  return new Date().toTimeString().slice(0, 12);
}

// ── Serial connection ────────────────────────────────────────────────────────

async function openSerial(port, baud) {
  // Resolve serialport from web/node_modules (single npm install location)
  const require = createRequire(new URL('../web/package.json', import.meta.url));
  try {
    const { SerialPort } = require('serialport');
    return await new Promise((resolve, reject) => {
      const sp = new SerialPort({ path: port, baudRate: baud }, (err) => {
        if (err) reject(err);
        else resolve(sp);
      });
    });
  } catch (importErr) {
    if (importErr.code === 'MODULE_NOT_FOUND') {
      throw new Error(
        `'serialport' package not found.\n` +
        `Install it with:\n\n  cd web && npm install\n`
      );
    }
    throw importErr;
  }
}

// ── Board session ────────────────────────────────────────────────────────────

class BoardSession {
  constructor(sp) {
    this._sp = sp;
    this._queue = [];      // parsed { raw, msg | null } objects
    this._waiters = [];    // pending { predicate, resolve, timer }
    this._allMessages = [];

    const rl = createInterface({ input: sp, crlfDelay: Infinity });
    rl.on('line', (raw) => this._onRawLine(raw));
    rl.on('close', () => { /* port closed */ });
  }

  _onRawLine(rawLine) {
    const line = rawLine.replace(/\r/g, '').trim();
    if (!line) return;

    let msg = null;
    let type = 'text';

    // Extract JSON from the line (may have prefix garbage on first boot line)
    const start = line.indexOf('{');
    const end   = line.lastIndexOf('}');
    if (start >= 0 && end > start) {
      try {
        msg = JSON.parse(line.slice(start, end + 1));
        type = msg.t || 'unknown';
      } catch {
        type = 'parse-error';
      }
    }

    const entry = { raw: line, msg, type };
    this._allMessages.push(entry);
    this._dispatch(entry);
  }

  _dispatch(entry) {
    for (let i = 0; i < this._waiters.length; i++) {
      const waiter = this._waiters[i];
      if (waiter.predicate(entry)) {
        this._waiters.splice(i, 1);
        clearTimeout(waiter.timer);
        waiter.resolve(entry);
        return;
      }
    }
    this._queue.push(entry);
  }

  /** Wait for an entry matching predicate, or null on timeout. */
  waitFor(predicate, timeoutMs = TIMEOUT_MS) {
    // Check already-queued entries first
    const idx = this._queue.findIndex(predicate);
    if (idx >= 0) {
      return Promise.resolve(this._queue.splice(idx, 1)[0]);
    }

    return new Promise((resolve) => {
      const timer = setTimeout(() => {
        const pos = this._waiters.findIndex((w) => w.timer === timer);
        if (pos >= 0) this._waiters.splice(pos, 1);
        resolve(null);
      }, timeoutMs);

      this._waiters.push({ predicate, resolve, timer });
    });
  }

  /** Wait for a JSON message with a specific 't' type. */
  waitForType(t, timeoutMs) {
    return this.waitFor((e) => e.msg?.t === t, timeoutMs);
  }

  /** Wait for an ack for a specific command. */
  waitForAck(cmd, timeoutMs) {
    return this.waitFor(
      (e) => e.msg?.t === 'ack' && e.msg?.cmd === cmd,
      timeoutMs,
    );
  }

  /** Drain all queued entries of a given type that arrived before now. */
  drainType(t) {
    const out = this._queue.filter((e) => e.msg?.t === t);
    this._queue = this._queue.filter((e) => e.msg?.t !== t);
    return out;
  }

  send(command) {
    if (this._sp.writable !== false) {
      this._sp.write(`${command}\n`);
    }
  }

  messages() { return this._allMessages; }

  close() {
    if (this._sp.isOpen) this._sp.close();
  }
}

// ── Output helpers ───────────────────────────────────────────────────────────

let _passed = 0;
let _failed = 0;
let _warned = 0;

function pass(label, detail = '') {
  _passed++;
  console.log(`  ${C.green}✓${C.reset} ${label}${detail ? `  ${C.dim}${detail}${C.reset}` : ''}`);
}
function fail(label, reason = '') {
  _failed++;
  console.log(`  ${C.red}✗${C.reset} ${label}${reason ? `  ${C.yellow}${reason}${C.reset}` : ''}`);
}
function warn(label, reason = '') {
  _warned++;
  console.log(`  ${C.yellow}⚠${C.reset} ${label}${reason ? `  ${C.dim}${reason}${C.reset}` : ''}`);
}
function info(msg) {
  console.log(`    ${C.dim}${msg}${C.reset}`);
}
function section(title) {
  console.log(`\n${C.bold}${title}${C.reset}`);
}
function observation(tag, msg) {
  console.log(`${C.dim}[${ts()}]${C.reset} ${C.cyan}[${tag}]${C.reset} ${msg}`);
}

// ── Diagnosis engine ─────────────────────────────────────────────────────────

function buildMatrix(messages, variantRequested) {
  const parsed     = messages.filter((m) => m.msg);
  const bootSeen   = parsed.some((m) => m.msg.t === 'boot') || messages.some((m) => m.raw.includes('"t":"boot"'));
  const pongSeen   = parsed.some((m) => m.msg.t === 'pong');
  const statuses   = parsed.filter((m) => m.msg.t === 'status');
  const latest     = statuses.length ? statuses[statuses.length - 1].msg : null;
  const frames     = parsed.filter((m) => m.msg.t === 'frame');
  const errors     = parsed.filter((m) => m.msg.t === 'error');
  const parseErrs  = messages.filter((m) => m.type === 'parse-error');

  const rawCanAck        = parsed.some((m) => m.msg.t === 'ack' && m.msg.cmd === 'can:raw:on');
  const rawCanInStatus   = statuses.some((m) => m.msg.rawCan !== undefined);
  const rawCanSupported  = rawCanAck || rawCanInStatus;
  const rawCanEnabled    = statuses.some((m) => Number(m.msg.rawCan) === 1);

  const filteredFrames   = frames.filter((m) => m._phase?.startsWith('filtered'));
  const rawFrames        = frames.filter((m) => m._phase?.startsWith('raw'));

  const variantOk = !variantRequested || (latest && latest.variant === variantRequested);

  const unknownCmds = errors.filter((m) => m.msg.msg === 'Unknown command').length;
  const noiseLikely = unknownCmds > 0 || parseErrs.length > 0;

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
    noiseLikely,
    filterMismatchLikely: rawCanSupported && rawCanEnabled && rawFrames.length > 0 && filteredFrames.length === 0,
    physicalCanIssueLikely: rawCanSupported && rawCanEnabled && rawFrames.length === 0,
    firmwareNeedsRawMode: !rawCanSupported,
  };
}

function diagnose(matrix) {
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

// ── Tag messages with phase (used by diagnosis) ──────────────────────────────

function tagPhase(session, phase) {
  session.messages().forEach((m) => { if (!m._phase) m._phase = phase; });
}

// ── SMOKE command ─────────────────────────────────────────────────────────────

async function runSmoke(session) {
  section('Boot / connection');

  // Wait for boot (board just powered up) or solicit status if already running
  let boot = await session.waitForType('boot', TIMEOUT_MS * 2);
  if (boot) {
    pass('Boot message received', `hw=${boot.msg.hw || '?'} variant=${boot.msg.variant || '?'}`);
    if (boot.msg.cap) info(`capabilities: ${boot.msg.cap}`);
  } else {
    session.send('status');
    const status = await session.waitForType('status', TIMEOUT_MS);
    if (status) {
      pass('Board already running — status responded', `variant=${status.msg.variant || '?'}`);
    } else {
      fail('Boot', 'No boot or status message received within timeout');
    }
  }

  // ── Ping / pong ────────────────────────────────────────────────────────────
  section('Ping / pong');
  session.send('ping');
  const pong = await session.waitForType('pong', TIMEOUT_MS);
  pong ? pass('Pong received') : fail('Pong', 'No pong within timeout');

  // ── Status ─────────────────────────────────────────────────────────────────
  section('Status message');
  session.send('status');
  const status = await session.waitForType('status', TIMEOUT_MS);
  if (status) {
    const s = status.msg;
    pass('Status received', `variant=${s.variant || '?'} ready=${s.ready || '?'}`);
    info(`fsd=${s.fsd ?? '?'}  sp=${s.sp ?? '?'}  up=${s.up ?? '?'}ms`);

    const required = ['t', 'hw', 'variant', 'ready'];
    const missing = required.filter((f) => s[f] === undefined);
    missing.length === 0
      ? pass('Status schema valid')
      : fail('Status schema', `missing fields: ${missing.join(', ')}`);
  } else {
    fail('Status', 'No status message received within timeout');
  }

  // ── Filtered stream ────────────────────────────────────────────────────────
  section('Filtered CAN stream');
  session.send('stream:on');
  const streamAck = await session.waitFor(
    (e) => e.msg?.t === 'ack' || (e.msg?.t === 'status' && e.msg?.stream?.on === 1),
    TIMEOUT_MS,
  );
  streamAck ? pass('stream:on acknowledged') : warn('stream:on', 'No ack — continuing');

  let frameCount = 0;
  const deadline = Date.now() + TIMEOUT_MS * 2;
  while (frameCount < 5 && Date.now() < deadline) {
    const frame = await session.waitForType('frame', Math.max(100, deadline - Date.now()));
    if (!frame) break;
    frameCount++;
  }
  tagPhase(session, 'filtered');

  if (frameCount >= 5) {
    pass(`${frameCount} filtered CAN frames received`);
  } else if (frameCount > 0) {
    warn(`Only ${frameCount} filtered frames`, 'few frames — is the board connected to the vehicle?');
  } else {
    warn('No filtered frames', 'expected when bench-testing without CAN bus');
  }

  session.send('stream:off');
  await delay(300);

  // ── Raw CAN stream ─────────────────────────────────────────────────────────
  section('Raw CAN stream');
  session.send('can:raw:on');
  const rawAck = await session.waitForAck('can:raw:on', TIMEOUT_MS);

  if (rawAck) {
    pass('can:raw:on acknowledged — raw mode supported');
    session.send('stream:on');
    await delay(TIMEOUT_MS);
    session.send('stream:off');

    const rawFrames = session.drainType('frame');
    tagPhase(session, 'raw');
    rawFrames.forEach((m) => { m._phase = 'raw'; });

    rawFrames.length > 0
      ? pass(`${rawFrames.length} raw CAN frames seen`)
      : warn('No raw frames', 'check physical CAN bus connection');

    session.send('can:raw:off');
    await delay(300);
  } else {
    info('can:raw:on not acknowledged — raw mode not available in this firmware build');
  }

  // ── Variant round-trip ─────────────────────────────────────────────────────
  section('Variant switching');
  session.send('status');
  const pre = await session.waitForType('status', TIMEOUT_MS);
  const originalVariant = pre?.msg?.variant || null;

  session.send('variant:hw4');
  await delay(300);
  session.send('status');
  const post = await session.waitForType('status', TIMEOUT_MS);
  post?.msg?.variant === 'hw4'
    ? pass('Variant switch to hw4 confirmed')
    : fail('Variant switch', `status shows variant=${post?.msg?.variant || '?'}`);

  if (originalVariant && originalVariant !== 'hw4') {
    session.send(`variant:${originalVariant}`);
    await delay(300);
    info(`Restored variant to ${originalVariant}`);
  }
}

// ── WATCH command ─────────────────────────────────────────────────────────────

async function runWatch(session) {
  let watchState = { fsd: null, profile: null, ready: null, rawCan: null, streamOn: null };

  if (WATCH_RAW) {
    session.send('can:raw:on');
    const ack = await session.waitForAck('can:raw:on', TIMEOUT_MS);
    ack ? info('raw CAN mode enabled') : warn('can:raw:on', 'not acknowledged');
  }

  session.send('stream:on');

  const filterSet = new Set(FILTER_IDS);
  const deadline = Date.now() + WATCH_DUR_MS;

  console.log(`\nWatching for ${WATCH_DUR_MS}ms${filterSet.size ? `  filter: ${[...filterSet].join(',')}` : '  (all frames)'}${WATCH_RAW ? '  [raw mode]' : ''}  Ctrl-C to stop\n`);

  while (Date.now() < deadline) {
    const remaining = Math.max(100, deadline - Date.now());
    const entry = await session.waitFor(() => true, remaining);
    if (!entry) break;

    const { msg, raw } = entry;
    if (!msg) {
      // raw text line
      console.log(`${C.dim}[${ts()}] text: ${raw}${C.reset}`);
      continue;
    }

    if (msg.t === 'frame') {
      const id = Number(msg.id);
      if (filterSet.size && !filterSet.has(id)) continue;
      const idHex  = `0x${id.toString(16).toUpperCase().padStart(3, '0')}`;
      const dir    = msg.dir === 'tx' ? `${C.yellow}TX${C.reset}` : `${C.green}RX${C.reset}`;
      const data   = String(msg.d || '').toUpperCase();
      console.log(`[${ts()}] ${dir} id=${C.cyan}${idHex}${C.reset} dlc=${msg.dlc} seq=${msg.seq ?? '-'} ms=${msg.ms ?? '-'}  ${data}`);
      continue;
    }

    if (msg.t === 'status') {
      const newFsd     = Number(msg.fsd);
      const newProfile = Number(msg.sp);
      const newReady   = msg.ready || null;
      const newRawCan  = Number(msg.rawCan);
      const streamOn   = msg.stream?.on;

      if (!watchState._seen) {
        watchState._seen = true;
        watchState.fsd      = newFsd;
        watchState.profile  = newProfile;
        watchState.ready    = newReady;
        watchState.rawCan   = newRawCan;
        watchState.streamOn = streamOn;
        if (WATCH_FSd || WATCH_PROFILE) {
          observation('status', `initial — ready=${newReady} fsd=${newFsd} sp=${newProfile} rawCan=${newRawCan} stream.on=${streamOn}`);
        }
        continue;
      }

      if (WATCH_FSd && watchState.fsd !== newFsd) {
        observation('fsd', `${watchState.fsd} → ${newFsd}`);
        watchState.fsd = newFsd;
      }
      if (WATCH_PROFILE && watchState.profile !== newProfile) {
        observation('profile', `${watchState.profile} → ${newProfile}`);
        watchState.profile = newProfile;
      }
      if (WATCH_FSd || WATCH_PROFILE) {
        if (watchState.ready !== newReady) {
          observation('ready', `${watchState.ready} → ${newReady}`);
          watchState.ready = newReady;
        }
        if (watchState.streamOn !== streamOn) {
          observation('stream', `on ${watchState.streamOn} → ${streamOn}`);
          watchState.streamOn = streamOn;
        }
      }
      continue;
    }

    if (msg.t === 'ack') {
      observation('ack', msg.cmd || '?');
      continue;
    }

    if (msg.t === 'error') {
      observation('error', msg.msg || '?');
      continue;
    }

    if (msg.t === 'log') {
      observation('log', msg.msg || raw);
    }
  }

  session.send('stream:off');
  if (WATCH_RAW) {
    session.send('can:raw:off');
    await delay(200);
  }

  console.log(`\nWatch ended.`);
}

// ── TEST command ──────────────────────────────────────────────────────────────

async function runTest(session) {
  // Capture baseline
  session.send('status');
  const baseline = await session.waitForType('status', TIMEOUT_MS);
  const origFsd     = baseline ? Number(baseline.msg.fsd) : null;
  const origProfile = baseline ? Number(baseline.msg.sp)  : null;

  if (baseline) {
    info(`Baseline — fsd=${origFsd} sp=${origProfile} variant=${baseline.msg.variant}`);
  } else {
    warn('No baseline status received — test results may be unreliable');
  }

  // ── FSD test ───────────────────────────────────────────────────────────────
  if (TEST_FSd) {
    section('FSD round-trip test');

    session.send('fsd:on');
    const onAck = await session.waitForAck('fsd:on', TIMEOUT_MS);
    onAck ? pass('fsd:on acknowledged') : fail('fsd:on', 'no ack');

    await delay(600);
    session.send('status');
    const afterOn = await session.waitForType('status', TIMEOUT_MS);
    if (afterOn) {
      Number(afterOn.msg.fsd) === 1
        ? pass('FSD enabled in status', `fsd=${afterOn.msg.fsd}`)
        : fail('FSD on verification', `fsd=${afterOn.msg.fsd} (expected 1)`);
    } else {
      fail('FSD on verification', 'no status after fsd:on');
    }

    session.send('fsd:off');
    const offAck = await session.waitForAck('fsd:off', TIMEOUT_MS);
    offAck ? pass('fsd:off acknowledged') : fail('fsd:off', 'no ack');

    await delay(600);
    session.send('status');
    const afterOff = await session.waitForType('status', TIMEOUT_MS);
    if (afterOff) {
      Number(afterOff.msg.fsd) === 0
        ? pass('FSD disabled in status', `fsd=${afterOff.msg.fsd}`)
        : fail('FSD off verification', `fsd=${afterOff.msg.fsd} (expected 0)`);
    } else {
      fail('FSD off verification', 'no status after fsd:off');
    }

    if (RESTORE && origFsd !== null) {
      const restoreCmd = origFsd === 1 ? 'fsd:on' : 'fsd:off';
      session.send(restoreCmd);
      await delay(400);
      info(`Restored fsd to ${origFsd} via ${restoreCmd}`);
    }
  }

  // ── Profile test ───────────────────────────────────────────────────────────
  if (TEST_PROFILE >= 0) {
    section(`Profile round-trip test (target=${TEST_PROFILE})`);

    const cmd = `profile:${TEST_PROFILE}`;
    session.send(cmd);
    const ack = await session.waitForAck(cmd, TIMEOUT_MS);
    ack ? pass(`${cmd} acknowledged`) : fail(cmd, 'no ack');

    await delay(700);
    session.send('status');
    const afterSet = await session.waitForType('status', TIMEOUT_MS);
    if (afterSet) {
      Number(afterSet.msg.sp) === TEST_PROFILE
        ? pass(`Profile ${TEST_PROFILE} confirmed in status`, `sp=${afterSet.msg.sp}`)
        : fail(`Profile ${TEST_PROFILE} verification`, `sp=${afterSet.msg.sp} (expected ${TEST_PROFILE})`);
    } else {
      fail('Profile verification', 'no status after profile command');
    }

    if (RESTORE && origProfile !== null) {
      session.send(`profile:${origProfile}`);
      await delay(400);
      info(`Restored profile to ${origProfile}`);
    }
  }

  if (!TEST_FSd && TEST_PROFILE < 0) {
    info('No test specified. Use --test-fsd and/or --test-profile <n>');
  }
}

// ── Entry point ──────────────────────────────────────────────────────────────

async function main() {
  console.log(`${C.bold}TeslaCANModder Board Debug${C.reset}  ${C.dim}command=${COMMAND} port=${PORT} baud=${BAUD}${C.reset}`);

  if (!['smoke', 'watch', 'test'].includes(COMMAND)) {
    console.error(`Unknown command "${COMMAND}". Use: smoke | watch | test`);
    process.exit(1);
  }

  let sp;
  try {
    sp = await openSerial(PORT, BAUD);
    info(`Opened ${PORT} @ ${BAUD} baud`);
  } catch (err) {
    console.error(`${C.red}FATAL${C.reset}: ${err.message}`);
    process.exit(1);
  }

  const session = new BoardSession(sp);

  info(`Waiting ${WARMUP_MS}ms for board to settle...`);
  await delay(WARMUP_MS);

  // Set requested variant first if supplied
  if (VARIANT) {
    session.send(`variant:${VARIANT}`);
    await delay(400);
    info(`variant:${VARIANT} sent`);
  }

  try {
    if (COMMAND === 'smoke') await runSmoke(session);
    if (COMMAND === 'watch') await runWatch(session);
    if (COMMAND === 'test')  await runTest(session);
  } finally {
    session.close();
  }

  // ── Summary ────────────────────────────────────────────────────────────────
  const matrix = buildMatrix(session.messages(), VARIANT);
  const dx     = diagnose(matrix);

  if (COMMAND !== 'watch') {
    console.log(`\n${'─'.repeat(50)}`);
    console.log(`${C.green}Passed${C.reset}: ${_passed}   ${_failed > 0 ? C.red : C.green}Failed${C.reset}: ${_failed}   ${_warned > 0 ? C.yellow : ''}Warned${C.reset}: ${_warned}`);
    console.log(`Diagnosis: ${dx}`);
    console.log('─'.repeat(50));
  }

  if (JSON_OUTPUT) {
    const allMsgs = session.messages();
    const report = {
      command: COMMAND,
      port: PORT,
      baud: BAUD,
      variant: VARIANT,
      passed: _passed,
      failed: _failed,
      warned: _warned,
      diagnosis: dx,
      matrix,
      latestStatus: matrix.latestStatus,
      sampleFrameIds: allMsgs
        .filter((m) => m.msg?.t === 'frame')
        .slice(0, 10)
        .map((m) => m.msg.id),
      messages: allMsgs.map(({ raw, type, msg, _phase }) => ({ raw, type, phase: _phase || null, msg: msg || null })),
    };
    console.log('\n' + JSON.stringify(report, null, 2));
  }

  process.exit(_failed > 0 ? 1 : 0);
}

main().catch((err) => {
  console.error(`${C.red}Unexpected error:${C.reset} ${err.message}`);
  process.exit(1);
});
