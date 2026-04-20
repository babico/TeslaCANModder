/** Argument parser — extracts CLI flags into a typed options object. */

export function parseArgs(argv) {
  const result = { _: [] };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === '-c') {
      result.erase = true;
    } else if (arg.startsWith('--')) {
      const key = arg.slice(2);
      const next = argv[i + 1];
      if (next !== undefined && !next.startsWith('--')) {
        result[key] = next;
        i++;
      } else {
        result[key] = true;
      }
    } else if (arg.startsWith('-') && arg.length > 1) {
      const key = arg.slice(1);
      const next = argv[i + 1];
      if (next !== undefined && !next.startsWith('-')) {
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

export function resolveOptions(args) {
  return {
    command:      args._[0] || 'smoke',
    port:         args.port ? String(args.port) : null,
    baud:         Number(args.baud) || 115200,
    timeoutMs:    Number(args.timeout) || 3000,
    warmupMs:     Number(args.warmup) || 2000,
    variant:      args.variant ? String(args.variant) : null,
    jsonOutput:   Boolean(args.json),
    noColor:      Boolean(args['no-color']),
    // watch
    watchDurMs:   Number(args.duration) || 10000,
    watchRaw:     Boolean(args.raw),
    watchFsd:     Boolean(args['watch-fsd']),
    watchProfile: Boolean(args['watch-profile']),
    bitDiff:      Boolean(args['bit-diff']),
    filterIds:    (args.filter ? String(args.filter) : '')
      .split(',').map(s => Number(s.trim())).filter(n => Number.isFinite(n) && n > 0),
    // test
    testFsd:      Boolean(args['test-fsd']),
    testProfile:  args['test-profile'] !== undefined ? Number(args['test-profile']) : -1,
    testFsdListen: Boolean(args['test-fsd-listen']),
    restore:      Boolean(args.restore),
    // flash
    flashHex:     args.hex ? String(args.hex) : null,
    eraseChip:    Boolean(args.erase),
    // scan
    scanDurMs:    Number(args['scan-duration']) || 5000,
    // dump
    dumpFile:     args.output ? String(args.output) : null,
    dumpFormat:   args.format ? String(args.format) : 'jsonl',
    // replay
    replayFile:   args.input ? String(args.input) : null,
    replaySpeed:  Number(args.speed) || 1,
    // benchmark
    benchDurMs:   Number(args['bench-duration']) || 10000,
    // vehicle
    vehicleCmd:   args['vehicle-cmd'] ? String(args['vehicle-cmd']) : null,
    // drive-context
    driveCtxDurMs: Number(args['drive-duration']) || Number(args.duration) || 30000,
    driveCtxOutput: args['drive-output'] ? String(args['drive-output']) : null,
    driveCtxExpectFull: Boolean(args['drive-expect-full']),
    driveCtxMinSamples: Number(args['drive-min-samples']) || 1,
    driveCtxNoteOutput: args['drive-note-output'] ? String(args['drive-note-output']) : null,
  };
}
