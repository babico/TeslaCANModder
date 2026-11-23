import { afterEach, describe, expect, it, jest } from "@jest/globals";
import type { RequestListener, Server } from "node:http";
import type { CanFrame, CanReaderOptions, TransportFactory } from "../src/can/socketcan.js";
import type { BridgeOptions } from "../src/http/bridge.js";
import type { PiServerOptions } from "../src/index.js";
import type { Recorder, RecorderOptions } from "../src/record/recorder.js";
import { fanoutFrames, parseBindConfig, startPiService } from "../src/main.js";

const ENV = {
	CAN_TRANSPORT: "socketcan",
	TCM_BIND_HOST: "127.0.0.1",
	TCM_BIND_PORT: "8090",
	TCM_BRIDGE_TOKEN: "service-token",
	TCM_CAPTURE_DIR: "/var/lib/teslacanmodder/captures",
} as NodeJS.ProcessEnv;

const FRAMES: CanFrame[] = [
	{ id: 0x3fd, dlc: 8, data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]), bus: 0, rxMs: 1 },
	{ id: 0x2b9, dlc: 2, data: new Uint8Array([0xaa, 0xbb]), bus: 0, rxMs: 2 },
];

function fakeReader(frames: readonly CanFrame[]): AsyncIterable<CanFrame> {
	return {
		async *[Symbol.asyncIterator](): AsyncGenerator<CanFrame> {
			for (const frame of frames) yield frame;
		},
	};
}

async function waitFor(predicate: () => boolean, tries = 50): Promise<void> {
	for (let i = 0; i < tries; i += 1) {
		if (predicate()) return;
		await new Promise<void>((resolve) => setImmediate(resolve));
	}
	throw new Error("condition not met before timeout");
}

describe("bind configuration", () => {
	it("reads the loopback pin from the environment", () => {
		expect(parseBindConfig(ENV)).toEqual({ host: "127.0.0.1", port: 8090 });
	});

	it("defaults to loopback:8090 and rejects wildcard hosts / bad ports", () => {
		expect(parseBindConfig({})).toEqual({ host: "127.0.0.1", port: 8090 });
		for (const env of [
			{ TCM_BIND_HOST: "0.0.0.0" },
			{ TCM_BIND_HOST: "::" },
			{ TCM_BIND_PORT: "0" },
			{ TCM_BIND_PORT: "70000" },
			{ TCM_BIND_PORT: "80x" },
		]) {
			expect(() => parseBindConfig(env)).toThrow();
		}
	});

	it("accepts only the three loopback host spellings", () => {
		for (const host of ["127.0.0.1", "localhost", "::1"]) {
			expect(parseBindConfig({ TCM_BIND_HOST: host })).toEqual({
				host,
				port: 8090,
			});
		}
	});

	it("rejects every non-loopback host, including wildcards and lookalikes", () => {
		for (const host of [
			"",
			"*",
			"0.0.0.0",
			"::",
			"[::]",
			"192.168.1.10",
			"10.0.0.5",
			"example.com",
			"localhost.evil.test",
			"127.0.0.2",
			"::ffff:127.0.0.1",
			"[::1]",
			"127.0.0.1:*",
		]) {
			expect(() => parseBindConfig({ TCM_BIND_HOST: host })).toThrow(/bind/i);
		}
	});
});

describe("frame fanout", () => {
	it("broadcasts every source frame to all consumers", async () => {
		const [a, b] = fanoutFrames(fakeReader(FRAMES), 2);
		const collect = async (source: AsyncIterable<CanFrame>): Promise<CanFrame[]> => {
			const out: CanFrame[] = [];
			for await (const frame of source) out.push(frame);
			return out;
		};
		const [left, right] = await Promise.all([collect(a), collect(b)]);
		expect(left).toEqual(FRAMES);
		expect(right).toEqual(FRAMES);
	});

	it("caps each subscriber queue at 256 frames (drop oldest) without affecting peers", async () => {
		const TOTAL = 300;
		const LIMIT = 256;
		const all = Array.from(
			{ length: TOTAL },
			(_, index): CanFrame => ({
				id: index,
				dlc: 0,
				data: new Uint8Array(0),
				bus: 0,
				rxMs: index,
			}),
		);
		const [stalled, active] = fanoutFrames(fakeReader(all), 2);

		// Only the active consumer is drained while the pump runs; the other
		// queue must saturate at LIMIT instead of growing without bound.
		const received: CanFrame[] = [];
		for await (const frame of active) received.push(frame);
		expect(received).toEqual(all);

		const delayed: CanFrame[] = [];
		for await (const frame of stalled) delayed.push(frame);
		expect(delayed.length).toBe(LIMIT);
		expect(delayed[0]!.rxMs).toBe(TOTAL - LIMIT);
		expect(delayed.at(-1)!.rxMs).toBe(TOTAL - 1);
	});
});

