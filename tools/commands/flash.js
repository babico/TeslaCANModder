/** flash command — Flash firmware to board via avrdude. */

import { setTimeout as delay } from 'node:timers/promises';
import { openSerial, BoardSession } from '../lib/session.js';

export async function runFlash(opts, out, C) {
  const { execFile } = await import('node:child_process');
  const { promisify } = await import('node:util');
  const execFileAsync = promisify(execFile);
  const path = await import('node:path');
  const fs = await import('node:fs');

  if (!opts.flashHex) {
    console.error(`${C.red}ERROR${C.reset}: --hex <path> is required for flash command`);
    process.exit(1);
  }

  if (!fs.existsSync(opts.flashHex)) {
    console.error(`${C.red}ERROR${C.reset}: Hex file not found: ${opts.flashHex}`);
    process.exit(1);
  }

  out.section('Flash firmware to board');
  out.info(`Hex file: ${opts.flashHex}`);
  out.info(`Port: ${opts.port}`);
  if (opts.eraseChip) out.info('Chip erase: enabled — EEPROM will be wiped (factory reset)');

  const avrdudeSearchPaths = [
    path.join(process.cwd(), 'hardware', '.pio-home', 'packages', 'tool-avrdude', 'avrdude.exe'),
    path.join(process.env.USERPROFILE || '', '.platformio', 'packages', 'tool-avrdude', 'avrdude.exe'),
    'avrdude',
  ];

  let avrdudePath = null, avrdudeConf = null;
  for (const p of avrdudeSearchPaths) {
    if (fs.existsSync(p)) {
      avrdudePath = p;
      const confPath = path.join(path.dirname(p), 'avrdude.conf');
      if (fs.existsSync(confPath)) avrdudeConf = confPath;
      break;
    }
  }

  if (!avrdudePath) {
    console.error(`${C.red}ERROR${C.reset}: avrdude not found. Install PlatformIO or add avrdude to PATH`);
    process.exit(1);
  }

  out.info(`Using avrdude: ${avrdudePath}`);

  const args = ['-v', '-p', 'atmega328p', '-c', 'arduino', '-P', opts.port, '-b', '115200'];
  if (avrdudeConf) args.unshift('-C', avrdudeConf);
  args.push(opts.eraseChip ? '-e' : '-D');
  args.push('-U', `flash:w:${opts.flashHex}:i`);

  try {
    out.info('Flashing...');
    const { stdout, stderr } = await execFileAsync(avrdudePath, args);
    if ((stderr + stdout).includes('bytes of flash verified')) out.pass('Firmware flashed successfully');
    else out.warn('Flash completed but verification unclear');
    if (stderr) console.log(`\n${C.dim}${stderr}${C.reset}`);
  } catch (err) {
    out.fail('Flash failed', err.message);
    if (err.stderr) console.log(`\n${C.red}${err.stderr}${C.reset}`);
    process.exit(1);
  }

  // Verify board boots by reading serial output
  out.info('Verifying board boot...');
  let sp;
  try {
    await delay(1500);
    sp = await openSerial(opts.port, opts.baud);
    const session = new BoardSession(sp, opts.timeoutMs);
    await delay(2500);

    const logs = session.drainType('log');
    const status = session.drainType('status');

    if (logs.length > 0 || status.length > 0) {
      out.pass('Board is responding');
      for (const e of logs) {
        out.info(`Board: ${e.msg?.msg || JSON.stringify(e.msg)}`);
      }
      if (status.length > 0) {
        const s = status[status.length - 1].msg;
        out.info(`Variant: ${s?.variant || '?'}  FSD: ${s?.fsd ?? '?'}  Nag: ${s?.nag ?? '?'}  Profile: ${s?.profile ?? '?'}`);
      }
    } else {
      out.warn('No serial output received — check board manually');
    }

    session.close();
  } catch (err) {
    out.warn(`Could not verify: ${err.message}`);
    if (sp && sp.isOpen) sp.close();
  }
}
