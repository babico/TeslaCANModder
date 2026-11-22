import { Buffer } from "node:buffer";
import { createHmac, timingSafeEqual } from "node:crypto";

/**
 * RT measurement gate — pure go/no-go decision plus a hardware-free command
 * skeleton. Importing this module never touches hardware or writes a file.
 *
 * No artifact exists in this repo: producing `pi/gate/rt-result.json` requires
 * a real PREEMPT_RT board, an injected runner/writer and the explicit opt-ins
 * enforced in `runRtMeasurement()`. CAN TX stays disabled until C1 consumes a
 * valid artifact via `isGateOpen()`.
 */

/** All three measurements use microseconds internally; thresholds mirror them. */
export interface RtMeasurements {
	/** cyclictest maximum latency, µs (strictly < 100 to pass). */
	cyclictestMaxUs: number;
	/** RX→TX probe p99.99 latency, µs (strictly < 2000 = 2 ms to pass). */
	rxtxP9999Us: number;
	/** RX→TX probe worst-case latency, µs (strictly < 5000 = 5 ms to pass). */
	rxtxWorstUs: number;
}

export interface RtThresholds {
	cyclictestMaxUs: number;
	rxtxP9999Us: number;
	rxtxWorstUs: number;
}

/** Binding thresholds from the ADR §6 / B3 brief — strict less-than on each. */
export const RT_THRESHOLDS: Readonly<RtThresholds> = Object.freeze({
	cyclictestMaxUs: 100,
	rxtxP9999Us: 2_000,
	rxtxWorstUs: 5_000,
});

export type RtFailureCode = "cyclictest" | "rxtx-p9999" | "rxtx-worst";

export interface RtVerdict {
	pass: boolean;
	failures: RtFailureCode[];
}

export interface RtGateDecision {
	measurements: RtMeasurements;
	thresholds: RtThresholds;
	verdict: RtVerdict;
}

/** A measurement counts only when finite, non-negative and strictly below its bound. */
function below(value: number, bound: number): boolean {
	return Number.isFinite(value) && value >= 0 && value < bound;
}

/** Pure decision — no I/O, no clock, no hardware. Returns a copied, immutable-shaped result. */
export function decideRtGate(
	measurements: RtMeasurements,
	thresholds: Readonly<RtThresholds> = RT_THRESHOLDS,
): RtGateDecision {
	const failures: RtFailureCode[] = [];
	if (!below(measurements.cyclictestMaxUs, thresholds.cyclictestMaxUs))
		failures.push("cyclictest");
	if (!below(measurements.rxtxP9999Us, thresholds.rxtxP9999Us)) failures.push("rxtx-p9999");
	if (!below(measurements.rxtxWorstUs, thresholds.rxtxWorstUs)) failures.push("rxtx-worst");
	return {
		measurements: { ...measurements },
		thresholds: { ...thresholds },
		verdict: { pass: failures.length === 0, failures },
	};
}

/** v1 gate artifact contract, consumed by C1's TX backend. */
export const RT_GATE_SCHEMA_VERSION = 1;
export const RT_GATE_ARTIFACT_PATH = "pi/gate/rt-result.json";

export interface RtGateArtifactPayload {
	schemaVersion: number;
	generatedAt: string;
	board: string;
	kernel: string;
	method: {
		cyclictestArgs: string;
		loadGenerator: string;
		durationMin: number;
		probe: string;
	};
	measurements: RtMeasurements;
	thresholds: RtThresholds;
	verdict: RtVerdict;
}

export interface RtGateSignature {
	alg: "HMAC-SHA256";
	/** Identifier for the key source — never the secret itself. */
	keyId: string;
	/** Lowercase hex HMAC over the canonical JSON of the payload without this field. */
	value: string;
}

export interface RtGateArtifact extends RtGateArtifactPayload {
	signature: RtGateSignature;
}

