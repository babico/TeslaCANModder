import { spawn } from "node:child_process";

import {
	busFromIface,
	CAN_FRAME_BYTES,
	type BusId,
	type CanFrame,
	type FrameTransport,
	type TransportFactory,
} from "./socketcan.js";

/**
 * Production SocketCAN read path: spawn `candump -L <iface>` (can-utils) and
 * convert its log lines into the shared `CanFrame` shape. No native Node module
 * is installed and no socket is opened by this module — the child process is
 * created only when a `FrameTransport` is consumed. UNVERIFIED on hardware:
 * line format is parsed from can-utils `-L` output, not from a captured Pi log.
 *
 * The process factory is injectable so tests use a fake stdout stream instead
 * of a real child process. This is NOT the optional TCP gateway client in
 * `socketcan.ts`; that path is unshipped and never used by the service.
 */

/** Status reported by the child's `exit` event (`code` is null when signalled). */
export interface CandumpExit {
	code: number | null;
	signal: NodeJS.Signals | null;
}

/** Minimal surface of a spawned `candump` child that the transport needs. */
export interface CandumpProcess {
	stdout: AsyncIterable<Buffer | string>;
	kill?(): void;
	/** Returns a deferred spawn/stream error so the reader can reconnect. */
	error?(): Error | undefined;
	/** Returns the exit status once `exit` fires, undefined while still running. */
	exit?(): CandumpExit | undefined;
}

/** Injected process seam. Receives the command and argv separately. */
export type CandumpProcessFactory = (command: string, args: readonly string[]) => CandumpProcess;

export interface CandumpTransportOptions {
	/** Override the process factory (tests). Defaults to `spawnCandumpProcess`. */
	processFactory?: CandumpProcessFactory;
	/** Override the binary name. Defaults to `candump`. */
	command?: string;
}

const LINE = /^(?:\(\s*[\d.]+\s*\)\s+)?(\S+)\s+(\S+)(?:\s+([\s\S]*))?$/;
const HEX_BYTES = /^[0-9a-fA-F]*$/;

/**
 * Parse one candump line into a `CanFrame`, or `undefined` for lines that carry
 * no 11-bit data frame (blank lines, remote frames, EFF/RTR/error IDs, other
 * interfaces, malformed hex). Both `-L` (`iface ID#DATA`) and the classic
 * (`iface ID [dlc] hh hh`) formats are accepted.
 */
export function parseCandumpLine(line: string, iface: string, rxMs: number): CanFrame | undefined {
	const match = LINE.exec(line.trim());
	if (!match || match[1] !== iface) return undefined;
	const bus: BusId = busFromIface(iface);
	const token = match[2] ?? "";
	const rest = match[3] ?? "";

	let idHex: string;
	let data: Uint8Array;
	if (token.includes("#")) {
		const [idPart, dataPart = ""] = token.split("#", 2);
		idHex = idPart ?? "";
		if (dataPart === "R" || dataPart === "r") return undefined; // remote frame
		if (!HEX_BYTES.test(dataPart) || dataPart.length % 2 !== 0) return undefined;
		if (dataPart.length / 2 > 8) return undefined;
		data = new Uint8Array(dataPart.length / 2);
		for (let i = 0; i < data.length; i += 1) {
			data[i] = Number.parseInt(dataPart.slice(i * 2, i * 2 + 2), 16);
		}
	} else {
		idHex = token;
		const classic = /^\[(\d+)\](?:\s+([\s\S]*))?$/.exec(rest.trim());
		if (!classic) return undefined;
		const dlc = Number(classic[1]);
		const byteTokens = (classic[2] ?? "").trim().split(/\s+/).filter(Boolean);
		if (!Number.isInteger(dlc) || dlc < 0 || dlc > 8 || byteTokens.length !== dlc) {
			return undefined;
		}
		data = new Uint8Array(dlc);
		for (let i = 0; i < dlc; i += 1) {
			const byte = byteTokens[i] ?? "";
			if (!/^[0-9a-fA-F]{2}$/.test(byte)) return undefined;
			data[i] = Number.parseInt(byte, 16);
		}
	}

	if (!/^[0-9a-fA-F]{1,8}$/.test(idHex)) return undefined;
	const id = Number.parseInt(idHex, 16);
	if (!Number.isInteger(id) || id > 0x7ff) return undefined; // drop EFF/RTR/ERR flags

	return { id, dlc: data.length, data, bus, rxMs };
}

