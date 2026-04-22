/** flash command — flash merged or PlatformIO build firmware to ESP32 via esptool. */

import { execFile } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { setTimeout as delay } from "node:timers/promises";
import { promisify } from "node:util";

import { openSerial, BoardSession } from "../lib/session.js";

const execFileAsync = promisify(execFile);
const FLASH_BAUD = "460800";

function firstExisting(paths) {
	return paths.find((candidate) => candidate && fs.existsSync(candidate)) ?? null;
}

function getPlatformIoHomes() {
	const candidates = [
		process.env.PLATFORMIO_CORE_DIR,
		path.join(os.homedir(), ".platformio"),
	].filter(Boolean);
	return [...new Set(candidates)];
}

function resolvePlatformIoTooling() {
	const homes = getPlatformIoHomes();
	const scriptsDir = process.platform === "win32" ? "Scripts" : "bin";
	const pythonName = process.platform === "win32" ? "python.exe" : "python";

	return {
		pythonExe: firstExisting(
			homes.map((home) => path.join(home, "penv", scriptsDir, pythonName)),
		),
		esptoolPy: firstExisting(
			homes.map((home) => path.join(home, "packages", "tool-esptoolpy", "esptool.py")),
		),
		bootApp0: firstExisting(
			homes.map((home) =>
				path.join(
					home,
					"packages",
					"framework-arduinoespressif32",
					"tools",
					"partitions",
					"boot_app0.bin",
				),
			),
		),
	};
}

export function resolveFlashImagePlan(imagePath, bootApp0Path) {
	const resolvedImagePath = path.resolve(imagePath);
	const imageName = path.basename(resolvedImagePath).toLowerCase();

	if (imageName !== "firmware.bin") {
		return {
			kind: "merged-image",
			label: "Merged flash image",
			segments: [{ address: "0x0", file: resolvedImagePath }],
		};
	}

	const buildDir = path.dirname(resolvedImagePath);
	const bootloader = path.join(buildDir, "bootloader.bin");
	const partitions = path.join(buildDir, "partitions.bin");

	if (
		!fs.existsSync(bootloader) ||
		!fs.existsSync(partitions) ||
		!bootApp0Path ||
		!fs.existsSync(bootApp0Path)
	) {
		throw new Error(
			"Detected raw PlatformIO firmware.bin without the full ESP32 boot image layout. Use build/firmware/<env>.bin or keep bootloader.bin and partitions.bin beside firmware.bin.",
		);
	}

	return {
		kind: "platformio-build",
		label: "PlatformIO build output",
		segments: [
			{ address: "0x1000", file: bootloader },
			{ address: "0x8000", file: partitions },
			{ address: "0xe000", file: bootApp0Path },
			{ address: "0x10000", file: resolvedImagePath },
		],
	};
}

function flattenSegments(segments) {
	return segments.flatMap((segment) => [segment.address, segment.file]);
}

function buildEsptoolInvocation(tooling) {
	if (tooling.pythonExe && tooling.esptoolPy) {
		return {
			command: tooling.pythonExe,
			prefixArgs: [tooling.esptoolPy],
			display: `${tooling.pythonExe} ${tooling.esptoolPy}`,
		};
	}

	return {
		command: tooling.pythonExe ?? "python",
		prefixArgs: ["-m", "esptool"],
		display: tooling.pythonExe ? `${tooling.pythonExe} -m esptool` : "python -m esptool",
	};
}

async function runEsptool(invocation, port, args) {
	return execFileAsync(
		invocation.command,
		[
			...invocation.prefixArgs,
			"--chip",
			"esp32",
			"--port",
			port,
			"--baud",
			FLASH_BAUD,
			"--before",
			"default_reset",
			...args,
		],
		{ maxBuffer: 10 * 1024 * 1024 },
	);
}

export async function runFlash(opts, out, C) {
	if (!opts.flashHex) {
		console.error(`${C.red}ERROR${C.reset}: --hex <path> is required for flash command`);
		process.exit(1);
	}

	if (!fs.existsSync(opts.flashHex)) {
		console.error(`${C.red}ERROR${C.reset}: Firmware image not found: ${opts.flashHex}`);
		process.exit(1);
	}

	const tooling = resolvePlatformIoTooling();
	const invocation = buildEsptoolInvocation(tooling);

	let flashPlan;
	try {
		flashPlan = resolveFlashImagePlan(opts.flashHex, tooling.bootApp0);
	} catch (error) {
		console.error(
			`${C.red}ERROR${C.reset}: ${error instanceof Error ? error.message : "Invalid firmware image"}`,
		);
		process.exit(1);
	}

	out.section("Flash firmware to board");
	out.info(`Firmware image: ${path.resolve(opts.flashHex)}`);
	out.info(`Flash layout: ${flashPlan.label}`);
	out.info(`Port: ${opts.port}`);
	out.info(`Flash baud: ${FLASH_BAUD}`);
	if (opts.eraseChip) out.info("Chip erase: enabled - flash will be fully erased before write");
	out.info(`Using esptool: ${invocation.display}`);

	try {
		if (opts.eraseChip) {
			out.info("Erasing flash...");
			await runEsptool(invocation, opts.port, ["--after", "no_reset", "erase_flash"]);
		}

		out.info("Writing firmware...");
		const { stdout, stderr } = await runEsptool(invocation, opts.port, [
			"--after",
			"hard_reset",
			"write_flash",
			"-z",
			"--flash_mode",
			"keep",
			"--flash_freq",
			"keep",
			"--flash_size",
			"keep",
			...flattenSegments(flashPlan.segments),
		]);

		out.pass("Firmware flashed successfully");
		if (stderr?.trim()) process.stderr.write(`\n${C.dim}${stderr}${C.reset}\n`);
		if (stdout?.trim()) process.stdout.write(`\n${C.dim}${stdout}${C.reset}\n`);
	} catch (error) {
		out.fail("Flash failed", error.message);
		if (error.stderr) process.stderr.write(`\n${C.red}${error.stderr}${C.reset}\n`);
		process.exit(1);
	}

	out.info("Verifying board boot...");
	let sp;
	try {
		await delay(1500);
		sp = await openSerial(opts.port, opts.baud);
		const session = new BoardSession(sp, opts.timeoutMs);
		await delay(2500);

		const logs = session.drainType("log");
		const status = session.drainType("status");

		if (logs.length > 0 || status.length > 0) {
			out.pass("Board is responding");
			for (const entry of logs) {
				out.info(`Board: ${entry.msg?.msg || JSON.stringify(entry.msg)}`);
			}
			if (status.length > 0) {
				const latestStatus = status[status.length - 1].msg;
				out.info(
					`Variant: ${latestStatus?.variant || "?"}  FSD: ${latestStatus?.fsd ?? "?"}  Nag: ${latestStatus?.nag ?? "?"}  Profile: ${latestStatus?.profile ?? "?"}`,
				);
			}
		} else {
			out.warn("No serial output received - check board manually");
		}

		session.close();
	} catch (error) {
		out.warn(`Could not verify: ${error.message}`);
		if (sp && sp.isOpen) sp.close();
	}
}
