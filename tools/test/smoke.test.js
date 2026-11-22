import { describe, expect, it, jest } from "@jest/globals";

import { runSmoke } from "../commands/smoke.js";

function makeOut() {
	return {
		section: jest.fn(),
		info: jest.fn(),
		pass: jest.fn(),
		warn: jest.fn(),
		fail: jest.fn(),
	};
}

describe("runSmoke raw CAN stream", () => {
	it("waits for frames instead of only sleeping after stream:on", async () => {
		const out = makeOut();
		const session = {
			send: jest.fn(),
			waitFor: jest.fn(async () => ({ msg: { t: "ack" } })),
			waitForAck: jest.fn(async () => ({ msg: { t: "ack", cmd: "can:raw:on" } })),
			waitForType: jest.fn(async () => null),
			drainType: jest.fn(() => []),
			messages: jest.fn(() => []),
		};
		let rawPhase = false;
		let rawFrameCalls = 0;
		session.send.mockImplementation((cmd) => {
			if (cmd === "can:raw:on") rawPhase = true;
			if (cmd === "can:raw:off") rawPhase = false;
		});
		session.waitForType.mockImplementation(async (type) => {
			if (type !== "frame" || !rawPhase) return null;
			rawFrameCalls += 1;
			return rawFrameCalls <= 2 ? { msg: { t: "frame", id: 1 } } : null;
		});

		await runSmoke(session, { timeoutMs: 20 }, out);

		expect(rawFrameCalls).toBeGreaterThanOrEqual(3);
		expect(out.pass).toHaveBeenCalledWith(expect.stringContaining("raw CAN frames"));
	});
});