/** Deterministic serialization so signer and verifier agree regardless of key order. */
function canonicalJson(value: unknown): string {
	if (Array.isArray(value)) return `[${value.map(canonicalJson).join(",")}]`;
	if (value !== null && typeof value === "object") {
		const entries = Object.entries(value as Record<string, unknown>)
			.filter(([, item]) => item !== undefined)
			.sort(([left], [right]) => (left < right ? -1 : left > right ? 1 : 0));
		return `{${entries
			.map(([key, item]) => `${JSON.stringify(key)}:${canonicalJson(item)}`)
			.join(",")}}`;
	}
	return JSON.stringify(value) ?? "null";
}

export function signPayload(
	payload: RtGateArtifactPayload,
	key: string,
	keyId: string,
): RtGateSignature {
	if (!key) throw new Error("RT gate signing key is empty");
	const value = createHmac("sha256", key).update(canonicalJson(payload)).digest("hex");
	return { alg: "HMAC-SHA256", keyId, value };
}

function constantHexEqual(left: string, right: string): boolean {
	const a = Buffer.from(left, "hex");
	const b = Buffer.from(right, "hex");
	return a.length > 0 && a.length === b.length && timingSafeEqual(a, b);
}

function thresholdsEqual(left: RtThresholds, right: Readonly<RtThresholds>): boolean {
	return (
		left?.cyclictestMaxUs === right.cyclictestMaxUs &&
		left?.rxtxP9999Us === right.rxtxP9999Us &&
		left?.rxtxWorstUs === right.rxtxWorstUs
	);
}

/** Verify signature, schema and binding thresholds. Does not itself open TX. */
export function verifyGateArtifact(
	artifact: RtGateArtifact,
	key: string,
	expectedSchema: number = RT_GATE_SCHEMA_VERSION,
): boolean {
	if (!key || artifact?.schemaVersion !== expectedSchema) return false;
	if (artifact.signature?.alg !== "HMAC-SHA256") return false;
	if (typeof artifact.signature.value !== "string") return false;
	if (!thresholdsEqual(artifact.thresholds, RT_THRESHOLDS)) return false;
	const { signature: _signature, ...body } = artifact;
	const expected = signPayload(body, key, artifact.signature.keyId);
	return (
		expected.keyId === artifact.signature.keyId &&
		constantHexEqual(expected.value, artifact.signature.value)
	);
}

/**
 * Single check C1 must call before enabling TX: valid signature, a passing
 * verdict, and measurements that independently re-pass the binding thresholds.
 */
export function isGateOpen(artifact: RtGateArtifact, key: string): boolean {
	if (!verifyGateArtifact(artifact, key)) return false;
	if (!artifact.verdict?.pass) return false;
	return decideRtGate(artifact.measurements).verdict.pass;
}

/** cyclictest args: mlockall, FIFO priority and an isolated-core-friendly interval. */
export function buildCyclictestArgs(durationMin: number): string[] {
	return [
		"--mlockall",
		"--smp",
		"--priority=80",
		"--interval=200",
		"--distance=0",
		`--duration=${Math.max(1, Math.floor(durationMin))}m`,
	];
}

/** Parse the cyclictest summary line's `Max:` field (microseconds). */
export function parseCyclictestMaxUs(stdout: string): number {
	const match = /Max:\s*(\d+(?:\.\d+)?)/.exec(stdout);
	if (!match) throw new Error("cyclictest output has no Max: value");
	const value = Number(match[1]);
	if (!Number.isFinite(value)) throw new Error("cyclictest Max value is not finite");
	return value;
}

export interface RxTxProbeStats {
	p9999Us: number;
	worstUs: number;
}

/** Parse the RX→TX probe's single-line JSON result. */
export function parseProbeStats(stdout: string): RxTxProbeStats {
	const line = stdout
		.trim()
		.split(/\r?\n/)
		.find((candidate) => candidate.startsWith("{"));
	if (!line) throw new Error("RX->TX probe output has no JSON object");
	const parsed = JSON.parse(line) as Partial<RxTxProbeStats>;
	if (typeof parsed.p9999Us !== "number" || typeof parsed.worstUs !== "number") {
		throw new Error("RX->TX probe JSON missing p9999Us/worstUs");
	}
	return { p9999Us: parsed.p9999Us, worstUs: parsed.worstUs };
}

export interface RtCommandResult {
	code: number | null;
	stdout: string;
	stderr: string;
}

