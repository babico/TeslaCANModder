import { afterEach, describe, expect, it, jest } from "@jest/globals";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";

import { runDump } from "../commands/dump.js";
import { runReplay } from "../commands/replay.js";
import { runDriveContext } from "../commands/drive-context.js";
import { parseFrameId, resolveSpeedCapMax, runDasDrive } from "../commands/das-drive.js";
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

describe("dump CSV safety", () => {
	it("neutralizes formula-like cells in CSV output", async () => {
		const tempDir = fs.mkdtempSync(path.join(process.cwd(), "tcm-dump-"));
		const outPath = path.join(tempDir, "frames.csv");
		const out = makeOut();
		const session = makeSession();
		let call = 0;
		session.waitFor = jest.fn(async () => {
			call += 1;
			if (call === 1) {
				return {
					msg: {
						t: "frame",
						id: 1,
						dir: "=cmd|'/c calc'!A1",
						dlc: 8,
						d: "@SUM(1+1)",
						bus: 0,
						seq: 1,
					},
				};
			}
			return null;
		});

		try {
			await runDump(
				session,
				{ dumpFile: outPath, dumpFormat: "csv", watchDurMs: 10, timeoutMs: 10 },
				out,
			);

			const csv = fs.readFileSync(outPath, "utf8");
			expect(csv).toContain("'=cmd");
			expect(csv).toContain("'@SUM");
			const dataLine = csv.split("\n")[1];
			for (const cell of dataLine.split(",")) {
				expect(cell).not.toMatch(/^[=+\-@\t\r]/);
			}
		} finally {
			fs.rmSync(tempDir, { recursive: true, force: true });
		}
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

	it("soak hard-fails when can:raw:on is not acknowledged", async () => {
		const out = makeOut();
		const session = makeSession({
			t: "status",
			dasDriveEnabled: false,
			dasSpeedCapKph: 50,
			dasSpeedCapMaxKph: 200,
		});
		session.waitForAck = jest.fn(async () => null);

		await runDasDrive(session, { action: "soak", duration: 10, timeoutMs: 10 }, out);

		expect(out.fail).toHaveBeenCalledWith("can:raw:on", expect.any(String));
		expect(session.send).not.toHaveBeenCalledWith("stream:on");
	});
});

describe("das-drive frame id parsing", () => {
	it("passes numeric ids through unchanged", () => {
		expect(parseFrameId(697)).toBe(697);
		expect(parseFrameId(0)).toBe(0);
	});

	it("parses prefixed and bare hex strings", () => {
		expect(parseFrameId("0x2B9")).toBe(697);
		expect(parseFrameId("2b9")).toBe(697);
	});

	it("parses decimal strings as decimal, not hex", () => {
		expect(parseFrameId("697")).toBe(697);
		expect(parseFrameId("69")).toBe(69);
	});

	it("returns NaN for unusable values", () => {
		expect(parseFrameId(undefined)).toBeNaN();
		expect(parseFrameId(null)).toBeNaN();
		expect(parseFrameId(true)).toBeNaN();
		expect(parseFrameId("")).toBeNaN();
	});
});

describe("das-drive soak with drive disabled", () => {
	const statusMsg = {
		t: "status",
		dasDriveEnabled: false,
		dasSpeedCapKph: 50,
		dasSpeedCapMaxKph: 200,
	};

	function soakSession(frameCount) {
		const session = makeSession(statusMsg);
		const original = session.waitForType;
		let frames = 0;
		session.waitForType = jest.fn(async (type, ...rest) => {
			if (type === "frame") {
				if (frames < frameCount) {
					frames += 1;
					return { msg: { t: "frame", id: 0x2b9 } };
				}
				return null;
			}
			return original(type, ...rest);
		});
		return session;
	}

	it("treats a short cancel burst as silence", async () => {
		const out = makeOut();
		await runDasDrive(soakSession(5), { action: "soak", duration: 10, timeoutMs: 10 }, out);
		expect(out.pass).toHaveBeenCalledWith(
			"Drive disabled and no DAS frames seen on bus (expected)",
		);
	});

	it("warns when frames keep flowing above the noise threshold", async () => {
		const out = makeOut();
		await runDasDrive(soakSession(15), { action: "soak", duration: 10, timeoutMs: 10 }, out);
		expect(out.warn).toHaveBeenCalledWith(
			"soak",
			"Drive was disabled but frames are still flowing",
		);
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
