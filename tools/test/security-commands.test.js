import { afterEach, describe, expect, it, jest } from "@jest/globals";
import os from "node:os";
import path from "node:path";

import { runDump } from "../commands/dump.js";
import { runReplay } from "../commands/replay.js";
import { runDriveContext } from "../commands/drive-context.js";
import { resolveSpeedCapMax, runDasDrive } from "../commands/das-drive.js";
import { getPlatformIoHomes } from "../commands/flash.js";

function makeOut() {
	return {
		section: jest.fn(),
		info: jest.fn(),
		pass: jest.fn(),
		warn: jest.fn(),
		fail: jest.fn(),
	};
}

function makeSession(statusMsg) {
	return {
		send: jest.fn(),
		waitFor: jest.fn(async () => ({ msg: { t: "ack", cmd: "status" } })),
		waitForType: jest.fn(async () => ({ msg: statusMsg })),
		waitForAck: jest.fn(async () => ({ msg: { t: "ack" } })),
	};
}

describe("path traversal refusal", () => {
	it("dump refuses --output outside cwd before opening a stream", async () => {
		const out = makeOut();
		const session = makeSession();

		await runDump(
			session,
			{ dumpFile: "../outside.jsonl", dumpFormat: "jsonl", watchDurMs: 10 },
			out,
		);

		expect(out.fail).toHaveBeenCalledWith("Invalid --output path");
		expect(session.send).not.toHaveBeenCalled();
	});

	it("replay refuses --input outside cwd before opening a stream", async () => {
		const out = makeOut();
		const session = makeSession();

		await runReplay(session, { replayFile: "../outside.jsonl", replaySpeed: 1 }, out);

		expect(out.fail).toHaveBeenCalledWith("Invalid --input path");
		expect(session.send).not.toHaveBeenCalled();
	});

	it("drive-context refuses --output outside cwd before querying status", async () => {
		const out = makeOut();
		const session = makeSession({ t: "status" });

		await runDriveContext(
			session,
			{ driveCtxDurMs: 10, driveCtxOutput: "../outside.json", driveCtxMinSamples: 1 },
			out,
		);

		expect(out.fail).toHaveBeenCalledWith("Invalid --output path");
		expect(session.send).not.toHaveBeenCalled();
	});
});

describe("das-drive speed cap", () => {
	it("derives the cap ceiling from status but never exceeds 200", () => {
		expect(resolveSpeedCapMax(undefined)).toBe(200);
		expect(resolveSpeedCapMax(500)).toBe(200);
		expect(resolveSpeedCapMax(80)).toBe(80);
		expect(resolveSpeedCapMax(0)).toBe(200);
	});

	it("set-speed refuses a value above the reported cap maximum", async () => {
		const out = makeOut();
		const session = makeSession({
			t: "status",
			dasDriveEnabled: false,
			dasSpeedCapKph: 50,
			dasSpeedCapMaxKph: 200,
		});

		await runDasDrive(session, { action: "set-speed", kph: 999, timeoutMs: 50 }, out);

		expect(out.fail).toHaveBeenCalledWith("set-speed", expect.stringContaining("1..200"));
		expect(session.send).not.toHaveBeenCalledWith("drive:speed:999");
	});

	it("set-speed honors a lower cap maximum from status", async () => {
		const out = makeOut();
		const session = makeSession({
			t: "status",
			dasDriveEnabled: false,
			dasSpeedCapKph: 50,
			dasSpeedCapMaxKph: 80,
		});

		await runDasDrive(session, { action: "set-speed", kph: 100, timeoutMs: 50 }, out);

		expect(out.fail).toHaveBeenCalledWith("set-speed", expect.stringContaining("1..80"));
	});
});

describe("flash platformio home", () => {
	const original = process.env.PLATFORMIO_CORE_DIR;

	afterEach(() => {
		if (original === undefined) delete process.env.PLATFORMIO_CORE_DIR;
		else process.env.PLATFORMIO_CORE_DIR = original;
	});

	it("ignores PLATFORMIO_CORE_DIR override", () => {
		const override = path.join(os.tmpdir(), "tcm-fake-core-dir");
		process.env.PLATFORMIO_CORE_DIR = override;

		expect(getPlatformIoHomes()).toEqual([path.join(os.homedir(), ".platformio")]);
		expect(getPlatformIoHomes()).not.toContain(override);
	});
});
