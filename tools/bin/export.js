#!/usr/bin/env node
/**
 * Standalone entry point for the decoder-dataset export command.
 *
 * Usage: node tools/bin/export.js --input mcu2.json --format dbc|csv|json
 */

import { parseArgs } from "../lib/args.js";
import { runExport } from "../commands/export.js";

const args = parseArgs(process.argv.slice(2));

// Minimal output shim — diagnostics go to stderr so stdout stays pipeable.
const out = {
	section: (t) => console.error(`\n${t}`),
	info: (m) => console.error(`  ${m}`),
	pass: (m) => console.error(`  ✓ ${m}`),
	fail: (m) => {
		console.error(`  ✗ ${m}`);
	},
};

const opts = {
	input: args.input ?? args.i ?? null,
	format: args.format ?? args.f ?? "json",
	output: args.output ?? args.o ?? null,
	overwrite: Boolean(args.overwrite),
};

runExport(opts, out).then((code) => process.exit(code ?? 0));
