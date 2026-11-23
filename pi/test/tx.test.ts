import { existsSync } from "node:fs";
import { isAbsolute } from "node:path";
import { describe, expect, it, jest } from "@jest/globals";
import * as measure from "../src/rt/measure.js";
import {
	RT_GATE_SCHEMA_VERSION,
	RT_THRESHOLDS,
	signPayload,
	type RtGateArtifact,
	type RtGateArtifactPayload,
	type RtMeasurements,
} from "../src/rt/measure.js";
import { isCanTxEnabled } from "../src/index.js";
import {
	buildDriveModeFrame,
	createGatedTx,
	driveChecksum,
	encodeCanFrame,
	readGateArtifact,
	type CanTxBackend,
	type TxFrame,
	type TxMode,
	type TxTransport,
} from "../src/can/tx.js";

const PASSING: RtMeasurements = {
	cyclictestMaxUs: 95,
	rxtxP9999Us: 1800,
	rxtxWorstUs: 4200,
};

function artifactPayload(overrides: Partial<RtGateArtifactPayload> = {}): RtGateArtifactPayload {
	return {
		schemaVersion: RT_GATE_SCHEMA_VERSION,
		generatedAt: "2026-11-23T00:00:00.000Z",
		board: "Pi 5 8GB",
		kernel: "6.12-rt",
		method: {
			cyclictestArgs: "--mlockall --priority=80",
			loadGenerator: "stress-ng",
			durationMin: 60,
			probe: "rt-can-test",
		},
		measurements: PASSING,
		thresholds: RT_THRESHOLDS,
		verdict: { pass: true, failures: [] },
		...overrides,
	};
}

function signedArtifact(key = "test-key"): RtGateArtifact {
	const body = artifactPayload();
	return { ...body, signature: signPayload(body, key, "env:PI_RT_GATE_KEY") };
}

const OPEN_ENV = { PI_CAN_TX: "1", PI_RT_GATE_KEY: "test-key" } as NodeJS.ProcessEnv;

const resolveGateArtifactPath = (
	measure as unknown as {
		resolveGateArtifactPath?: (env?: NodeJS.ProcessEnv) => string;
	}
).resolveGateArtifactPath;

/** Fake transport that records every encoded frame handed to it. */
function fakeTransport(): TxTransport & { sent: Uint8Array[] } {
	const sent: Uint8Array[] = [];
	return {
		sent,
		async send(raw: Uint8Array): Promise<void> {
			sent.push(raw.slice());
		},
	};
}

const CACHED = new Uint8Array([0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00]);

// ── Independent oracles (do NOT import the implementation under test) ───────

/** Reference mode mapping + additive checksum, written from the firmware spec. */
function oracleFrame(mode: TxMode, cached: Uint8Array): TxFrame {
	const bits: Record<TxMode, number> = { chill: 0, standard: 1, performance: 2 };
	const data = Uint8Array.from(cached);
	data[0] = (data[0] & ~0x60) | (bits[mode] << 5);
	let sum = 0;
	for (let i = 0; i < 7; i += 1) sum = (sum + data[i]) & 0xff;
	data[7] = sum;
	return { id: 0x334, dlc: 8, data };
}

/** Reference 16-byte SocketCAN struct can_frame encoder (native little-endian). */
function oracleWire(frame: TxFrame): Uint8Array {
	const raw = new Uint8Array(16);
	const id = frame.id;
	raw[0] = id & 0xff;
	raw[1] = (id >> 8) & 0xff;
	raw[2] = (id >> 16) & 0xff;
	raw[3] = (id >> 24) & 0xff;
	raw[4] = frame.dlc;
	for (let i = 0; i < frame.dlc; i += 1) raw[8 + i] = frame.data[i];
	return raw;
}

describe("PI_CAN_TX feature flag (wired in src/index.ts)", () => {
	it("defaults to disabled for unset / 0 / non-literal values", () => {
		expect(isCanTxEnabled({})).toBe(false);
		expect(isCanTxEnabled({ PI_CAN_TX: "0" })).toBe(false);
		expect(isCanTxEnabled({ PI_CAN_TX: "true" })).toBe(false);
		expect(isCanTxEnabled({ PI_CAN_TX: "1" })).toBe(true);
	});
});

describe("frame builders — firmware conventions verified by independent oracle", () => {
	it.each<TxMode>(["chill", "standard", "performance"])(
		"buildDriveModeFrame(%s) matches the oracle bit-for-bit",
		(mode) => {
			expect(buildDriveModeFrame(mode, CACHED)).toEqual(oracleFrame(mode, CACHED));
		},
	);

	it("driveChecksum excludes the trailing checksum byte and wraps to 8 bits", () => {
		const sum = (0x11 + 0x22 + 0x33 + 0x44 + 0x55 + 0x66 + 0x77) & 0xff;
		expect(driveChecksum(CACHED, 8)).toBe(sum);
		expect(driveChecksum(new Uint8Array([0xff, 0x02, 0x00]), 3)).toBe(0x01);
	});

	it("encodeCanFrame matches the independent SocketCAN wire encoder", () => {
		const frame = oracleFrame("performance", CACHED);
		expect(encodeCanFrame(frame)).toEqual(oracleWire(frame));
	});

	it("buildDriveModeFrame rejects a short cached frame (DLC guard)", () => {
		expect(buildDriveModeFrame("standard", new Uint8Array(7))).toBeUndefined();
	});
});

