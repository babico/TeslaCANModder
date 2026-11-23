import { realpathSync } from "node:fs";
import type { RequestListener, Server } from "node:http";
import { fileURLToPath } from "node:url";

import {
	busFromIface,
	createCanReader,
	type CanFrame,
	type CanReaderOptions,
	type TransportFactory,
} from "./can/socketcan.js";
import { createCandumpTransportFactory } from "./can/candump.js";
import { createBridgeListener, type BridgeOptions } from "./http/bridge.js";
import { createPiServer, getCanTransport, type PiServerOptions } from "./index.js";
import { createRecorder, type Recorder, type RecorderOptions } from "./record/recorder.js";

/**
 * Runtime entrypoint for `tcm-can.service` (`pi/dist/main.js`).
 *
 * Composition: candump -> read-only CanReader -> bridge + JSONL recorder,
 * served on the loopback port pinned by `TCM_BIND_HOST`/`TCM_BIND_PORT`.
 * CAN TX is deliberately UNMOUNTED: this module never imports `can/tx.ts`,
 * never reads the RT gate artifact and never opens a TX socket. Importing it
 * has no side effects; the bootstrap at the bottom runs only on a direct run.
 *
 * UNVERIFIED on hardware: no Pi, can-utils, vcan or bench capture was
 * available. All process/library behaviour here is covered by fakes only.
 */

export const DEFAULT_BIND_HOST = "127.0.0.1";
export const DEFAULT_BIND_PORT = 8090;
export const DEFAULT_CAPTURE_DIR = "/var/lib/teslacanmodder/captures";

const WILDCARD_HOSTS = new Set(["", "*", "0.0.0.0", "::", "[::]"]);

export interface BindConfig {
	host: string;
	port: number;
}

/** Parse and validate the loopback bind from the environment; fail closed. */
export function parseBindConfig(env: NodeJS.ProcessEnv = process.env): BindConfig {
	const host = env.TCM_BIND_HOST ?? DEFAULT_BIND_HOST;
	const rawPort = env.TCM_BIND_PORT ?? String(DEFAULT_BIND_PORT);
	if (WILDCARD_HOSTS.has(host) || host.includes("*")) {
		throw new Error(`invalid TCM_BIND_HOST bind configuration: "${host}" is not loopback`);
	}
	if (!/^\d+$/.test(rawPort)) {
		throw new Error(`invalid TCM_BIND_PORT bind configuration: "${rawPort}"`);
	}
	const port = Number(rawPort);
	if (!Number.isInteger(port) || port < 1 || port > 65535) {
		throw new Error(`invalid TCM_BIND_PORT bind configuration: "${rawPort}"`);
	}
	return { host, port };
}

export function parseIfaces(env: NodeJS.ProcessEnv = process.env): string[] {
	const fromEnv = (env.TCM_CAN_IFACES ?? "")
		.split(",")
		.map((value) => value.trim())
		.filter(Boolean);
	if (fromEnv.length > 0) return fromEnv;
	return getCanTransport(env) === "vcan" ? ["vcan0"] : ["can0"];
}

/**
 * Broadcast one source to `count` independent consumers. Backpressure-free by
 * design: each subscriber has its own queue, so a slow recorder cannot stall
 * the bridge's status snapshot. A shared pump starts on first consumption.
 */
export function fanoutFrames(
	source: AsyncIterable<CanFrame>,
	count: number,
): AsyncIterable<CanFrame>[] {
	if (count < 1) throw new Error("fanoutFrames requires at least one consumer");
	const subscribers = Array.from({ length: count }, () => ({
		queue: [] as CanFrame[],
		done: false,
		wake: undefined as (() => void) | undefined,
	}));
	let started = false;

	async function pump(): Promise<void> {
		try {
			for await (const frame of source) {
				for (const sub of subscribers) {
					sub.queue.push(frame);
					sub.wake?.();
					sub.wake = undefined;
				}
			}
		} catch {
			// A failed source ends all consumers cleanly; the reader itself
			// reconnects internally, so this only covers exhausted sources.
		} finally {
			for (const sub of subscribers) {
				sub.done = true;
				sub.wake?.();
				sub.wake = undefined;
			}
		}
	}

	return subscribers.map((sub) => ({
		async *[Symbol.asyncIterator](): AsyncGenerator<CanFrame> {
			if (!started) {
				started = true;
				void pump();
			}
			while (true) {
				if (sub.queue.length > 0) {
					yield sub.queue.shift()!;
					continue;
				}
				if (sub.done) return;
				await new Promise<void>((resolve) => {
					sub.wake = resolve;
				});
			}
		},
	}));
}

/** Merge independent per-bus streams into one bridge/recorder feed. */
export function mergeFrames(sources: readonly AsyncIterable<CanFrame>[]): AsyncIterable<CanFrame> {
	return {
		async *[Symbol.asyncIterator](): AsyncGenerator<CanFrame> {
			let remaining = sources.length;
			if (remaining === 0) return;
			const queues: CanFrame[][] = sources.map(() => []);
			const done = sources.map(() => false);
			const wake: Array<(() => void) | undefined> = sources.map(() => undefined);

			sources.forEach((source, index) => {
				void (async () => {
					try {
						for await (const frame of source) {
							queues[index]!.push(frame);
							wake[index]?.();
							wake[index] = undefined;
						}
					} finally {
						if (!done[index]) {
							done[index] = true;
							remaining -= 1;
						}
						wake[index]?.();
						wake[index] = undefined;
					}
				})();
			});

			while (remaining > 0 || queues.some((queue) => queue.length > 0)) {
				let yielded = false;
				for (let index = 0; index < sources.length; index += 1) {
					while (queues[index]!.length > 0) {
						yield queues[index]!.shift()!;
						yielded = true;
					}
				}
				if (!yielded) {
					await Promise.all(
						sources.map((_source, index) =>
							done[index] || queues[index]!.length > 0
								? Promise.resolve()
								: new Promise<void>((resolve) => {
										wake[index] = resolve;
									}),
						),
					);
				}
			}
		},
	};
}

