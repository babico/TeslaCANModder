#!/usr/bin/env node
/**
 * TeslaCANModder — Modular Board Debug Tool
 *
 * Usage:  node tools/debug.js <command> [options]
 *
 * Commands:
 *   smoke      Protocol health-check (default)
 *   watch      Live frame / state monitor
 *   test       FSD / profile round-trip test
 *   flash      Flash firmware to board
 *   scan       Discover CAN IDs on the bus
 *   dump       Record CAN frames to file
 *   replay     Replay recorded CAN frames
 *   benchmark  Measure CAN throughput
 *   vehicle    Send vehicle control commands
 *
 * Run with --help for full option list per command.
 */

import { setTimeout as delay } from 'node:timers/promises';
import { parseArgs, resolveOptions } from './lib/args.js';
import { createColors, createOutput } from './lib/output.js';
import { openSerial, BoardSession } from './lib/session.js';
import { buildMatrix, diagnose } from './lib/diagnosis.js';
import { runSmoke } from './commands/smoke.js';
import { runWatch } from './commands/watch.js';
import { runTest } from './commands/test.js';
import { runFlash } from './commands/flash.js';
import { runScan } from './commands/scan.js';
import { runDump } from './commands/dump.js';
import { runReplay } from './commands/replay.js';
import { runBenchmark } from './commands/benchmark.js';
import { runVehicle } from './commands/vehicle.js';

const args = parseArgs(process.argv.slice(2));
const opts = resolveOptions(args);

if (!opts.port) {
  console.error('ERROR: --port is required');
  console.error('Usage: node tools/debug.js <command> --port COM3 [options]');
  process.exit(1);
}

const COMMANDS = ['smoke', 'watch', 'test', 'flash', 'scan', 'dump', 'replay', 'benchmark', 'vehicle'];
const C = createColors(opts.noColor);
const out = createOutput(C);

async function main() {
  console.log(`${C.bold}TeslaCANModder Board Debug${C.reset}  ${C.dim}command=${opts.command} port=${opts.port} baud=${opts.baud}${C.reset}`);

  if (!COMMANDS.includes(opts.command)) {
    console.error(`Unknown command "${opts.command}". Use: ${COMMANDS.join(' | ')}`);
    process.exit(1);
  }

  if (opts.command === 'flash') {
    await runFlash(opts, out, C);
    process.exit(0);
  }

  let sp;
  try {
    sp = await openSerial(opts.port, opts.baud);
    out.info(`Opened ${opts.port} @ ${opts.baud} baud`);
  } catch (err) {
    console.error(`${C.red}FATAL${C.reset}: ${err.message}`);
    process.exit(1);
  }

  const session = new BoardSession(sp, opts.timeoutMs);
  out.info(`Waiting ${opts.warmupMs}ms for board to settle...`);
  await delay(opts.warmupMs);

  if (opts.variant) {
    session.send(`variant:${opts.variant}`);
    await delay(400);
    out.info(`variant:${opts.variant} sent`);
  }

  try {
    switch (opts.command) {
      case 'smoke':     await runSmoke(session, opts, out); break;
      case 'watch':     await runWatch(session, opts, out, C); break;
      case 'test':      await runTest(session, opts, out); break;
      case 'scan':      await runScan(session, opts, out, C); break;
      case 'dump':      await runDump(session, opts, out); break;
      case 'replay':    await runReplay(session, opts, out, C); break;
      case 'benchmark': await runBenchmark(session, opts, out); break;
      case 'vehicle':   await runVehicle(session, opts, out); break;
    }
  } finally {
    session.close();
  }

  const matrix = buildMatrix(session.messages(), opts.variant);
  const dx = diagnose(matrix);
  const { passed, failed, warned } = out.counts();

  if (opts.command !== 'watch') {
    console.log(`\n${'─'.repeat(50)}`);
    console.log(`${C.green}Passed${C.reset}: ${passed}   ${failed > 0 ? C.red : C.green}Failed${C.reset}: ${failed}   ${warned > 0 ? C.yellow : ''}Warned${C.reset}: ${warned}`);
    console.log(`Diagnosis: ${dx}`);
    console.log('─'.repeat(50));
  }

  if (opts.jsonOutput) {
    const report = {
      command: opts.command, port: opts.port, baud: opts.baud, variant: opts.variant,
      passed, failed, warned, diagnosis: dx, matrix, latestStatus: matrix.latestStatus,
      sampleFrameIds: session.messages().filter(m => m.msg?.t === 'frame').slice(0, 10).map(m => m.msg.id),
    };
    console.log('\n' + JSON.stringify(report, null, 2));
  }

  process.exit(failed > 0 ? 1 : 0);
}

main();