describe("hard gate — TX unreachable unless BOTH flag and artifact are valid", () => {
	it("flag off: send is a no-op even with a valid artifact", async () => {
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: { PI_CAN_TX: "0", PI_RT_GATE_KEY: "test-key" },
			artifact: signedArtifact(),
			transport,
		});
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("flag unset: send is a no-op even with a valid artifact", async () => {
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: { PI_RT_GATE_KEY: "test-key" },
			artifact: signedArtifact(),
			transport,
		});
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("missing artifact: send is a no-op", async () => {
		const transport = fakeTransport();
		const backend = createGatedTx({ env: OPEN_ENV, transport });
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("default artifact loader fails closed when no rt-result.json exists", () => {
		expect(existsSync(new URL("../gate/rt-result.json", import.meta.url))).toBe(false);
		expect(readGateArtifact(new URL("../gate/rt-result.json", import.meta.url).pathname)).toBe(
			undefined,
		);
	});

	it("loads the pi-rooted absolute gate path by default; env and injected paths still win", () => {
		expect(typeof resolveGateArtifactPath).toBe("function");
		const paths: string[] = [];
		const readArtifact = (path: string) => {
			paths.push(path);
			return undefined;
		};

		createGatedTx({ env: OPEN_ENV, transport: fakeTransport(), readArtifact });
		expect(paths[0]).toBe(resolveGateArtifactPath!(OPEN_ENV));
		expect(isAbsolute(paths[0]!)).toBe(true);

		createGatedTx({
			env: { ...OPEN_ENV, PI_RT_GATE_ARTIFACT_PATH: "/tmp/rt.json" },
			transport: fakeTransport(),
			readArtifact,
		});
		expect(paths.at(-1)).toBe("/tmp/rt.json");

		createGatedTx({
			env: OPEN_ENV,
			gatePath: "/custom/rt.json",
			transport: fakeTransport(),
			readArtifact,
		});
		expect(paths.at(-1)).toBe("/custom/rt.json");
	});

	it("tampered artifact: send is a no-op", async () => {
		const tampered = signedArtifact();
		tampered.measurements = { ...PASSING, cyclictestMaxUs: 9999 };
		const transport = fakeTransport();
		const backend = createGatedTx({ env: OPEN_ENV, artifact: tampered, transport });
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("lying verdict: send is a no-op", async () => {
		const body = artifactPayload({ verdict: { pass: false, failures: ["rxtx-worst"] } });
		const artifact: RtGateArtifact = {
			...body,
			signature: signPayload(body, "test-key", "k"),
		};
		const transport = fakeTransport();
		const backend = createGatedTx({ env: OPEN_ENV, artifact, transport });
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("wrong signing key: send is a no-op", async () => {
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: { PI_CAN_TX: "1", PI_RT_GATE_KEY: "wrong-key" },
			artifact: signedArtifact("test-key"),
			transport,
		});
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});

	it("no transport configured: enabled stays false", async () => {
		const backend = createGatedTx({ env: OPEN_ENV, artifact: signedArtifact() });
		expect(backend.enabled).toBe(false);
		expect(await backend.send("standard", CACHED)).toBe(false);
	});

	it("short cached frame never reaches the transport", async () => {
		const transport = fakeTransport();
		const backend: CanTxBackend = createGatedTx({
			env: OPEN_ENV,
			artifact: signedArtifact(),
			transport,
		});
		expect(backend.enabled).toBe(true);
		expect(await backend.send("standard", new Uint8Array(7))).toBe(false);
		expect(transport.sent).toHaveLength(0);
	});
});

describe("reachability — only through an injected gate + transport", () => {
	it("real gate + signed artifact + fake transport: one correct frame is sent", async () => {
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: OPEN_ENV,
			artifact: signedArtifact(),
			transport,
		});
		expect(backend.enabled).toBe(true);
		expect(await backend.send("performance", CACHED)).toBe(true);
		expect(transport.sent).toHaveLength(1);
		expect(transport.sent[0]).toEqual(oracleWire(oracleFrame("performance", CACHED)));
	});

	it("injected fake gate + fake transport: send path is reached", async () => {
		const gate = jest.fn<() => boolean>(() => true);
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: OPEN_ENV,
			artifact: signedArtifact(),
			isGateOpenFn: gate,
			transport,
		});
		expect(backend.enabled).toBe(true);
		expect(await backend.send("chill", CACHED)).toBe(true);
		// Gate is checked at construction and re-checked inside send().
		expect(gate).toHaveBeenCalledTimes(2);
		expect(transport.sent).toHaveLength(1);
	});

	it("re-checks the gate per send: a gate that closes blocks the next frame", async () => {
		let open = true;
		const gate = jest.fn<() => boolean>(() => open);
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: OPEN_ENV,
			artifact: signedArtifact(),
			isGateOpenFn: gate,
			transport,
		});
		expect(await backend.send("chill", CACHED)).toBe(true);
		open = false;
		expect(await backend.send("chill", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(1);
		expect(gate).toHaveBeenCalledTimes(3);
	});

	it("re-checks the PI_CAN_TX flag per send", async () => {
		const env = { ...OPEN_ENV };
		const transport = fakeTransport();
		const backend = createGatedTx({ env, artifact: signedArtifact(), transport });
		expect(await backend.send("chill", CACHED)).toBe(true);
		env.PI_CAN_TX = "0";
		expect(await backend.send("chill", CACHED)).toBe(false);
		expect(transport.sent).toHaveLength(1);
	});

	it("injected fake gate returning false blocks the send", async () => {
		const gate = jest.fn<() => boolean>(() => false);
		const transport = fakeTransport();
		const backend = createGatedTx({
			env: OPEN_ENV,
			artifact: signedArtifact(),
			isGateOpenFn: gate,
			transport,
		});
		expect(backend.enabled).toBe(false);
		expect(await backend.send("chill", CACHED)).toBe(false);
		expect(gate).toHaveBeenCalledTimes(2);
		expect(transport.sent).toHaveLength(0);
	});
});
