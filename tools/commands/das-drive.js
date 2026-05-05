/**
 * das-drive command — Diagnostics & live exerciser for the gamepad-driven
 * DAS injection feature (firmware/lib/vehicle/can/feature/das_drive.h).
 *
 * Run modes:
 *   --action status                  Print current drive flags + speed/cap.
 *   --action soak --duration 5000    Repeatedly request status, report frame
 *                                    rate of DAS_control / DAS_steeringControl
 *                                    / APS_eacMonitor on the wire (needs
 *                                    `can:raw:on` + `stream:on` running).
 *   --action set-cap --kph 50        Send drive:cap:50 and verify status.
 *   --action set-speed --kph 25      Send drive:speed:25 and verify.
 *   --action toggle                  Flip drive:on / drive:off and verify.
 *
 * SAFETY: Only use against a bench board or a vehicle on jack stands /
 * private property. `set-cap` and `toggle` MUTATE persisted NVS state.
 */

import { setTimeout as delay } from "node:timers/promises";

// CAN IDs that the DAS Drive feature emits on BUS_CHASSIS.
const DAS_CONTROL_ID = 0x2b9; // 25 Hz longitudinal
const DAS_STEER_ID = 0x488; // 50 Hz lateral
const APS_EAC_ID = 0x27d; // 10 Hz EPAS allow

const ACTIONS = new Set(["status", "soak", "set-cap", "set-speed", "toggle"]);

/** Pull the latest /api/status-shape fields from a `status` message. */
function pickDrive(msg) {
	if (!msg) return null;
	return {
		enabled: !!msg.dasDriveEnabled,
		limit: msg.dasSpeedLimitKph,
		cap: msg.dasSpeedCapKph,
		capMax: msg.dasSpeedCapMaxKph,
	};
}

/** Send a command and wait for an ack/log/status that confirms it landed. */
async function sendAndConfirm(session, cmd, timeoutMs) {
	session.send(cmd);
	const reply = await session.waitFor(
		(e) =>
			e.msg?.t === "ack" ||
			e.msg?.t === "log" ||
			(e.msg?.t === "status" && e.msg?.dasDriveEnabled !== undefined),
		timeoutMs,
	);
	return reply;
}

/** Re-query status and return the parsed drive block. */
async function fetchDriveStatus(session, timeoutMs) {
	session.send("status");
	const s = await session.waitForType("status", timeoutMs);
	return pickDrive(s?.msg);
}

