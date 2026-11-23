import { createConnection, type Socket } from "node:net";
import { getCanTransport, type CanTransport } from "../index.js";

/** Chassis / vehicle / body bus index (X179: can0 / can1 / can2). */
export type BusId = 0 | 1 | 2;

/** Read-only CAN frame emitted by the Pi reader and consumed by A3/A4. */
export interface CanFrame {
	id: number;
	dlc: number;
	data: Uint8Array;
	bus: BusId;
	rxMs: number;
}

/** Source of raw SocketCAN `struct can_frame` byte chunks (may be split or coalesced). */
export interface FrameTransport {
	frames(): AsyncIterable<Uint8Array>;
	close?(): void;
}

/** Factory so tests inject a fake transport instead of touching a SocketCAN socket. */
export type TransportFactory = (iface: string, backend: CanTransport) => FrameTransport;

/** JSON shape forwarded to the command bridge (matches the firmware serial contract). */
export interface WireFrame {
	t: "frame";
	dir: "rx";
	bus: BusId;
	id: number;
	dlc: number;
	d: string;
}

export interface CanReaderOptions {
	/**
	 * Override the raw frame source. Defaults to the OPTIONAL TCP gateway client
	 * for backward compatibility; production composes the `candump` factory
	 * from `candump.ts` instead.
	 */
	transport?: TransportFactory;
	/** Clock used for `rxMs`. Defaults to `Date.now`. */
	now?: () => number;
	/** Reconnect backoff hook. Defaults to a setTimeout promise. */
	sleep?: (ms: number) => Promise<void>;
	/** Backoff after a transport error. Defaults to 250 ms. */
	reconnectDelayMs?: number;
	/** Max reconnects before surfacing the error. Defaults to unlimited. */
	maxReconnects?: number;
	/** CAN backend from A1's `CAN_TRANSPORT`. Defaults to `getCanTransport()`. */
	backend?: CanTransport;
}

/** Linux `struct can_frame`: uint32 id, uint8 dlc, 3 pad, uint8 data[8]. */
export const CAN_FRAME_BYTES = 16;
const CAN_SFF_MASK = 0x7ff;
const CAN_MAX_DLC = 8;
const CAN_EFF_FLAG = 0x80000000;
const CAN_RTR_FLAG = 0x40000000;
const CAN_ERR_FLAG = 0x20000000;
const CAN_ANY_FLAGS = CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG;
const EMPTY = new Uint8Array(0);

/**
 * Map can0/can1/can2 (and their vcanN loopback aliases used by the `vcan`
 * backend) to bus 0/1/2. Unknown interfaces are rejected.
 */
export function busFromIface(iface: string): BusId {
	const match = /^v?can([012])$/.exec(iface);
	if (!match) {
		throw new Error(`unsupported CAN interface "${iface}" (expected can0, can1, or can2)`);
	}
	return Number(match[1]) as BusId;
}

/** Parse one 16-byte SocketCAN can_frame into a CanFrame. */
export function parseCanFrame(raw: Uint8Array, bus: BusId, rxMs: number): CanFrame {
	if (raw.length !== CAN_FRAME_BYTES) {
		throw new Error(`invalid can_frame length: ${raw.length} (expected ${CAN_FRAME_BYTES})`);
	}
	const canId = new DataView(raw.buffer, raw.byteOffset, raw.byteLength).getUint32(0, true);
	if ((canId & CAN_ANY_FLAGS) !== 0) {
		throw new Error(
			`unsupported CAN frame flags in id 0x${canId.toString(16)} (11-bit frames only)`,
		);
	}
	const id = canId & CAN_SFF_MASK;
	const dlc = raw[4];
	if (dlc > CAN_MAX_DLC) {
		throw new Error(`invalid can_dlc: ${dlc}`);
	}
	return { id, dlc, data: raw.slice(8, 8 + dlc), bus, rxMs };
}

/** Convert a CanFrame to the firmware-compatible `{t:"frame"}` wire object. */
export function frameToWire(frame: CanFrame): WireFrame {
	return {
		t: "frame",
		dir: "rx",
		bus: frame.bus,
		id: frame.id,
		dlc: frame.dlc,
		d: Buffer.from(frame.data).toString("hex"),
	};
}

/** Single-line JSON for the command bridge. */
export function frameToWireJson(frame: CanFrame): string {
	return JSON.stringify(frameToWire(frame));
}

