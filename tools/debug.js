#!/usr/bin/env node
/**
 * TeslaCANModder — Modular Board Debug Tool
 *
 * Usage: node tools/debug.js <command> [options]
 */

import { setTimeout as delay } from "node:timers/promises";
import { parseArgs, resolveOptions } from "./lib/args.js";
import { createColors, createOutput } from "./lib/output.js";
import { openSerial, BoardSession } from "./lib/session.js";
import { buildMatrix, diagnose } from "./lib/diagnosis.js";
import { runSmoke } from "./commands/smoke.js";
import { runWatch } from "./commands/watch.js";
import { runTest } from "./commands/test.js";
import { runFlash } from "./commands/flash.js";
import { runScan } from "./commands/scan.js";
import { runDump } from "./commands/dump.js";
import { runReplay } from "./commands/replay.js";
import { runBenchmark } from "./commands/benchmark.js";
import { runVehicle } from "./commands/vehicle.js";
import { runExport } from "./commands/export.js";
import { runDriveContext } from "./commands/drive-context.js";

const SESSION_COMMAND_HANDLERS = {
	smoke: runSmoke,
	watch: runWatch,
	test: runTest,
	scan: runScan,
	dump: runDump,
	replay: runReplay,
	benchmark: runBenchmark,
	vehicle: runVehicle,
	"drive-context": runDriveContext,
};

const PORTLESS_COMMANDS = new Set(["export"]);
const COMMANDS = [...Object.keys(SESSION_COMMAND_HANDLERS), "flash", ...PORTLESS_COMMANDS];

function printUsage() {
	console.error("Usage: node tools/debug.js <command> --port COM3 [options]");
	console.error(`Commands: ${COMMANDS.join(" | ")}`);
	console.error("Note: export does not require --port");
}

function buildExportOptions(args) {
	return {
		input: args.input ?? args.i ?? null,
		format: args.format ?? args.f ?? "json",
		output: args.output ?? args.o ?? null,
		overwrite: Boolean(args.overwrite),
	};
}

async function run() {
	const args = parseArgs(process.argv.slice(2));
	const opts = resolveOptions(args);
	const C = createColors(opts.noColor);
	const out = createOutput(C);

	if (!COMMANDS.includes(opts.command)) {
		console.error(`Unknown command "${opts.command}".`);
		printUsage();
		return 1;
	}

	if (opts.command === "export") {
		return await runExport(buildExportOptions(args), out);
	}

	if (!opts.port) {
		console.error("ERROR: --port is required for this command");
		printUsage();
		return 1;
	}

	if (opts.command === "flash") {
		await runFlash(opts, out, C);
		return 0;
	}

	return await runSessionCommand(opts, out, C);
}

async function runSessionCommand(opts, out, C) {
	console.log(
		`${C.bold}TeslaCANModder Board Debug${C.reset}  ${C.dim}` +
			`command=${opts.command} port=${opts.port} baud=${opts.baud}${C.reset}`,
	);

	let sp;
	try {
		sp = await openSerial(opts.port, opts.baud);
		out.info(`Opened ${opts.port} @ ${opts.baud} baud`);
	} catch (err) {
		console.error(`${C.red}FATAL${C.reset}: ${err.message}`);
		return 1;
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
		const handler = SESSION_COMMAND_HANDLERS[opts.command];
		await handler(session, opts, out, C);
	} finally {
		session.close();
	}

	const matrix = buildMatrix(session.messages(), opts.variant);
	const dx = diagnose(matrix);
	const { passed, failed, warned } = out.counts();

	if (opts.command !== "watch") {
		console.log(`\n${"-".repeat(50)}`);
		console.log(
			`${C.green}Passed${C.reset}: ${passed}   ` +
				`${failed > 0 ? C.red : C.green}Failed${C.reset}: ${failed}   ` +
				`${warned > 0 ? C.yellow : ""}Warned${C.reset}: ${warned}`,
		);
		console.log(`Diagnosis: ${dx}`);
		console.log("-".repeat(50));
	}

	if (opts.jsonOutput) {
		const report = {
			command: opts.command,
			port: opts.port,
			baud: opts.baud,
			variant: opts.variant,
			passed,
			failed,
			warned,
			diagnosis: dx,
			matrix,
			latestStatus: matrix.latestStatus,
			sampleFrameIds: session
				.messages()
				.filter((m) => m.msg?.t === "frame")
				.slice(0, 10)
				.map((m) => m.msg.id),
		};
		console.log(`\n${JSON.stringify(report, null, 2)}`);
	}

	return failed > 0 ? 1 : 0;
}

run()
	.then((code) => process.exit(code ?? 0))
	.catch((err) => {
		console.error(err?.stack || err?.message || String(err));
		process.exit(1);
	});
