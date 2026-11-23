import { readFileSync } from "node:fs";

import { isCanTxEnabled } from "../index.js";
import { isGateOpen, RT_GATE_ARTIFACT_PATH, type RtGateArtifact } from "../rt/measure.js";

/**
 * Gated Linux CAN TX backend.
 *
 * DISABLED BY DEFAULT. Importing this module touches no hardware, opens no
 * socket and reads no file. Transmission is reachable only when BOTH the
 * `PI_CAN_TX=1` feature flag and B3's `isGateOpen()` gate artifact check pass;
 * either one missing or invalid leaves the backend inert.
 *
 * No gate artifact exists in this repo, no bench capture was available and no
 * vehicle acceptance is claimed here. Frame layout/checksum rules mirror this
 * repo's firmware (`firmware/lib/vehicle/can/ids.h:driveChecksum` and
 * `firmware/lib/vehicle/can/feature/drive/drive_mode.h:buildDriveModeFrame`).
 * Real injection stays a release blocker until a signed gate artifact and a
 * listen-only capture reference exist. No Sentry code is used.
 */

/** Vehicle-bus drive-config frame ID (firmware `CAN_ID_DRIVE_CONFIG`). */
export const CAN_ID_DRIVE_MODE = 0x334;

export type TxMode = "chill" | "standard" | "performance";

/** A fully built CAN frame, before SocketCAN serialization. */
export interface TxFrame {
	id: number;
	dlc: number;
	data: Uint8Array;
}

/** Sink for encoded `struct can_frame` bytes. Injected in tests; no default socket. */
export interface TxTransport {
	send(raw: Uint8Array): Promise<void>;
}

/** Fail-closed artifact loader; returns `undefined` on any read/parse error. */
export type GateArtifactReader = (path: string) => RtGateArtifact | undefined;

/** Gate predicate, injectable so tests can prove reachability without an artifact. */
export type GateCheck = (artifact: RtGateArtifact, key: string) => boolean;

export interface GatedTxOptions {
	/** Environment holding `PI_CAN_TX` and `PI_RT_GATE_KEY`. Defaults to `process.env`. */
	env?: NodeJS.ProcessEnv;
	/** Explicit artifact injection. Pass `undefined` to simulate a missing file. */
	artifact?: RtGateArtifact | undefined;
	/** HMAC key; defaults to `env.PI_RT_GATE_KEY`. */
	gateKey?: string;
	/** Artifact path; defaults to `pi/gate/rt-result.json`. */
	gatePath?: string;
	/** Artifact loader; defaults to `readGateArtifact`. */
	readArtifact?: GateArtifactReader;
	/** Gate predicate; defaults to B3's `isGateOpen()`. */
	isGateOpenFn?: GateCheck;
	/** Encoded-frame sink. Without one the backend is disabled. */
	transport?: TxTransport;
}

export interface CanTxBackend {
	/** True only when flag, gate and transport all resolved open at construction. */
	readonly enabled: boolean;
	/** Build and send one frame; no-op returning false whenever the gate is shut. */
	send(mode: TxMode, cachedFrame: Uint8Array): Promise<boolean>;
}

/** Additive trailing checksum from firmware `driveChecksum()`: sum of bytes 0..len-2. */
export function driveChecksum(data: Uint8Array, len: number): number {
	let sum = 0;
	for (let i = 0; i < len - 1; i += 1) sum = (sum + data[i]) & 0xff;
	return sum;
}

/**
 * Build the 0x334 drive-config injection frame using the firmware convention:
 * copy the cached 8-byte frame, write the 2-bit mode into byte[0] bits[6:5],
 * recompute the additive checksum in byte[7]. No cached frame (DLC < 8) means
 * no frame is produced.
 */
export function buildDriveModeFrame(mode: TxMode, cachedFrame: Uint8Array): TxFrame | undefined {
	if (cachedFrame.length < 8) return undefined;
	const data = Uint8Array.from(cachedFrame.subarray(0, 8));
	const modeBits: Record<TxMode, number> = { chill: 0, standard: 1, performance: 2 };
	data[0] = (data[0] & ~0x60) | ((modeBits[mode] & 0x03) << 5);
	data[7] = driveChecksum(data, 8);
	return { id: CAN_ID_DRIVE_MODE, dlc: 8, data };
}

/** Serialize a frame into the 16-byte native-endian SocketCAN `struct can_frame`. */
export function encodeCanFrame(frame: TxFrame): Uint8Array {
	if (!Number.isInteger(frame.id) || frame.id < 0 || frame.id > 0x7ff) {
		throw new Error(`unsupported CAN id: ${frame.id} (11-bit frames only)`);
	}
	if (!Number.isInteger(frame.dlc) || frame.dlc < 0 || frame.dlc > 8) {
		throw new Error(`invalid can_dlc: ${frame.dlc}`);
	}
	const raw = new Uint8Array(16);
	new DataView(raw.buffer).setUint32(0, frame.id, true);
	raw[4] = frame.dlc;
	raw.set(frame.data.subarray(0, frame.dlc), 8);
	return raw;
}

/** Read and parse the B3 artifact lazily; never throws, never logs a key. */
export function readGateArtifact(path: string): RtGateArtifact | undefined {
	try {
		return JSON.parse(readFileSync(path, "utf8")) as RtGateArtifact;
	} catch {
		return undefined;
	}
}

/**
 * Construct the gated TX backend. `enabled` is a construction-time snapshot,
 * but the hard gate is RE-EVALUATED on every `send()`: `PI_CAN_TX === "1"` AND
 * a valid, passing, self-consistent gate artifact AND an injected transport.
 * A gate or flag that closes after construction blocks subsequent frames — no
 * cached "open" decision can authorize a transmission. Any failure makes the
 * `send()` a no-op returning false that never touches the transport.
 */
export function createGatedTx(options: GatedTxOptions = {}): CanTxBackend {
	const env = options.env ?? process.env;
	const transport = options.transport;
	const gateCheck = options.isGateOpenFn ?? isGateOpen;
	const gateKey = options.gateKey ?? env.PI_RT_GATE_KEY ?? "";
	const gatePath = options.gatePath ?? RT_GATE_ARTIFACT_PATH;
	const load = options.readArtifact ?? readGateArtifact;

	function gateOpen(): boolean {
		if (!isCanTxEnabled(env) || transport === undefined) return false;
		const artifact = "artifact" in options ? options.artifact : load(gatePath);
		return artifact !== undefined && gateCheck(artifact, gateKey);
	}

	const enabled = gateOpen();

	return {
		enabled,
		async send(mode: TxMode, cachedFrame: Uint8Array): Promise<boolean> {
			if (!gateOpen()) return false;
			const frame = buildDriveModeFrame(mode, cachedFrame);
			if (!frame) return false;
			await transport!.send(encodeCanFrame(frame));
			return true;
		},
	};
}
