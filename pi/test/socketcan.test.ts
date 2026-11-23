import { describe, expect, it } from "@jest/globals";
import type { BusId, CanFrame } from "../src/can/socketcan.js";
import {
	CAN_FRAME_BYTES,
	busFromIface,
	createCanReader,
	frameToWire,
	frameToWireJson,
	createNetTransport,
	parseCanFrame,
	parseGatewayPort,
} from "../src/can/socketcan.js";
import { canFrameBytes, createFakeFactory } from "./support/fakeSocket.js";

/** Literal 0x3FD (1021) frame: dlc=8, payload 01..08. */
const FSD_FRAME = new Uint8Array([
	0xfd,
	0x03,
	0x00,
	0x00, // can_id = 0x3FD, 11-bit, little-endian
	0x08,
	0x00,
	0x00,
	0x00, // can_dlc = 8 + 3 pad bytes
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08, // data[8]
]);

const noSleep = async (): Promise<void> => {};

describe("socketcan frame parsing", () => {
	it("uses the 16-byte SocketCAN can_frame layout", () => {
		expect(CAN_FRAME_BYTES).toBe(16);
	});

	it("parses a literal 11-bit can_frame into a CanFrame", () => {
		expect(parseCanFrame(FSD_FRAME, 0, 1234)).toEqual({
			id: 0x3fd,
			dlc: 8,
			data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
			bus: 0,
			rxMs: 1234,
		});
	});

	it("accepts DLC 0 through 8 and rejects DLC 9+ or a short buffer", () => {
		expect(parseCanFrame(canFrameBytes(0x7ff, 0, []), 1, 0)).toEqual({
			id: 0x7ff,
			dlc: 0,
			data: new Uint8Array(0),
			bus: 1,
			rxMs: 0,
		});
		expect(parseCanFrame(canFrameBytes(0x2b9, 8, [1, 2, 3, 4, 5, 6, 7, 8]), 0, 0).data).toEqual(
			new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
		);
		expect(() =>
			parseCanFrame(canFrameBytes(0x123, 9, [1, 2, 3, 4, 5, 6, 7, 8, 9]), 0, 0),
		).toThrow(/dlc/i);
		expect(() => parseCanFrame(FSD_FRAME.subarray(0, 15), 0, 0)).toThrow(/length/i);
	});

	it("rejects extended, RTR, and error frames (11-bit IDs only)", () => {
		expect(() => parseCanFrame(canFrameBytes(0x80000123, 1, [0]), 0, 0)).toThrow(
			/extended|flag/i,
		);
		expect(() => parseCanFrame(canFrameBytes(0x40000123, 1, [0]), 0, 0)).toThrow(
			/extended|flag/i,
		);
		expect(() => parseCanFrame(canFrameBytes(0x20000123, 1, [0]), 0, 0)).toThrow(
			/extended|flag/i,
		);
	});

	it("maps can0/can1/can2 to bus 0/1/2", async () => {
		const cases: Array<[string, BusId]> = [
			["can0", 0],
			["can1", 1],
			["can2", 2],
		];
		for (const [iface, bus] of cases) {
			const fake = createFakeFactory([[{ kind: "chunk", bytes: FSD_FRAME }]]);
			const frames: CanFrame[] = [];
			for await (const frame of createCanReader(iface, {
				transport: fake.factory,
				now: () => 42,
				sleep: noSleep,
			})) {
				frames.push(frame);
			}
			expect(frames).toEqual([
				{
					id: 0x3fd,
					dlc: 8,
					data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
					bus,
					rxMs: 42,
				},
			]);
			expect(fake.ifaces()).toEqual([iface]);
		}
	});

	it("accepts vcanN loopback aliases used by the vcan backend", () => {
		expect(busFromIface("vcan0")).toBe(0);
		expect(busFromIface("vcan2")).toBe(2);
		expect(() => busFromIface("vcan3")).toThrow(/can0/i);
	});

	it("rejects interfaces outside can0..can2", () => {
		expect(() => busFromIface("can3")).toThrow(/can0/i);
		expect(() => busFromIface("eth0")).toThrow(/can0/i);
		expect(() => createCanReader("can9", { transport: createFakeFactory([]).factory })).toThrow(
			/can0/i,
		);
	});

	it("reconnects after a transport error and keeps yielding frames", async () => {
		const second = canFrameBytes(0x2b9, 2, [0xaa, 0xbb]);
		const fake = createFakeFactory([
			[
				{ kind: "chunk", bytes: FSD_FRAME },
				{ kind: "error", error: new Error("link down") },
			],
			[{ kind: "chunk", bytes: second }], // clean end terminates the reader
		]);
		let tick = 0;
		const frames: CanFrame[] = [];
		for await (const frame of createCanReader("can1", {
			transport: fake.factory,
			now: () => 100 + tick++,
			sleep: noSleep,
		})) {
			frames.push(frame);
		}
		expect(frames).toEqual([
			{
				id: 0x3fd,
				dlc: 8,
				data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]),
				bus: 1,
				rxMs: 100,
			},
			{ id: 0x2b9, dlc: 2, data: new Uint8Array([0xaa, 0xbb]), bus: 1, rxMs: 101 },
		]);
		expect(fake.openCount()).toBe(2);
		expect(fake.ifaces()).toEqual(["can1", "can1"]);
	});

	it("reassembles a frame split across chunks and two frames in one chunk", async () => {
		const a = canFrameBytes(0x3fd, 1, [0x11]);
		const b = canFrameBytes(0x2b9, 2, [0x22, 0x33]);
		const joined = new Uint8Array(32);
		joined.set(a, 0);
		joined.set(b, 16);
		const fake = createFakeFactory([
			[
				{ kind: "chunk", bytes: joined.subarray(0, 20) }, // frame A + 4 bytes of B
				{ kind: "chunk", bytes: joined.subarray(20) }, // remainder of B
			],
		]);
		const frames: CanFrame[] = [];
		for await (const frame of createCanReader("can0", {
			transport: fake.factory,
			now: () => 7,
			sleep: noSleep,
		})) {
			frames.push(frame);
		}
		expect(frames).toEqual([
			{ id: 0x3fd, dlc: 1, data: new Uint8Array([0x11]), bus: 0, rxMs: 7 },
			{ id: 0x2b9, dlc: 2, data: new Uint8Array([0x22, 0x33]), bus: 0, rxMs: 7 },
		]);
	});

	it("passes the A1 CAN_TRANSPORT backend to the transport factory", async () => {
		const fake = createFakeFactory([[{ kind: "chunk", bytes: FSD_FRAME }]]);
		let count = 0;
		for await (const _frame of createCanReader("can0", {
			transport: fake.factory,
			backend: "vcan",
			now: () => 0,
			sleep: noSleep,
		})) {
			count += 1;
		}
		expect(count).toBe(1);
		expect(fake.backends()).toEqual(["vcan"]);
	});
});