/** Hardware-bound runner is injected; no default executes a process. */
export interface RtCommandRunner {
	run(command: string, args: string[]): Promise<RtCommandResult>;
}

export interface RtArtifactWriter {
	write(path: string, contents: string): Promise<void>;
}

export interface RtMeasurementOptions {
	/** Explicit opt-in. Without `true` nothing runs and nothing is written. */
	enable?: boolean;
	env?: NodeJS.ProcessEnv;
	runner?: RtCommandRunner;
	writer?: RtArtifactWriter;
	cyclictestArgs?: string[];
	probeCommand?: { command: string; args: string[] };
	artifactPath?: string;
	board?: string;
	kernel?: string;
	durationMin?: number;
	loadGenerator?: string;
	key?: string;
	keyId?: string;
	now?: () => string;
}

export interface RtMeasurementResult {
	artifact: RtGateArtifact;
	decision: RtGateDecision;
}

/**
 * Measurement command skeleton. Requires: `enable: true`, `PI_RT_MEASURE=1`,
 * `PI_RT_GATE_KEY`, an injected runner and an injected writer. All defaults
 * fail closed so importing/calling this on a dev machine produces no artifact.
 */
export async function runRtMeasurement(
	options: RtMeasurementOptions = {},
): Promise<RtMeasurementResult> {
	const env = options.env ?? process.env;
	if (options.enable !== true || env.PI_RT_MEASURE !== "1") {
		throw new Error("RT measurement disabled: requires enable=true and PI_RT_MEASURE=1");
	}
	const key = options.key ?? env.PI_RT_GATE_KEY;
	if (!key) throw new Error("RT gate signing key missing: set PI_RT_GATE_KEY");
	const runner = options.runner;
	if (!runner) throw new Error("No RT command runner configured");
	const writer = options.writer;
	if (!writer) throw new Error("No RT artifact writer configured");

	const durationMin = options.durationMin ?? 60;
	const cyclictestArgs = options.cyclictestArgs ?? buildCyclictestArgs(durationMin);
	const cyclictest = await runner.run("cyclictest", cyclictestArgs);
	if (cyclictest.code !== 0) {
		throw new Error(`cyclictest exited ${cyclictest.code}: ${cyclictest.stderr}`);
	}

	const probe = options.probeCommand ?? { command: "rt-can-test", args: ["--duration", "60"] };
	const probeResult = await runner.run(probe.command, probe.args);
	if (probeResult.code !== 0) {
		throw new Error(`RX->TX probe exited ${probeResult.code}: ${probeResult.stderr}`);
	}

	// Parse the probe once so both percentiles come from the same sample set.
	const stats = parseProbeStats(probeResult.stdout);
	const measurements: RtMeasurements = {
		cyclictestMaxUs: parseCyclictestMaxUs(cyclictest.stdout),
		rxtxP9999Us: stats.p9999Us,
		rxtxWorstUs: stats.worstUs,
	};

	const decision = decideRtGate(measurements);
	const body: RtGateArtifactPayload = {
		schemaVersion: RT_GATE_SCHEMA_VERSION,
		generatedAt: (options.now ?? (() => new Date().toISOString()))(),
		board: options.board ?? env.PI_RT_BOARD ?? "UNVERIFIED",
		kernel: options.kernel ?? env.PI_RT_KERNEL ?? "UNVERIFIED",
		method: {
			cyclictestArgs: cyclictestArgs.join(" "),
			loadGenerator: options.loadGenerator ?? env.PI_RT_LOAD ?? "UNVERIFIED",
			durationMin,
			probe: probe.command,
		},
		measurements: decision.measurements,
		thresholds: decision.thresholds,
		verdict: decision.verdict,
	};
	const signature = signPayload(
		body,
		key,
		options.keyId ?? env.PI_RT_GATE_KEY_ID ?? "env:PI_RT_GATE_KEY",
	);
	const artifact: RtGateArtifact = { ...body, signature };
	await writer.write(
		options.artifactPath ?? RT_GATE_ARTIFACT_PATH,
		`${JSON.stringify(artifact, null, 2)}\n`,
	);
	return { artifact, decision };
}
