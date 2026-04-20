/** watch command — Live CAN frame / state monitor with optional bit-diff. */

import { setTimeout as delay } from "node:timers/promises";
import { ts } from "../lib/output.js";

export function parseHexData(hexStr) {
	const clean = String(hexStr || "")
		.replace(/\s+/g, "")
		.toUpperCase();
	const bytes = [];
	for (let i = 0; i < clean.length; i += 2) bytes.push(parseInt(clean.slice(i, i + 2), 16));
	return bytes;
}

export function compareBits(prev, curr) {
	const changes = [];
	for (let byteIdx = 0; byteIdx < Math.max(prev.length, curr.length); byteIdx++) {
		const pB = prev[byteIdx] ?? 0;
		const cB = curr[byteIdx] ?? 0;
		if (pB !== cB) {
			const changedBits = [];
			for (let bit = 0; bit < 8; bit++) {
				const pBit = (pB >> bit) & 1;
				const cBit = (cB >> bit) & 1;
				if (pBit !== cBit) changedBits.push({ bit, from: pBit, to: cBit });
			}
			changes.push({ byteIdx, prevByte: pB, currByte: cB, changedBits });
		}
	}
	return changes;
}

export async function runWatch(session, opts, out, C) {
	const { watchDurMs, watchRaw, watchFsd, watchProfile, bitDiff, filterIds } = opts;
	const frameHistory = new Map();
	let watchState = { fsd: null, profile: null, ready: null, rawCan: null, streamOn: null };

	if (watchRaw) {
		session.send("can:raw:on");
		const ack = await session.waitForAck("can:raw:on", opts.timeoutMs);
		if (ack) out.info("raw CAN mode enabled");
		else out.warn("can:raw:on", "not acknowledged");
	}

	session.send("stream:on");
	const filterSet = new Set(filterIds);
	const deadline = Date.now() + watchDurMs;

	console.log(
		`\nWatching for ${watchDurMs}ms${filterSet.size ? `  filter: ${[...filterSet].join(",")}` : "  (all frames)"}${watchRaw ? "  [raw mode]" : ""}  Ctrl-C to stop\n`,
	);

	while (Date.now() < deadline) {
		const remaining = Math.max(100, deadline - Date.now());
		const entry = await session.waitFor(() => true, remaining);
		if (!entry) break;

		const { msg, raw } = entry;
		if (!msg) {
			console.log(`${C.dim}[${ts()}] text: ${raw}${C.reset}`);
			continue;
		}

		if (msg.t === "frame") {
			const id = Number(msg.id);
			if (filterSet.size && !filterSet.has(id)) continue;
			const idHex = `0x${id.toString(16).toUpperCase().padStart(3, "0")}`;
			const dir = msg.dir === "tx" ? `${C.yellow}TX${C.reset}` : `${C.green}RX${C.reset}`;
			const data = String(msg.d || "").toUpperCase();

			if (bitDiff) {
				const currentBytes = parseHexData(msg.d);
				const history = frameHistory.get(id) || { lastRxBytes: [], lastTxBytes: [] };
				if (msg.dir === "tx") {
					const refBytes =
						history.lastRxBytes.length > 0 ? history.lastRxBytes : history.lastTxBytes;
					const changes = compareBits(refBytes, currentBytes);
					if (changes.length > 0) {
						console.log(
							`[${ts()}] ${dir} id=${C.cyan}${idHex}${C.reset} dlc=${msg.dlc}  ${data}`,
						);
						for (const { byteIdx, prevByte, currByte, changedBits } of changes) {
							console.log(
								`    ${C.bold}Byte ${byteIdx}${C.reset}: 0x${prevByte.toString(16).toUpperCase().padStart(2, "0")} → 0x${currByte.toString(16).toUpperCase().padStart(2, "0")}`,
							);
							for (const { bit, from, to } of changedBits)
								console.log(
									`      ${C.yellow}bit ${bit}${C.reset}: ${from} → ${to}`,
								);
						}
					}
					history.lastTxBytes = currentBytes;
				} else {
					history.lastRxBytes = currentBytes;
				}
				frameHistory.set(id, history);
			} else {
				console.log(
					`[${ts()}] ${dir} id=${C.cyan}${idHex}${C.reset} dlc=${msg.dlc} seq=${msg.seq ?? "-"} ms=${msg.ms ?? "-"}  ${data}`,
				);
			}
			continue;
		}

		if (msg.t === "status") {
			const newFsd = Number(msg.fsd),
				newProfile = Number(msg.sp),
				newReady = msg.ready || null;
			if (!watchState._seen) {
				watchState = {
					_seen: true,
					fsd: newFsd,
					profile: newProfile,
					ready: newReady,
					rawCan: Number(msg.rawCan),
					streamOn: msg.stream?.on,
				};
				if (watchFsd || watchProfile)
					out.observation(
						"status",
						`initial — ready=${newReady} fsd=${newFsd} sp=${newProfile}`,
					);
				continue;
			}
			if (watchFsd && watchState.fsd !== newFsd) {
				out.observation("fsd", `${watchState.fsd} → ${newFsd}`);
				watchState.fsd = newFsd;
			}
			if (watchProfile && watchState.profile !== newProfile) {
				out.observation("profile", `${watchState.profile} → ${newProfile}`);
				watchState.profile = newProfile;
			}
			continue;
		}

		if (msg.t === "ack") {
			out.observation("ack", msg.cmd || "?");
			continue;
		}
		if (msg.t === "error") {
			out.observation("error", msg.msg || "?");
			continue;
		}
		if (msg.t === "log") out.observation("log", msg.msg || raw);
	}

	session.send("stream:off");
	if (watchRaw) {
		session.send("can:raw:off");
		await delay(200);
	}
	console.log("\nWatch ended.");
}