describe("optional TCP gateway validation", () => {
	it("parses a valid CAN_GATEWAY_PORT and rejects non-integers out of 1..65535", () => {
		expect(parseGatewayPort({ CAN_GATEWAY_PORT: "3290" })).toBe(3290);
		expect(parseGatewayPort({})).toBe(3290); // documented default
		for (const value of ["0", "65536", "-1", "80x", "3.5", " 12"]) {
			expect(() => parseGatewayPort({ CAN_GATEWAY_PORT: value })).toThrow(/port/i);
		}
	});

	it("fails fast at construction on a bad port or a port+bus overflow", () => {
		expect(() => createNetTransport("can0", "socketcan", { CAN_GATEWAY_PORT: "0" })).toThrow(
			/port/i,
		);
		expect(() =>
			createNetTransport("can2", "socketcan", { CAN_GATEWAY_PORT: "65534" }),
		).toThrow(/port/i);
	});
});

describe("reader reassembly across a reconnect", () => {
	it("keeps a partial frame across a transport error (carry hoisted outside the loop)", async () => {
		const a = canFrameBytes(0x3fd, 1, [0x11]);
		const b = canFrameBytes(0x2b9, 2, [0x22, 0x33]);
		const joined = new Uint8Array(32);
		joined.set(a, 0);
		joined.set(b, 16);
		const fake = createFakeFactory([
			[
				{ kind: "chunk", bytes: joined.subarray(0, 20) }, // A + 4 bytes of B
				{ kind: "error", error: new Error("link bounced") },
			],
			[{ kind: "chunk", bytes: joined.subarray(20) }], // remainder of B
		]);
		const frames: CanFrame[] = [];
		for await (const frame of createCanReader("can0", {
			transport: fake.factory,
			now: () => 7,
			sleep: noSleep,
		})) {
			frames.push(frame);
		}
		expect(frames).toEqual([
			{ id: 0x3fd, dlc: 1, data: new Uint8Array([0x11]), bus: 0, rxMs: 7 },
			{ id: 0x2b9, dlc: 2, data: new Uint8Array([0x22, 0x33]), bus: 0, rxMs: 7 },
		]);
	});
});

describe("socketcan wire serialization", () => {
	it("emits the firmware {t:frame} JSON shape", () => {
		const frame = parseCanFrame(FSD_FRAME, 0, 1);
		expect(frameToWire(frame)).toEqual({
			t: "frame",
			dir: "rx",
			bus: 0,
			id: 1021,
			dlc: 8,
			d: "0102030405060708",
		});
		expect(frameToWireJson(frame)).toBe(
			'{"t":"frame","dir":"rx","bus":0,"id":1021,"dlc":8,"d":"0102030405060708"}',
		);
	});

	it("serializes a zero-length frame with an empty hex field", () => {
		const frame = parseCanFrame(canFrameBytes(0x123, 0, []), 2, 0);
		expect(frameToWire(frame)).toEqual({
			t: "frame",
			dir: "rx",
			bus: 2,
			id: 0x123,
			dlc: 0,
			d: "",
		});
	});
});
