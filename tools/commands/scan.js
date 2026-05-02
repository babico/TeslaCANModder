/** scan command — Discover CAN IDs on the bus and report unique IDs seen. */

import { setTimeout as delay } from "node:timers/promises";
import { ts } from "../lib/output.js";

export async function runScan(session, opts, out, C) {
	const { scanDurMs, timeoutMs, watchRaw } = opts;

	out.section("CAN ID Scanner");
	out.info(`Scanning for ${scanDurMs}ms...`);

	if (watchRaw) {
		session.send("can:raw:on");
		await session.waitForAck("can:raw:on", timeoutMs);
	}

	session.send("stream:on");

	const idMap = new Map(); // id -> { count, firstSeen, lastSeen, dirs: Set, dlcs: Set }
	const deadline = Date.now() + scanDurMs;

	while (Date.now() < deadline) {
		const entry = await session.waitFor(() => true, Math.max(100, deadline - Date.now()));
		if (!entry?.msg || entry.msg.t !== "frame") continue;

		const id = Number(entry.msg.id);
		const now = ts();
		const existing = idMap.get(id);

		if (existing) {
			existing.count++;
			existing.lastSeen = now;
			existing.dirs.add(entry.msg.dir);
			existing.dlcs.add(entry.msg.dlc);
		} else {
			idMap.set(id, {
				count: 1,
				firstSeen: now,
				lastSeen: now,
				dirs: new Set([entry.msg.dir]),
				dlcs: new Set([entry.msg.dlc]),
			});
		}
	}

	session.send("stream:off");
	if (watchRaw) {
		session.send("can:raw:off");
		await delay(200);
	}

	out.section("Scan Results");

	if (idMap.size === 0) {
		out.warn("No CAN IDs detected", "Check bus connection and vehicle state");
		return;
	}

	const sorted = [...idMap.entries()].sort((a, b) => a[0] - b[0]);

	console.log(
		`\n  ${"ID".padEnd(8)} ${"Hex".padEnd(8)} ${"Count".padEnd(8)} ${"Dir".padEnd(8)} ${"DLC".padEnd(8)} First Seen`,
	);
	console.log(`  ${"─".repeat(56)}`);

	for (const [id, info] of sorted) {
		const hex = `0x${id.toString(16).toUpperCase().padStart(3, "0")}`;
		const dirs = [...info.dirs].join("/").toUpperCase();
		const dlcs = [...info.dlcs].join(",");
		console.log(
			`  ${C.cyan}${String(id).padEnd(8)}${C.reset} ${hex.padEnd(8)} ${String(info.count).padEnd(8)} ${dirs.padEnd(8)} ${dlcs.padEnd(8)} ${info.firstSeen}`,
		);
	}

	console.log(`\n  ${C.bold}${idMap.size} unique CAN IDs${C.reset} detected in ${scanDurMs}ms`);
	out.pass(
		`Scan complete — ${idMap.size} unique IDs, ${[...idMap.values()].reduce((s, v) => s + v.count, 0)} total frames`,
	);
}
