/** vehicle command — Send vehicle control commands interactively or one-shot. */

import { setTimeout as delay } from "node:timers/promises";

const VEHICLE_COMMANDS = {
	// mirrors
	"mirror-fold": "mirror:fold",
	"mirror-unfold": "mirror:unfold",
	"mirror-heat": "mirror:heat",
	// locks
	lock: "lock",
	unlock: "unlock",
	"child-lock": "lock:child",
	horn: "horn",
	// trunk
	"frunk-open": "frunk:open",
	"frunk-close": "frunk:close",
	"trunk-open": "trunk:open",
	"trunk-close": "trunk:close",
	glovebox: "glovebox",
	// lights
	"fog-front": "light:fog:front",
	"fog-rear": "light:fog:rear",
	"dome-on": "light:dome:on",
	"dome-off": "light:dome:off",
	"dome-auto": "light:dome:auto",
	// wipers
	"wiper-off": "wiper:off",
	"wiper-1": "wiper:1",
	"wiper-2": "wiper:2",
	"wiper-3": "wiper:3",
	// climate
	"climate-on": "climate:keep",
	"climate-off": "climate:off",
	// charge
	"charge-start": "charge:start",
	"charge-stop": "charge:stop",
	"charge-port": "chargeport",
	// sentry
	"sentry-on": "sentry:on",
	"sentry-off": "sentry:off",
	// vent
	"vent-open": "vent:open",
	"vent-close": "vent:close",
	// power
	"acc-on": "power:acc:on",
	"acc-off": "power:acc:off",
	"power-ready": "power:ready",
	"power-off": "power:off",
};

export async function runVehicle(session, opts, out) {
	const { vehicleCmd, timeoutMs } = opts;

	if (!vehicleCmd) {
		out.section("Available Vehicle Commands");
		const keys = Object.keys(VEHICLE_COMMANDS).sort();
		for (const k of keys) console.log(`  --vehicle-cmd ${k}`);
		console.log(`\n  ${keys.length} commands available`);
		return;
	}

	const mapped = VEHICLE_COMMANDS[vehicleCmd];
	if (!mapped) {
		out.fail(`Unknown vehicle command: ${vehicleCmd}`);
		out.info(`Available: ${Object.keys(VEHICLE_COMMANDS).join(", ")}`);
		return;
	}

	out.section(`Vehicle Command: ${vehicleCmd}`);
	session.send(mapped);

	const ack = await session.waitFor((e) => e.msg?.t === "ack" || e.msg?.t === "error", timeoutMs);

	if (ack?.msg?.t === "ack") {
		out.pass(`${vehicleCmd} acknowledged`, `cmd=${ack.msg.cmd}`);
	} else if (ack?.msg?.t === "error") {
		out.fail(`${vehicleCmd} error`, ack.msg.msg);
	} else {
		out.warn(`${vehicleCmd}`, "No response within timeout");
	}
}
