/** benchmark command — Measure CAN throughput and latency. */

import { setTimeout as delay } from "node:timers/promises";

export async function runBenchmark(session, opts, out) {
	const { benchDurMs, timeoutMs } = opts;

	out.section("CAN Throughput Benchmark");
	out.info(`Duration: ${benchDurMs}ms`);

	session.send("stream:on");
	await delay(500);

	let frameCount = 0;
	let minInterval = Infinity,
		maxInterval = 0;
	let lastTime = Date.now();
	const idCounts = new Map();
	const startTime = Date.now();
	const deadline = startTime + benchDurMs;

	while (Date.now() < deadline) {
		const entry = await session.waitFor(() => true, Math.max(100, deadline - Date.now()));
		if (!entry?.msg || entry.msg.t !== "frame") continue;

		frameCount++;
		const now = Date.now();
		const interval = now - lastTime;
		if (frameCount > 1) {
			if (interval < minInterval) minInterval = interval;
			if (interval > maxInterval) maxInterval = interval;
		}
		lastTime = now;

		const id = Number(entry.msg.id);
		idCounts.set(id, (idCounts.get(id) || 0) + 1);
	}

	session.send("stream:off");
	await delay(200);

	const elapsed = Date.now() - startTime;
	const fps = frameCount / (elapsed / 1000);

	out.section("Benchmark Results");
	out.info(`Elapsed: ${elapsed}ms`);
	out.info(`Frames: ${frameCount}`);
	out.info(`Throughput: ${fps.toFixed(1)} frames/sec`);
	out.info(`Unique IDs: ${idCounts.size}`);

	if (frameCount > 1) {
		out.info(`Min interval: ${minInterval}ms`);
		out.info(`Max interval: ${maxInterval}ms`);
		out.info(`Avg interval: ${(elapsed / frameCount).toFixed(1)}ms`);
	}

	// Top 5 busiest IDs
	const top = [...idCounts.entries()].sort((a, b) => b[1] - a[1]).slice(0, 5);
	if (top.length) {
		out.info("Top 5 busiest IDs:");
		for (const [id, count] of top) {
			const hex = `0x${id.toString(16).toUpperCase().padStart(3, "0")}`;
			out.info(
				`  ${hex} (${id}): ${count} frames (${((count / frameCount) * 100).toFixed(1)}%)`,
			);
		}
	}

	out.pass(`Benchmark complete — ${fps.toFixed(1)} fps over ${(elapsed / 1000).toFixed(1)}s`);
}
