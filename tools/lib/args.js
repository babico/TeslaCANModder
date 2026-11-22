/** Argument parser — extracts CLI flags into a typed options object. */

export function parseArgs(argv) {
	const result = { _: [] };
	for (let i = 0; i < argv.length; i++) {
		const arg = argv[i];
		if (arg === "-c") {
			result.erase = true;
		} else if (arg.startsWith("--")) {
			const key = arg.slice(2);
			const next = argv[i + 1];
			if (next !== undefined && !next.startsWith("--")) {
				result[key] = next;
				i++;
			} else {
				result[key] = true;
			}
		} else if (arg.startsWith("-") && arg.length > 1) {
			const key = arg.slice(1);
			const next = argv[i + 1];
			if (next !== undefined && !next.startsWith("-")) {
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

/**
 * Validate a numeric flag instead of hiding bad input behind `Number(x) || default`.
 * Booleans, blanks, non-integers and out-of-range values are rejected.
 */
function numericFlag(args, key, fallback, { min = 1, max = Number.MAX_SAFE_INTEGER } = {}) {
	const value = args[key];
	if (value === undefined) return fallback;
	const num = typeof value === "boolean" || value === "" ? NaN : Number(value);
	if (!Number.isSafeInteger(num) || num < min || num > max) {
		throw new Error(
			`Invalid --${key}: expected integer ${min}..${max}, got ${JSON.stringify(value)}`,
		);
	}
	return num;
}

export function resolveOptions(args) {
	// `duration` is a shared flag: --watch duration, --das-drive duration and the
	// fallback for --drive-duration. Precedence for drive-context:
	// --drive-duration > --duration > 30000.
	const durationMs = args.duration === undefined ? null : numericFlag(args, "duration", null);

	return {
		command: args._[0] || "smoke",
		port: args.port ? String(args.port) : null,
		baud: numericFlag(args, "baud", 115200),
		timeoutMs: numericFlag(args, "timeout", 3000),
		warmupMs: numericFlag(args, "warmup", 2000),
		variant: args.variant ? String(args.variant) : null,
		jsonOutput: Boolean(args.json),
		noColor: Boolean(args["no-color"]),
		// watch
		watchDurMs: durationMs ?? 10000,
		watchRaw: Boolean(args.raw),
		watchFsd: Boolean(args["watch-fsd"]),
		watchProfile: Boolean(args["watch-profile"]),
		bitDiff: Boolean(args["bit-diff"]),
		filterIds: (args.filter ? String(args.filter) : "")
			.split(",")
			.map((s) => Number(s.trim()))
			.filter((n) => Number.isFinite(n) && n > 0),
		// test
		testFsd: Boolean(args["test-fsd"]),
		testProfile: numericFlag(args, "test-profile", -1, { min: 0, max: 3 }),
		testFsdListen: Boolean(args["test-fsd-listen"]),
		restore: Boolean(args.restore),
		// flash
		flashHex: args.hex ? String(args.hex) : null,
		flashBaud: numericFlag(args, "flash-baud", 460800, { min: 9600, max: 3000000 }),
		eraseChip: Boolean(args.erase),
		// scan
		scanDurMs: numericFlag(args, "scan-duration", 5000),
		// dump
		dumpFile: args.output ? String(args.output) : null,
		dumpFormat: args.format ? String(args.format) : "jsonl",
		// replay
		replayFile: args.input ? String(args.input) : null,
		replaySpeed: numericFlag(args, "speed", 1),
		// benchmark
		benchDurMs: numericFlag(args, "bench-duration", 10000),
		// vehicle
		vehicleCmd: args["vehicle-cmd"] ? String(args["vehicle-cmd"]) : null,
		// drive-context
		driveCtxDurMs: numericFlag(args, "drive-duration", durationMs ?? 30000),
		driveCtxOutput: args["drive-output"] ? String(args["drive-output"]) : null,
		driveCtxExpectFull: Boolean(args["drive-expect-full"]),
		driveCtxMinSamples: numericFlag(args, "drive-min-samples", 1),
		driveCtxNoteOutput: args["drive-note-output"] ? String(args["drive-note-output"]) : null,
		// das-drive (gamepad CAN injection diagnostics)
		action: args.action ? String(args.action) : null,
		kph: args.kph === undefined ? null : numericFlag(args, "kph", null, { min: 0, max: 1000 }),
		duration: durationMs,
	};
}
