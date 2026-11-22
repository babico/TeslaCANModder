/** dump command — Record CAN frames to file (JSONL or CSV). */

import { createWriteStream } from "node:fs";
import { setTimeout as delay } from "node:timers/promises";
import { ts } from "../lib/output.js";
import { csvField } from "../lib/canExport.js";
import { resolveInsideCwd } from "../lib/paths.js";
import { endStream, writeWithDrain } from "../lib/streams.js";

export async function runDump(session, opts, out) {
	const { watchDurMs, dumpFile, dumpFormat, timeoutMs, watchRaw } = opts;
	const outputName = dumpFile || `dump-${Date.now()}.${dumpFormat === "csv" ? "csv" : "jsonl"}`;

	const outputPath = resolveInsideCwd(outputName);
	if (!outputPath) {
		out.fail("Invalid --output path");
		return;
	}

	out.section("CAN Frame Dump");
	out.info(`Duration: ${watchDurMs}ms`);
	out.info(`Output: ${outputPath} (${dumpFormat})`);

	const stream = createWriteStream(outputPath);
	const streamErrors = [];
	stream.on("error", (error) => streamErrors.push(error));
	if (dumpFormat === "csv") {
		await writeWithDrain(stream, "timestamp,id,id_hex,dir,dlc,data,bus,seq\n");
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
			const cells = [now, m.id, hex, m.dir, m.dlc, m.d ?? "", m.bus ?? 0, m.seq ?? ""];
			await writeWithDrain(stream, `${cells.map(csvField).join(",")}\n`);
		} else {
			await writeWithDrain(
				stream,
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

	await endStream(stream, streamErrors);
	out.pass(`Dumped ${frameCount} frames to ${outputPath}`);
}