export async function runDasDrive(session, opts, out) {
	const { timeoutMs } = opts;
	const action = opts.action || "status";
	if (!ACTIONS.has(action)) {
		out.fail("das-drive", `Unknown --action "${action}". Valid: ${[...ACTIONS].join(", ")}`);
		return;
	}

	// Always print the starting picture so soak/set/toggle have a baseline.
	out.section("Initial DAS drive state");
	const before = await fetchDriveStatus(session, timeoutMs);
	if (!before) {
		out.fail("status", "Board did not respond to status query");
		return;
	}
	out.info(
		`enabled=${before.enabled}  speedLimit=${before.limit} kph  ` +
			`cap=${before.cap} kph  capMax=${before.capMax} kph`,
	);

	// ── status: just print and exit ──────────────────────────────────────────
	if (action === "status") {
		out.pass("DAS drive status reported");
		return;
	}

	// ── set-cap: mutate the runtime cap and verify NVS round-trip ────────────
	if (action === "set-cap") {
		const kph = Number(opts.kph);
		if (!Number.isFinite(kph) || kph < 1 || kph > 200) {
			out.fail("set-cap", "--kph must be an integer 1..200");
			return;
		}
		out.section(`Setting cap → ${kph} kph`);
		const reply = await sendAndConfirm(session, `drive:cap:${kph}`, timeoutMs);
		if (!reply) out.warn("drive:cap", "no ack within timeout — verifying via status");
		const after = await fetchDriveStatus(session, timeoutMs);
		if (after && Math.abs(after.cap - kph) < 0.5) {
			out.pass(`Cap accepted (status reports ${after.cap} kph)`);
		} else {
			out.fail("set-cap", `Cap did not update (status reports ${after?.cap})`);
		}
		return;
	}

	// ── set-speed: mutate the user limit (firmware clamps to current cap) ───
	if (action === "set-speed") {
		const kph = Number(opts.kph);
		if (!Number.isFinite(kph) || kph < 1) {
			out.fail("set-speed", "--kph must be a positive integer");
			return;
		}
		out.section(`Setting user speed limit → ${kph} kph`);
		await sendAndConfirm(session, `drive:speed:${kph}`, timeoutMs);
		const after = await fetchDriveStatus(session, timeoutMs);
		const expected = Math.min(kph, after?.cap ?? kph);
		if (after && Math.abs(after.limit - expected) < 0.5) {
			out.pass(`Speed limit reports ${after.limit} kph (expected ${expected})`);
		} else {
			out.fail("set-speed", `Speed limit reports ${after?.limit}, expected ${expected}`);
		}
		return;
	}

	// ── toggle: flip enabled, confirm, restore previous state ────────────────
	if (action === "toggle") {
		const target = !before.enabled;
		out.section(`Toggling drive → ${target ? "ON" : "OFF"}`);
		await sendAndConfirm(session, target ? "drive:on" : "drive:off", timeoutMs);
		const mid = await fetchDriveStatus(session, timeoutMs);
		if (mid?.enabled !== target) {
			out.fail("toggle", `Status reports enabled=${mid?.enabled}, expected ${target}`);
			return;
		}
		out.pass(`Drive is now ${target ? "ON" : "OFF"}`);
		// Restore — we only flip momentarily so we don't leave the board armed.
		await sendAndConfirm(session, before.enabled ? "drive:on" : "drive:off", timeoutMs);
		const back = await fetchDriveStatus(session, timeoutMs);
		out.info(`Restored to enabled=${back?.enabled}`);
		return;
	}

	// ── soak: enable raw stream + count DAS-frame cadence on the wire ────────
	if (action === "soak") {
		const duration = Number(opts.duration) || 5000;
		out.section(`Soaking ${duration} ms — measuring DAS frame cadence`);

		// We need raw CAN frames to see the IDs we emit.
		session.send("can:raw:on");
		await session.waitForAck("can:raw:on", timeoutMs);
		session.send("stream:on");

		const counts = { control: 0, steer: 0, eac: 0 };
		const deadline = Date.now() + duration;
		while (Date.now() < deadline) {
			const f = await session.waitForType("frame", Math.max(50, deadline - Date.now()));
			if (!f) continue;
			// Frame messages carry the CAN id — accept either decimal or hex string.
			const idRaw = f.msg.id ?? f.msg.canId;
			const id = typeof idRaw === "string" ? parseInt(idRaw, 16) || Number(idRaw) : idRaw;
			if (id === DAS_CONTROL_ID) counts.control++;
			else if (id === DAS_STEER_ID) counts.steer++;
			else if (id === APS_EAC_ID) counts.eac++;
		}

		session.send("stream:off");
		session.send("can:raw:off");

		const sec = duration / 1000;
		const ctrlHz = counts.control / sec;
		const steerHz = counts.steer / sec;
		const eacHz = counts.eac / sec;
		out.info(
			`DAS_control     : ${counts.control} frames (${ctrlHz.toFixed(1)} Hz, expect ~25)`,
		);
		out.info(`DAS_steeringCtl : ${counts.steer} frames (${steerHz.toFixed(1)} Hz, expect ~50)`);
		out.info(`APS_eacMonitor  : ${counts.eac} frames (${eacHz.toFixed(1)} Hz, expect ~10)`);

		// Tolerance ±20% — very loose, just catches "frames not flowing" bugs.
		const ok =
			Math.abs(ctrlHz - 25) < 5 && Math.abs(steerHz - 50) < 10 && Math.abs(eacHz - 10) < 2;
		if (!before.enabled) {
			// Drive was off — frames should be ~0 unless cancel burst is in progress.
			if (counts.control + counts.steer + counts.eac < 10) {
				out.pass("Drive disabled and no DAS frames seen on bus (expected)");
			} else {
				out.warn("soak", "Drive was disabled but frames are still flowing");
			}
		} else if (ok) {
			out.pass("All three DAS frame rates within ±20% of nominal");
		} else {
			out.fail("soak", "One or more DAS frame rates out of spec");
		}
	}
}
