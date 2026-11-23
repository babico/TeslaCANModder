import { describe, expect, it } from "@jest/globals";
import type { BusId, CanFrame } from "../src/can/socketcan.js";
import { createCandumpTransportFactory, parseCandumpLine } from "../src/can/candump.js";
import { createCanReader } from "../src/can/socketcan.js";
import { fakeCandump } from "./support/fakeProcess.js";

const noSleep = async (): Promise<void> => {};

describe("candump -L line parsing", () => {
	it("parses a timestamped SFF data line into the shared CanFrame shape", () => {
		expect(
			parseCandumpLine("(1764000000.123456) can0 3FD#0102030405060708", "can0", 42),
		).toEqual({
			id: 0x3fd,
			dlc: 8,
			data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
			bus: 0,
			rxMs: 42,
		} as CanFrame);
	});

	it("parses an untimestamped line and a zero-length payload", () => {
		expect(parseCandumpLine("can1 123#", "can1", 7)).toEqual({
			id: 0x123,
			dlc: 0,
			data: new Uint8Array(0),
			bus: 1 as BusId,
			rxMs: 7,
		});
	});

	it("accepts lowercase hex and maps can2 to bus 2", () => {
		expect(parseCandumpLine("can2 2b9#aabb", "can2", 0)).toEqual({
			id: 0x2b9,
			dlc: 2,
			data: new Uint8Array([0xaa, 0xbb]),
			bus: 2,
			rxMs: 0,
		});
	});

	it("accepts the classic bracketed format as a fallback", () => {
		expect(parseCandumpLine("can0 3FD [2] 01 02", "can0", 9)).toEqual({
			id: 0x3fd,
			dlc: 2,
			data: new Uint8Array([1, 2]),
			bus: 0,
			rxMs: 9,
		});
	});

	it("skips remote frames, error/extended IDs, wrong interfaces and malformed lines", () => {
		for (const line of [
			"",
			"(1764000000.1) can0 123#R",
			"can0 20000004#0000000000000000",
			"can0 18FEF100#1122334455667788",
			"can1 123#0102", // wrong iface for the can0 transport
			"can0 xyz#0102",
			"can0 123#0",
			"can0 123#010203040506070809101112", // > 8 bytes
			"garbage",
		]) {
			expect(parseCandumpLine(line, "can0", 0)).toBeUndefined();
		}
	});
});

describe("candump transport wired through the shared reader", () => {
	it("spawns `candump -L can0` and converts log lines into CanFrames", async () => {
		const fake = fakeCandump([
			[
				{ kind: "line", line: "(1764000000.1) can0 3FD#0102030405060708" },
				{ kind: "line", line: "can0 2B9#AABB" },
			],
		]);
		const frames: CanFrame[] = [];
		for await (const frame of createCanReader("can0", {
			transport: createCandumpTransportFactory({ processFactory: fake.factory }),
			now: () => 5,
			sleep: noSleep,
		})) {
			frames.push(frame);
		}
		expect(fake.calls()).toEqual([["candump", ["-L", "can0"]]]);
		expect(frames).toEqual([
			{
				id: 0x3fd,
				dlc: 8,
				data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
				bus: 0,
				rxMs: 5,
			},
			{ id: 0x2b9, dlc: 2, data: new Uint8Array([0xaa, 0xbb]), bus: 0, rxMs: 5 },
		]);
	});

	it("reconnects the process after a stream error and keeps converting lines", async () => {
		const fake = fakeCandump([
			[
				{ kind: "line", line: "can1 3FD#01" },
				{ kind: "error", error: new Error("candump died") },
			],
			[{ kind: "line", line: "can1 2B9#AA" }],
		]);
		const frames: CanFrame[] = [];
		for await (const frame of createCanReader("can1", {
			transport: createCandumpTransportFactory({ processFactory: fake.factory }),
			now: () => 11,
			sleep: noSleep,
		})) {
			frames.push(frame);
		}
		expect(frames.map((frame) => frame.id)).toEqual([0x3fd, 0x2b9]);
		expect(fake.calls().length).toBe(2);
		expect(fake.killCount()).toBe(2);
	});

	it("reconnects after a clean child exit with no spawn error", async () => {
		// The child ends its stdout normally (no error seam) but its `exit`
		// event fired: today the reader returns and stops seeing CAN forever.
		const fake = fakeCandump([
			[
				{ kind: "line", line: "can0 3FD#01" },
				{ kind: "exit", code: 0 },
			],
			[
				{ kind: "line", line: "can0 2B9#AA" },
				{ kind: "exit", code: 0 },
			],
		]);
		const frames: CanFrame[] = [];
		let finalError: unknown;
		try {
			for await (const frame of createCanReader("can0", {
				transport: createCandumpTransportFactory({ processFactory: fake.factory }),
				now: () => 12,
				sleep: noSleep,
				maxReconnects: 1,
			})) {
				frames.push(frame);
			}
		} catch (error) {
			// The reader rethrows once its reconnect budget is exhausted; that
			// is expected here and proves clean exits were reconnectable.
			finalError = error;
		}
		expect(frames.map((frame) => frame.id)).toEqual([0x3fd, 0x2b9]);
		expect(fake.calls().length).toBe(2);
		expect(String(finalError)).toMatch(/candump exited unexpectedly/);
	});
});