export type ListenFn = (server: Server, host: string, port: number) => Promise<() => Promise<void>>;

const defaultListen: ListenFn = async (server, host, port) => {
	await new Promise<void>((resolve, reject) => {
		const onError = (error: Error): void => reject(error);
		server.once("error", onError);
		server.listen(port, host, () => {
			server.off("error", onError);
			resolve();
		});
	});
	return () =>
		new Promise<void>((resolve, reject) => {
			server.close((error) => (error ? reject(error) : resolve()));
		});
};

export interface PiServiceOptions {
	/** Environment source. Defaults to `process.env`. */
	env?: NodeJS.ProcessEnv;
	/** CAN interfaces; defaults to `TCM_CAN_IFACES` or `can0` (`vcan0` for vcan). */
	ifaces?: string[];
	/** Capture directory; defaults to `TCM_CAPTURE_DIR` or `/var/lib/teslacanmodder`. */
	captureDir?: string;
	/** Raw frame source factory. Defaults to the `candump` process transport. */
	transport?: TransportFactory;
	/** Clock for `rxMs`; defaults to `Date.now`. */
	now?: () => number;
	/** Injectable seams for composition tests. */
	readerFactory?: (iface: string, options: CanReaderOptions) => AsyncIterable<CanFrame>;
	bridgeFactory?: (options: BridgeOptions) => RequestListener;
	recorderFactory?: (dir: string, options: RecorderOptions) => Recorder;
	serverFactory?: (options: PiServerOptions) => Server;
	listen?: ListenFn;
	/**
	 * Deliberately never called. Retained only so composition tests can assert
	 * that even a supplied TX capability stays UNMOUNTED.
	 */
	txFactory?: () => unknown;
}

export interface PiServiceHandle {
	server: Server;
	recorder: Recorder;
	/** Close the listener and end the recorder exactly once. */
	stop(): Promise<void>;
}

/** Compose the read-only Pi service and bind it to the pinned loopback port. */
export async function startPiService(options: PiServiceOptions = {}): Promise<PiServiceHandle> {
	const env = options.env ?? process.env;
	const { host, port } = parseBindConfig(env); // validate before any composition

	const transport = options.transport ?? createCandumpTransportFactory();
	const now = options.now ?? Date.now;
	const backend = getCanTransport(env);
	const ifaces = options.ifaces ?? parseIfaces(env);
	const captureDir = options.captureDir ?? env.TCM_CAPTURE_DIR ?? DEFAULT_CAPTURE_DIR;

	const makeReader =
		options.readerFactory ??
		((iface: string, opts: CanReaderOptions) => createCanReader(iface, opts));
	const makeBridge = options.bridgeFactory ?? createBridgeListener;
	const makeRecorder = options.recorderFactory ?? createRecorder;
	const makeServer = options.serverFactory ?? createPiServer;
	const doListen = options.listen ?? defaultListen;
	// options.txFactory is intentionally ignored — no TX path is mounted.

	const branches = ifaces.map((iface) =>
		fanoutFrames(makeReader(iface, { transport, now, backend }), 2),
	);
	const frameFeed = mergeFrames(branches.map((pair) => pair[0]!));
	const recorderFeed = mergeFrames(branches.map((pair) => pair[1]!));

	const bridge = makeBridge({ token: env.TCM_BRIDGE_TOKEN, reader: frameFeed });
	const recorder = makeRecorder(captureDir, {
		provenance: {
			bus: ifaces.map(busFromIface),
			vehicle: "UNVERIFIED",
			firmware: "UNVERIFIED",
		},
	});
	const server = makeServer({ transport: backend, bridge });

	const close = await doListen(server, host, port);

	void (async () => {
		try {
			for await (const frame of recorderFeed) await recorder.write(frame);
		} catch {
			// A broken read path must not crash the HTTP service or leave an
			// unhandled rejection; stop() still ends the recorder cleanly.
		}
	})();

	let stopPromise: Promise<void> | undefined;
	return {
		server,
		recorder,
		stop(): Promise<void> {
			if (!stopPromise) {
				stopPromise = (async () => {
					await close();
					await recorder.stop();
				})();
			}
			return stopPromise;
		},
	};
}

function isDirectRun(): boolean {
	const entry = process.argv[1];
	if (!entry) return false;
	try {
		return realpathSync(entry) === realpathSync(fileURLToPath(import.meta.url));
	} catch {
		return false;
	}
}

if (isDirectRun()) {
	startPiService()
		.then(() => {
			// The server keeps the event loop alive; no console output on success.
		})
		.catch((error: unknown) => {
			console.error("tcm-can failed to start:", error);
			process.exitCode = 1;
		});
}
