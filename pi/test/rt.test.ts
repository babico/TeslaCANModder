import { existsSync } from "node:fs";
import { tmpdir } from "node:os";
import { isAbsolute } from "node:path";
import { describe, expect, it } from "@jest/globals";
import * as measure from "../src/rt/measure.js";
import {
	decideRtGate,
	isGateOpen,
	runRtMeasurement,
	RT_GATE_ARTIFACT_PATH,
	RT_GATE_SCHEMA_VERSION,
	RT_THRESHOLDS,
	signPayload,
	verifyGateArtifact,
	type RtArtifactWriter,
	type RtCommandRunner,
	type RtGateArtifact,
	type RtGateArtifactPayload,
	type RtMeasurements,
} from "../src/rt/measure.js";

/** Look up the path resolver dynamically so the RED test stays runnable. */
const resolveGateArtifactPath = (
	measure as unknown as {
		resolveGateArtifactPath?: (env?: NodeJS.ProcessEnv) => string;
	}
).resolveGateArtifactPath;

const envForMeasurement = (): NodeJS.ProcessEnv => ({
	PI_RT_MEASURE: "1",
	PI_RT_GATE_KEY: "test-key",
});

/** Known-passing measurements; tests override one field at a time. */
const PASSING: RtMeasurements = {
	cyclictestMaxUs: 95,
	rxtxP9999Us: 1800,
	rxtxWorstUs: 4200,
};

function withMeasurements(overrides: Partial<RtMeasurements>): RtMeasurements {
	return { ...PASSING, ...overrides };
}