describe("Pi service composition", () => {
	let stopCalled = 0;

	afterEach(() => {
		stopCalled = 0;
	});

	it("wires reader -> bridge + recorder on the pinned loopback port and mounts no TX", async () => {
		const readerFactory = jest.fn((_iface: string, _opts: CanReaderOptions) =>
			fakeReader(FRAMES),
		);
		const bridgeListener: RequestListener = () => undefined;
		const bridgeFactory = jest.fn((_opts: BridgeOptions) => bridgeListener);
		const recorder = {
			path: "/var/lib/teslacanmodder/captures/capture.jsonl",
			provenance: { vehicle: "unknown", bus: [0, 1, 2], firmware: "unknown", date: "x" },
			write: jest.fn<Recorder["write"]>(async () => {}),
			record: jest.fn<Recorder["record"]>(async () => {}),
			stop: jest.fn<Recorder["stop"]>(async () => {}),
		} satisfies Recorder;
		const recorderFactory = jest.fn((_dir: string, _opts: RecorderOptions) => recorder);
		const serverFactory = jest.fn((_opts: PiServerOptions) => ({}) as Server);
		const listen = jest.fn(async (_server: Server, _host: string, _port: number) => {
			return async () => {
				stopCalled += 1;
			};
		});
		const transport: TransportFactory = () => {
			throw new Error("fake transport should not be opened by the composition test");
		};
		const txFactory = jest.fn(() => ({ enabled: false }));

		const now = () => 1234;
		const handle = await startPiService({
			env: { ...ENV, PI_CAN_TX: "1" },
			ifaces: ["can0"],
			transport,
			now,
			readerFactory,
			bridgeFactory,
			recorderFactory,
			serverFactory,
			listen,
			txFactory,
		});

		expect(readerFactory).toHaveBeenCalledTimes(1);
		const [iface, readerOpts] = readerFactory.mock.calls[0]!;
		expect(iface).toBe("can0");
		expect(readerOpts.transport).toBe(transport);
		expect(readerOpts.now).toBe(now);

		expect(bridgeFactory).toHaveBeenCalledTimes(1);
		expect(bridgeFactory.mock.calls[0]![0]!.token).toBe("service-token");
		expect(serverFactory).toHaveBeenCalledTimes(1);
		expect(serverFactory.mock.calls[0]![0]!.bridge).toBe(bridgeListener);

		expect(recorderFactory).toHaveBeenCalledWith(
			"/var/lib/teslacanmodder/captures",
			expect.anything(),
		);
		expect(listen).toHaveBeenCalledWith(expect.anything(), "127.0.0.1", 8090);

		await waitFor(() => recorder.write.mock.calls.length === FRAMES.length);
		expect(recorder.write.mock.calls.map((call) => call[0])).toEqual(FRAMES);

		// PI_CAN_TX=1 is present in env but TX capability is never wired.
		expect(txFactory).not.toHaveBeenCalled();

		await handle.stop();
		expect(stopCalled).toBe(1);
		expect(recorder.stop).toHaveBeenCalledTimes(1);
	});

	it("rejects a wildcard bind before composing anything", async () => {
		const readerFactory = jest.fn(() => fakeReader([]));
		await expect(
			startPiService({
				env: { ...ENV, TCM_BIND_HOST: "0.0.0.0" },
				readerFactory,
			}),
		).rejects.toThrow(/bind/i);
		expect(readerFactory).not.toHaveBeenCalled();
	});
});
