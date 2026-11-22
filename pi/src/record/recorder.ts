import { once } from "node:events";
import { createWriteStream, mkdirSync } from "node:fs";
import { join } from "node:path";
import type { Writable } from "node:stream";
import type { BusId, CanFrame } from "../can/socketcan.js";

/** Capture provenance written as the first JSONL line of every recording. */
export interface Provenance {
	vehicle: string;
	bus: BusId[];
	firmware: string;
	date: string;
}

export interface RecorderOptions {
	/** Overrides for the provenance header. Unspecified fields use safe defaults. */
	provenance?: Partial<Provenance>;
	/** Clock used for the default `date`. Defaults to `Date.now`. */
	now?: () => Date;
	/** Output file name inside `dir`. Defaults to `capture-<date>.jsonl`. */
	fileName?: string;
	/** Injected writable stream (tests). Defaults to a real `createWriteStream`. */
	stream?: Writable;
}

export interface Recorder {
	readonly path: string;
	readonly provenance: Readonly<Provenance>;
	/** Append one A2 frame. Resolves after the write clears backpressure. */
	write(frame: CanFrame): Promise<void>;
	/** Alias for `write`. */
	record(frame: CanFrame): Promise<void>;
	/** Flush all pending writes, end the stream and surface any captured error. */
	stop(): Promise<void>;
}

function frameLine(frame: CanFrame): string {
	return JSON.stringify({
		t: "frame",
		dir: "rx",
		bus: frame.bus,
		id: frame.id,
		dlc: frame.dlc,
		d: Buffer.from(frame.data).toString("hex"),
		ts: frame.rxMs,
	});
}

/**
 * Create a JSONL CAN capture recorder in `dir`.
 *
 * The first line is a `{vehicle,bus,firmware,date}` provenance object; every
 * following line is an A2 frame in the firmware-compatible `{t:"frame"}`
 * shape plus a `ts` timestamp (the frame's `rxMs`). Writes are serialized so
 * ordering is preserved and backpressure is awaited. A persistent `error`
 * listener captures the first stream error so it is never an uncaught event
 * and is surfaced by `write()`/`stop()`.
 */
export function createRecorder(dir: string, options: RecorderOptions = {}): Recorder {
	const now = options.now ?? (() => new Date());
	const provenance: Provenance = {
		vehicle: options.provenance?.vehicle ?? "unknown",
		bus: options.provenance?.bus ?? [0, 1, 2],
		firmware: options.provenance?.firmware ?? "unknown",
		date: options.provenance?.date ?? now().toISOString(),
	};
	const fileName = options.fileName ?? `capture-${provenance.date.replace(/[:.]/g, "-")}.jsonl`;
	const path = join(dir, fileName);

	if (!options.stream) mkdirSync(dir, { recursive: true });
	const stream = options.stream ?? createWriteStream(path);

	// Persistent listener: the stream never emits an unhandled `error`, and the
	// first error is retained for `write()`/`stop()` to surface.
	let firstError: Error | undefined;
	stream.on("error", (error: Error) => {
		if (!firstError) firstError = error;
	});

	// Serialized write chain. `tail` always resolves (the rejection is kept on
	// the returned task) so a failed write cannot stall later writes or stop().
	let tail: Promise<void> = Promise.resolve();
	function enqueue(chunk: string): Promise<void> {
		const task = tail.then(async () => {
			if (firstError) throw firstError;
			if (!stream.write(chunk)) await once(stream, "drain"); // rejects on `error`
			if (firstError) throw firstError;
		});
		tail = task.catch(() => undefined);
		return task;
	}

	const write = (frame: CanFrame): Promise<void> => enqueue(`${frameLine(frame)}\n`);
	void enqueue(`${JSON.stringify(provenance)}\n`); // header; awaited through `tail` in stop()

	let stopPromise: Promise<void> | undefined;
	function stop(): Promise<void> {
		if (!stopPromise) {
			stopPromise = (async () => {
				await tail; // header + all frames, with drain already awaited
				if (firstError) throw firstError;
				const finished = once(stream, "finish"); // rejects if an error lands here
				stream.end();
				await finished;
				if (firstError) throw firstError;
			})();
		}
		return stopPromise;
	}

	return { path, provenance, write, record: write, stop };
}