/** Serialize a CanFrame back into the 16-byte can_frame the shared reader parses. */
function frameToCanBytes(frame: CanFrame): Uint8Array {
	const raw = new Uint8Array(CAN_FRAME_BYTES);
	new DataView(raw.buffer).setUint32(0, frame.id, true);
	raw[4] = frame.dlc;
	raw.set(frame.data.subarray(0, frame.dlc), 8);
	return raw;
}

/** Real process factory. Spawn errors and exit status are surfaced to the reader. */
export const spawnCandumpProcess: CandumpProcessFactory = (command, args) => {
	const child = spawn(command, [...args], { stdio: ["ignore", "pipe", "pipe"] });
	let spawnError: Error | undefined;
	let exitStatus: CandumpExit | undefined;
	child.on("error", (error: Error) => {
		spawnError = error;
		child.stdout?.destroy();
	});
	child.on("exit", (code, signal) => {
		exitStatus = { code, signal };
	});
	return {
		stdout: child.stdout as AsyncIterable<Buffer>,
		kill: () => child.kill(),
		error: () => spawnError,
		exit: () => exitStatus,
	};
};

/**
 * Build a `FrameTransport` from the injected process factory. Spawns one
 * `candump -L <iface>` per open and converts each stdout line to a 16-byte
 * can_frame chunk. Line buffering handles chunks that split mid-line; the
 * shared reader handles chunks that split mid-frame and reconnects.
 */
export function createCandumpTransport(
	iface: string,
	options: CandumpTransportOptions = {},
): FrameTransport {
	const factory = options.processFactory ?? spawnCandumpProcess;
	const command = options.command ?? "candump";
	const args = ["-L", iface];
	let active: CandumpProcess | undefined;

	return {
		async *frames(): AsyncGenerator<Uint8Array> {
			const process = factory(command, args);
			active = process;
			try {
				let partial = "";
				const consume = (rawLine: string): Uint8Array | undefined => {
					const line = rawLine.trim();
					if (!line) return undefined;
					const frame = parseCandumpLine(line, iface, Date.now());
					return frame ? frameToCanBytes(frame) : undefined;
				};
				for await (const chunk of process.stdout) {
					partial += typeof chunk === "string" ? chunk : chunk.toString("utf8");
					const lines = partial.split(/\r?\n/);
					partial = lines.pop() ?? "";
					for (const line of lines) {
						const bytes = consume(line);
						if (bytes) yield bytes;
					}
				}
				const tail = consume(partial);
				if (tail) yield tail;
				const error = process.error?.();
				if (error) throw error;
				// A clean stdout end is NOT a healthy steady state: when the
				// child has exited (even with code 0) the reader must see an
				// error so createCanReader() reconnects instead of silently
				// going blind. A stream that merely ends with no exit event
				// remains a clean end for injected/fake transports.
				const exit = process.exit?.();
				if (exit) {
					throw new Error(
						`candump exited unexpectedly (code=${exit.code ?? "null"}, signal=${exit.signal ?? "none"})`,
					);
				}
			} finally {
				process.kill?.();
				if (active === process) active = undefined;
			}
		},
		close: () => active?.kill?.(),
	};
}

/** TransportFactory adapter for `createCanReader()`. */
export function createCandumpTransportFactory(
	options: CandumpTransportOptions = {},
): TransportFactory {
	return (iface: string) => createCandumpTransport(iface, options);
}
