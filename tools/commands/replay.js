/** replay command — Replay recorded CAN frames from a JSONL file. */

import { createReadStream } from "node:fs";
import { createInterface } from "node:readline";
import { setTimeout as delay } from "node:timers/promises";
import { resolveInsideCwd } from "../lib/paths.js";

export async function runReplay(session, opts, out, _C) {
	const { replayFile, replaySpeed } = opts;

	if (!replayFile) {
		console.error("ERROR: --input <path> is required for replay command");
		process.exit(1);
	}

	const inputPath = resolveInsideCwd(replayFile);
	if (!inputPath) {
		out.fail("Invalid --input path");
		return;
	}

	out.section("CAN Frame Replay");
	out.info(`File: ${inputPath}`);
	out.info(`Speed: ${replaySpeed}x`);

	const rawLines = [];
	const rl = createInterface({ input: createReadStream(inputPath), crlfDelay: Infinity });
	for await (const line of rl) rawLines.push(line);
	const { frames: lines, skipped } = parseReplayLines(rawLines.join("\n"));

	out.info(`Skipped: ${skipped}`);

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

	out.pass(`Replayed ${sent} frames at ${replaySpeed}x speed; skipped: ${skipped}`);
}

/**
 * Split recorded text into replayable frames and count every non-empty line
 * that could not be used (CSV headers, malformed JSON, wrong shape).
 */
export function parseReplayLines(text) {
	const frames = [];
	let skipped = 0;
	for (const raw of String(text).split(/\r?\n/)) {
		const trimmed = raw.trim();
		if (!trimmed) continue;
		if (trimmed.startsWith("timestamp,")) {
			skipped += 1;
			continue;
		}
		try {
			const obj = JSON.parse(trimmed);
			if (isReplayFrameLike(obj)) frames.push(obj);
			else skipped += 1;
		} catch {
			skipped += 1;
		}
	}
	return { frames, skipped };
}

export function isReplayFrameLike(value) {
	if (!value || typeof value !== "object") return false;
	if (typeof value.id !== "number") return false;
	if (value.data !== undefined && typeof value.data !== "string") return false;
	if (value.bus !== undefined && typeof value.bus !== "number") return false;
	return true;
}