function payload(overrides: Partial<RtGateArtifactPayload> = {}): RtGateArtifactPayload {
	return {
		schemaVersion: RT_GATE_SCHEMA_VERSION,
		generatedAt: "2026-11-23T00:00:00.000Z",
		board: "Pi 5 8GB",
		kernel: "6.12-rt",
		method: {
			cyclictestArgs: "--mlockall --priority=80 --interval=200",
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

function signed(key = "test-key"): RtGateArtifact {
	const body = payload();
	return { ...body, signature: signPayload(body, key, "env:PI_RT_GATE_KEY") };
}

class MemoryWriter implements RtArtifactWriter {
	readonly files = new Map<string, string>();

	async write(path: string, contents: string): Promise<void> {
		this.files.set(path, contents);
	}
}

/** Hardware-free fake: cyclictest text + probe JSON, both literal. */
const fakeRunner: RtCommandRunner = {
	async run(command) {
		if (command === "cyclictest") {
			return {
				code: 0,
				stdout: "T: 0 ( 1234) P:80 I:200 C:1800000 Min: 5 Act: 12 Avg: 14 Max: 95\n",
				stderr: "",
			};
		}
		if (command === "rt-can-test") {
			return { code: 0, stdout: '{"p9999Us":1800,"worstUs":4200}\n', stderr: "" };
		}
		throw new Error(`unexpected command: ${command}`);
	},
};

describe("RT gate decision — boundary values", () => {
	const cyclicCases: Array<[number, boolean]> = [
		[99, true],
		[100, false],
		[101, false],
	];
	it.each(cyclicCases)("cyclictest max %i us -> pass=%s", (us, pass) => {
		const decision = decideRtGate(withMeasurements({ cyclictestMaxUs: us }));
		expect(decision.verdict.pass).toBe(pass);
		expect(decision.verdict.failures).toEqual(pass ? [] : ["cyclictest"]);
	});

	// 1.9 / 2.0 / 2.1 ms expressed in the function's microsecond unit.
	const p9999Cases: Array<[number, boolean]> = [
		[1900, true],
		[2000, false],
		[2100, false],
	];
	it.each(p9999Cases)("RX->TX p99.99 %i us (2 ms bound) -> pass=%s", (us, pass) => {
		const decision = decideRtGate(withMeasurements({ rxtxP9999Us: us }));
		expect(decision.verdict.pass).toBe(pass);
		expect(decision.verdict.failures).toEqual(pass ? [] : ["rxtx-p9999"]);
	});

	// 4.9 / 5.0 / 5.1 ms expressed in the function's microsecond unit.
	const worstCases: Array<[number, boolean]> = [
		[4900, true],
		[5000, false],
		[5100, false],
	];
	it.each(worstCases)("RX->TX worst %i us (5 ms bound) -> pass=%s", (us, pass) => {
		const decision = decideRtGate(withMeasurements({ rxtxWorstUs: us }));
		expect(decision.verdict.pass).toBe(pass);
		expect(decision.verdict.failures).toEqual(pass ? [] : ["rxtx-worst"]);
	});

	it("states the binding thresholds explicitly", () => {
		expect(RT_THRESHOLDS).toEqual({
			cyclictestMaxUs: 100,
			rxtxP9999Us: 2000,
			rxtxWorstUs: 5000,
		});
	});

	it("all three metrics must pass", () => {
		const decision = decideRtGate(
			withMeasurements({ cyclictestMaxUs: 100, rxtxP9999Us: 2000, rxtxWorstUs: 5000 }),
		);
		expect(decision.verdict.pass).toBe(false);
		expect(decision.verdict.failures).toEqual(["cyclictest", "rxtx-p9999", "rxtx-worst"]);
	});

	it("fails closed on invalid measurements", () => {
		for (const bad of [Number.NaN, Number.POSITIVE_INFINITY, -1]) {
			expect(decideRtGate(withMeasurements({ cyclictestMaxUs: bad })).verdict.pass).toBe(
				false,
			);
			expect(decideRtGate(withMeasurements({ rxtxP9999Us: bad })).verdict.pass).toBe(false);
			expect(decideRtGate(withMeasurements({ rxtxWorstUs: bad })).verdict.pass).toBe(false);
		}
	});
});

describe("RT gate artifact signature", () => {
	it("round-trips a valid signature", () => {
		expect(verifyGateArtifact(signed(), "test-key")).toBe(true);
	});

	it("rejects a tampered measurement, wrong key and old schema", () => {
		const tampered = signed();
		tampered.measurements = { ...PASSING, cyclictestMaxUs: 9999 };
		expect(verifyGateArtifact(tampered, "test-key")).toBe(false);
		expect(verifyGateArtifact(signed(), "wrong-key")).toBe(false);
		expect(verifyGateArtifact(signed(), "test-key", 999)).toBe(false);
	});
});

describe("gate open for TX", () => {
	it("opens only for a valid, passing, self-consistent artifact", () => {
		expect(isGateOpen(signed(), "test-key")).toBe(true);

		const failing = payload({ verdict: { pass: false, failures: ["rxtx-worst"] } });
		expect(
			isGateOpen(
				{ ...failing, signature: signPayload(failing, "test-key", "k") },
				"test-key",
			),
		).toBe(false);

		// Claimed pass but measurements actually fail: re-decision must catch it.
		const lying = payload({
			measurements: withMeasurements({ cyclictestMaxUs: 250 }),
		});
		expect(
			isGateOpen({ ...lying, signature: signPayload(lying, "test-key", "k") }, "test-key"),
		).toBe(false);

		// A signed artifact missing measurements must fail closed, not throw.
		const brokenBody = {
			...payload(),
			measurements: undefined,
		} as unknown as RtGateArtifactPayload;
		const broken = { ...brokenBody, signature: signPayload(brokenBody, "test-key", "k") };
		expect(isGateOpen(broken, "test-key")).toBe(false);
	});

	it("requires a measurement duration of at least 60 minutes", () => {
		const shortBody = payload({
			method: {
				cyclictestArgs: "--mlockall --priority=80",
				loadGenerator: "stress-ng",
				durationMin: 30,
				probe: "rt-can-test",
			},
		});
		const short = { ...shortBody, signature: signPayload(shortBody, "test-key", "k") };
		expect(isGateOpen(short, "test-key")).toBe(false);

		const nanBody = payload({
			method: { ...payload().method, durationMin: Number.NaN },
		});
		const nan = { ...nanBody, signature: signPayload(nanBody, "test-key", "k") };
		expect(isGateOpen(nan, "test-key")).toBe(false);
	});
});

describe("RT measurement command skeleton (no hardware)", () => {
	it("refuses to run or write without explicit opt-in", async () => {
		const writer = new MemoryWriter();
		await expect(runRtMeasurement({ env: {}, writer, runner: fakeRunner })).rejects.toThrow(
			/disabled/i,
		);
		expect(writer.files.size).toBe(0);
	});

	it("requires an injected runner; nothing executes on import", async () => {
		await expect(
			runRtMeasurement({
				enable: true,
				env: { PI_RT_MEASURE: "1", PI_RT_GATE_KEY: "test-key" },
				writer: new MemoryWriter(),
			}),
		).rejects.toThrow(/runner/i);
	});

	it("writes one signed artifact through injected runner and writer", async () => {
		const writer = new MemoryWriter();
		const result = await runRtMeasurement({
			enable: true,
			env: envForMeasurement(),
			runner: fakeRunner,
			writer,
			now: () => "2026-11-23T01:02:03.000Z",
		});

		const expectedPath =
			resolveGateArtifactPath?.(envForMeasurement()) ?? RT_GATE_ARTIFACT_PATH;
		expect([...writer.files.keys()]).toEqual([expectedPath]);
		const artifact = JSON.parse(writer.files.get(expectedPath)!) as RtGateArtifact;
		expect(artifact.measurements).toEqual(PASSING);
		expect(artifact.verdict.pass).toBe(true);
		expect(artifact.generatedAt).toBe("2026-11-23T01:02:03.000Z");
		expect(verifyGateArtifact(artifact, "test-key")).toBe(true);
		expect(isGateOpen(artifact, "test-key")).toBe(true);
		expect(result.decision.verdict.pass).toBe(true);
	});

	it("no rt-result.json artifact exists in this task", () => {
		expect(existsSync(new URL("../gate/rt-result.json", import.meta.url))).toBe(false);
	});
});

describe("gate artifact path resolution", () => {
	it("defaults to an absolute path rooted at the pi/ package", () => {
		expect(typeof resolveGateArtifactPath).toBe("function");
		const path = resolveGateArtifactPath!({});
		expect(isAbsolute(path)).toBe(true);
		expect(path.replace(/\\/g, "/")).toMatch(/\/pi\/gate\/rt-result\.json$/);
	});

	it("returns the same path from a different working directory", () => {
		const expected = resolveGateArtifactPath!({});
		const original = process.cwd();
		try {
			process.chdir(tmpdir());
			expect(resolveGateArtifactPath!({})).toBe(expected);
		} finally {
			process.chdir(original);
		}
	});

	it("honors an explicit environment override for tests", () => {
		const custom = "/tmp/custom-rt-result.json";
		expect(resolveGateArtifactPath!({ PI_RT_GATE_ARTIFACT_PATH: custom })).toBe(custom);
	});
});