const defaultSleep = (ms: number): Promise<void> =>
	new Promise((resolve) => setTimeout(resolve, ms));

function concatBytes(left: Uint8Array, right: Uint8Array): Uint8Array {
	const merged = new Uint8Array(left.length + right.length);
	merged.set(left, 0);
	merged.set(right, left.length);
	return merged;
}

/** Parse `CAN_GATEWAY_PORT` strictly: integer 1..65535, fail fast on anything else. */
export function parseGatewayPort(env: NodeJS.ProcessEnv = process.env): number {
	const raw = env.CAN_GATEWAY_PORT ?? "3290";
	if (!/^\d+$/.test(raw)) {
		throw new Error(`invalid CAN_GATEWAY_PORT "${raw}": expected an integer in 1..65535`);
	}
	const port = Number(raw);
	if (!Number.isInteger(port) || port < 1 || port > 65535) {
		throw new Error(`invalid CAN_GATEWAY_PORT "${raw}": expected an integer in 1..65535`);
	}
	return port;
}

/**
 * OPTIONAL, UNSHIPPED TCP gateway client. Reads raw can_frame structs from a
 * local gateway that streams one SocketCAN interface per TCP port (base
 * `CAN_GATEWAY_PORT`, plus bus index). No such gateway ships in this repo and
 * the production service uses the `candump` transport instead (`candump.ts`).
 * Kept only as an injectable seam for bench experiments; never call this "the
 * SocketCAN read path". Bad port configuration fails fast at construction.
 */
export function createNetTransport(
	iface: string,
	_backend: CanTransport,
	env: NodeJS.ProcessEnv = process.env,
): FrameTransport {
	const bus = busFromIface(iface);
	const host = env.CAN_GATEWAY_HOST ?? "127.0.0.1";
	const basePort = parseGatewayPort(env);
	if (basePort + bus > 65535) {
		throw new Error(`invalid CAN_GATEWAY_PORT: base ${basePort} + bus ${bus} exceeds 65535`);
	}
	let active: Socket | undefined;

	return {
		async *frames(): AsyncGenerator<Uint8Array> {
			const socket = createConnection({ host, port: basePort + bus });
			active = socket;
			try {
				for await (const chunk of socket as AsyncIterable<Buffer>) {
					yield new Uint8Array(chunk.buffer, chunk.byteOffset, chunk.byteLength);
				}
			} finally {
				socket.destroy();
				active = undefined;
			}
		},
		close: () => active?.destroy(),
	};
}

/**
 * Read-only async iterable of CAN frames from one SocketCAN interface.
 * Transport errors trigger a reconnect (and backoff); a clean stream end ends
 * the reader. Frame parsing is pure and independent of the transport.
 */
export function createCanReader(
	iface: string,
	options: CanReaderOptions = {},
): AsyncIterable<CanFrame> {
	const bus = busFromIface(iface);
	const factory = options.transport ?? createNetTransport;
	const now = options.now ?? Date.now;
	const sleep = options.sleep ?? defaultSleep;
	const reconnectDelayMs = options.reconnectDelayMs ?? 250;
	const maxReconnects = options.maxReconnects ?? Number.POSITIVE_INFINITY;
	const backend = options.backend ?? getCanTransport();

	async function* iterate(): AsyncGenerator<CanFrame> {
		let reconnects = 0;
		// Hoisted outside the reconnect loop: a TCP/candump stream can split a
		// frame across a process/link bounce, so partial bytes survive reconnects.
		let carry = EMPTY;
		while (true) {
			const transport = factory(iface, backend);
			try {
				for await (const chunk of transport.frames()) {
					if (chunk.length === 0) continue;
					const merged = carry.length === 0 ? chunk : concatBytes(carry, chunk);
					let offset = 0;
					while (merged.length - offset >= CAN_FRAME_BYTES) {
						yield parseCanFrame(
							merged.subarray(offset, offset + CAN_FRAME_BYTES),
							bus,
							now(),
						);
						offset += CAN_FRAME_BYTES;
					}
					carry = offset === merged.length ? EMPTY : merged.slice(offset);
				}
				return;
			} catch (error) {
				reconnects += 1;
				if (reconnects > maxReconnects) throw error;
				await sleep(reconnectDelayMs);
			} finally {
				transport.close?.();
			}
		}
	}

	return iterate();
}
