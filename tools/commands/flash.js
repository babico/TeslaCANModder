/** flash command — Flash firmware to board via avrdude. */

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
  if (opts.eraseChip) out.info('Chip erase: enabled');

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
}
