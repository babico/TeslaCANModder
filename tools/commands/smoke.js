/** smoke command — Protocol health-check. */

import { setTimeout as delay } from "node:timers/promises";
import { tagPhase } from "../lib/diagnosis.js";

export async function runSmoke(session, opts, out) {
	const { timeoutMs, variant } = opts;

	out.section("Boot / connection");
	let boot = await session.waitForType("boot", timeoutMs * 2);
	if (boot) {
		out.pass(
			"Boot message received",
			`hw=${boot.msg.hw || "?"} variant=${boot.msg.variant || "?"}`,
		);
		if (boot.msg.cap) out.info(`capabilities: ${boot.msg.cap}`);
	} else {
		session.send("status");
		const status = await session.waitForType("status", timeoutMs);
		if (status)
			out.pass(
				"Board already running — status responded",
				`variant=${status.msg.variant || "?"}`,
			);
		else out.fail("Boot", "No boot or status message received within timeout");
	}

	out.section("Ping / pong");
	session.send("ping");
	const pong = await session.waitForType("pong", timeoutMs);
	if (pong) out.pass("Pong received");
	else out.fail("Pong", "No pong within timeout");

	out.section("Status message");
	session.send("status");
	const status = await session.waitForType("status", timeoutMs);
	if (status) {
		const s = status.msg;
		out.pass("Status received", `variant=${s.variant || "?"} ready=${s.ready || "?"}`);
		out.info(`fsd=${s.fsd ?? "?"}  sp=${s.sp ?? "?"}  up=${s.up ?? "?"}ms`);
		const missing = ["t", "hw", "variant", "ready"].filter((f) => s[f] === undefined);
		if (missing.length === 0) out.pass("Status schema valid");
		else out.fail("Status schema", `missing fields: ${missing.join(", ")}`);
	} else {
		out.fail("Status", "No status message received within timeout");
	}

	out.section("Filtered CAN stream");
	session.send("stream:on");
	const streamAck = await session.waitFor(
		(e) => e.msg?.t === "ack" || (e.msg?.t === "status" && e.msg?.stream?.on === 1),
		timeoutMs,
	);
	if (streamAck) out.pass("stream:on acknowledged");
	else out.warn("stream:on", "No ack — continuing");

	let frameCount = 0;
	const deadline = Date.now() + timeoutMs * 2;
	while (frameCount < 5 && Date.now() < deadline) {
		const frame = await session.waitForType("frame", Math.max(100, deadline - Date.now()));
		if (!frame) break;
		frameCount++;
	}
	tagPhase(session, "filtered");

	if (frameCount >= 5) out.pass(`${frameCount} filtered CAN frames received`);
	else if (frameCount > 0)
		out.warn(`Only ${frameCount} filtered frames`, "few frames — board connected to vehicle?");
	else out.warn("No filtered frames", "expected when bench-testing without CAN bus");

	session.send("stream:off");
	await delay(300);

	out.section("Raw CAN stream");
	session.send("can:raw:on");
	const rawAck = await session.waitForAck("can:raw:on", timeoutMs);

	if (rawAck) {
		out.pass("can:raw:on acknowledged — raw mode supported");
		session.send("stream:on");
		await delay(timeoutMs);
		session.send("stream:off");
		const rawFrames = session.drainType("frame");
		tagPhase(session, "raw");
		rawFrames.forEach((m) => {
			m._phase = "raw";
		});
		if (rawFrames.length > 0) out.pass(`${rawFrames.length} raw CAN frames seen`);
		else out.warn("No raw frames", "check physical CAN bus connection");
		session.send("can:raw:off");
		await delay(300);
	} else {
		out.info("can:raw:on not acknowledged — raw mode not available");
	}

	out.section("Variant switching");
	session.send("status");
	const pre = await session.waitForType("status", timeoutMs);
	const originalVariant = pre?.msg?.variant || null;

	session.send("variant:hw4");
	await delay(300);
	session.send("status");
	const post = await session.waitForType("status", timeoutMs);
	if (post?.msg?.variant === "hw4") out.pass("Variant switch to hw4 confirmed");
	else out.fail("Variant switch", `status shows variant=${post?.msg?.variant || "?"}`);

	if (originalVariant && originalVariant !== "hw4") {
		session.send(`variant:${originalVariant}`);
		await delay(300);
		out.info(`Restored variant to ${originalVariant}`);
	}
}
