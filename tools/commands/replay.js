/** replay command — Replay recorded CAN frames from a JSONL file. */

import { createReadStream } from "node:fs";
import { resolve } from "node:path";
import { createInterface } from "node:readline";
import { setTimeout as delay } from "node:timers/promises";

export async function runReplay(session, opts, out, _C) {
	const { replayFile, replaySpeed } = opts;

	if (!replayFile) {
		console.error("ERROR: --input <path> is required for replay command");
		process.exit(1);
	}

	if (!isSafePathInput(replayFile)) {
		out.fail("Invalid --input path");
		return;
	}

	const inputPath = resolve(replayFile);

	out.section("CAN Frame Replay");
	out.info(`File: ${inputPath}`);
	out.info(`Speed: ${replaySpeed}x`);

	const lines = [];
	const rl = createInterface({ input: createReadStream(inputPath), crlfDelay: Infinity });
	for await (const line of rl) {
		const trimmed = line.trim();
		if (!trimmed || trimmed.startsWith("timestamp,")) continue; // skip CSV header
		try {
			const obj = JSON.parse(trimmed);
			if (isReplayFrameLike(obj)) {
				lines.push(obj);
			}
		} catch {
			// skip non-JSON lines
		}
	}

	if (lines.length === 0) {
		out.fail("No frames found in file");
		return;
	}

	out.info(`Loaded ${lines.length} frames`);

	let sent = 0;
	for (let i = 0; i < lines.length; i++) {
		const frame = lines[i];
		const cmd = JSON.stringify({
			cmd: "raw",
			id: frame.id,
			data: frame.data,
			bus: frame.bus ?? 0,
		});
		session.send(cmd);
		sent++;

		if (i % 100 === 0 && i > 0) {
			out.info(`Sent ${sent}/${lines.length} frames...`);
		}

		// Delay between frames based on replay speed (default ~10ms between frames)
		if (replaySpeed > 0) {
			await delay(Math.max(1, Math.round(10 / replaySpeed)));
		}
	}

	out.pass(`Replayed ${sent} frames at ${replaySpeed}x speed`);
}

function isSafePathInput(value) {
	return typeof value === "string" && value.length > 0 && !value.includes("\0");
}

function isReplayFrameLike(value) {
	if (!value || typeof value !== "object") return false;
	if (typeof value.id !== "number") return false;
	if (value.data !== undefined && typeof value.data !== "string") return false;
	if (value.bus !== undefined && typeof value.bus !== "number") return false;
	return true;
}
