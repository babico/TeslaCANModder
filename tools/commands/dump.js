/** dump command — Record CAN frames to file (JSONL or CSV). */

import { createWriteStream } from "node:fs";
import { resolve } from "node:path";
import { setTimeout as delay } from "node:timers/promises";
import { ts } from "../lib/output.js";

export async function runDump(session, opts, out) {
	const { watchDurMs, dumpFile, dumpFormat, timeoutMs, watchRaw } = opts;
	const outputName = dumpFile || `dump-${Date.now()}.${dumpFormat === "csv" ? "csv" : "jsonl"}`;

	if (!isSafePathInput(outputName)) {
		out.fail("Invalid --output path");
		return;
	}

	const outputPath = resolve(outputName);

	out.section("CAN Frame Dump");
	out.info(`Duration: ${watchDurMs}ms`);
	out.info(`Output: ${outputPath} (${dumpFormat})`);

	const stream = createWriteStream(outputPath);
	if (dumpFormat === "csv") {
		stream.write("timestamp,id,id_hex,dir,dlc,data,bus,seq\n");
	}

	if (watchRaw) {
		session.send("can:raw:on");
		await session.waitForAck("can:raw:on", timeoutMs);
	}

	session.send("stream:on");
	let frameCount = 0;
	const deadline = Date.now() + watchDurMs;

	while (Date.now() < deadline) {
		const entry = await session.waitFor(() => true, Math.max(100, deadline - Date.now()));
		if (!entry?.msg || entry.msg.t !== "frame") continue;

		frameCount++;
		const m = entry.msg;
		const now = ts();

		if (dumpFormat === "csv") {
			const hex = `0x${Number(m.id).toString(16).toUpperCase().padStart(3, "0")}`;
			stream.write(
				`${now},${m.id},${hex},${m.dir},${m.dlc},${m.d || ""},${m.bus ?? 0},${m.seq ?? ""}\n`,
			);
		} else {
			stream.write(
				JSON.stringify({
					ts: now,
					id: m.id,
					dir: m.dir,
					dlc: m.dlc,
					data: m.d,
					bus: m.bus,
					seq: m.seq,
				}) + "\n",
			);
		}
	}

	session.send("stream:off");
	if (watchRaw) {
		session.send("can:raw:off");
		await delay(200);
	}

	stream.end();
	out.pass(`Dumped ${frameCount} frames to ${outputPath}`);
}

function isSafePathInput(value) {
	return typeof value === "string" && value.length > 0 && !value.includes("\0");
}
