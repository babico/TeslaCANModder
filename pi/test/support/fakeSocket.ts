import type { CanTransport } from "../../src/index.js";
import type { FrameTransport, TransportFactory } from "../../src/can/socketcan.js";

/**
 * Encode one Linux `struct can_frame` (16 bytes, native little-endian):
 * uint32 can_id, uint8 can_dlc, 3 pad bytes, uint8 data[8].
 */
export function canFrameBytes(id: number, dlc: number, data: readonly number[]): Uint8Array {
	const raw = new Uint8Array(16);
	new DataView(raw.buffer).setUint32(0, id, true);
	raw[4] = dlc;
	data.forEach((byte, index) => {
		raw[8 + index] = byte;
	});
	return raw;
}

/** One scripted step emitted by a fake transport. */
export type FakeStep = { kind: "chunk"; bytes: Uint8Array } | { kind: "error"; error: Error };

export interface FakeFactory {
	factory: TransportFactory;
	openCount: () => number;
	ifaces: () => string[];
	backends: () => CanTransport[];
}

/**
 * Build an injected transport factory. Each call to `factory` consumes the next
 * script entry; the transport yields that script's chunks and then ends cleanly.
 */
export function createFakeFactory(scriptPerOpen: FakeStep[][]): FakeFactory {
	const opens: Array<{ iface: string; backend: CanTransport }> = [];

	const factory: TransportFactory = (iface, backend) => {
		opens.push({ iface, backend });
		const steps = scriptPerOpen[opens.length - 1] ?? [];
		const transport: FrameTransport = {
			async *frames(): AsyncGenerator<Uint8Array> {
				for (const step of steps) {
					if (step.kind === "error") throw step.error;
					yield step.bytes;
				}
			},
		};
		return transport;
	};

	return {
		factory,
		openCount: () => opens.length,
		ifaces: () => opens.map((open) => open.iface),
		backends: () => opens.map((open) => open.backend),
	};
}
