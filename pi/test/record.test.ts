import { mkdtempSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { Writable } from "node:stream";
import { describe, expect, it } from "@jest/globals";
import type { CanFrame } from "../src/can/socketcan.js";
import { createRecorder, sanitizeFileName } from "../src/record/recorder.js";

/** Injected A2 frames — no hardware, no SocketCAN, no TX. */
const FRAMES: CanFrame[] = [
	{ id: 0x3fd, dlc: 8, data: new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8]), bus: 0, rxMs: 1000 },
	{ id: 0x2b9, dlc: 2, data: new Uint8Array([0xaa, 0xbb]), bus: 1, rxMs: 1005 },
];

function frameLine(frame: CanFrame): Record<string, unknown> {
	return {
		t: "frame",
		dir: "rx",
		bus: frame.bus,
		id: frame.id,
		dlc: frame.dlc,
		d: Buffer.from(frame.data).toString("hex"),
		ts: frame.rxMs,
	};
}

/** Fake writable that accumulates chunks so no real file is opened. */
function collectingStream(): { stream: Writable; chunks: () => string[] } {
	const written: string[] = [];
	const stream = new Writable({
		write(chunk, _encoding, callback) {
			written.push(String(chunk));
			callback();
		},
	});
	return { stream, chunks: () => written };
}

function tempDir(): string {
	return mkdtempSync(join(tmpdir(), "tcm-rec-"));
}

describe("CAN capture recorder", () => {
	it("flushes a provenance header followed by N frames on stop()", async () => {
		const dir = tempDir();
		try {
			const recorder = createRecorder(dir, {
				provenance: {
					vehicle: "VIN-123",
					bus: [0, 1],
					firmware: "1.5.0",
					date: "2026-11-23T00:00:00.000Z",
				},
			});
			for (const frame of FRAMES) await recorder.write(frame);
			await recorder.stop();

			const lines = readFileSync(recorder.path, "utf8")
				.trim()
				.split("\n")
				.map((line) => JSON.parse(line) as Record<string, unknown>);
			expect(lines).toHaveLength(FRAMES.length + 1);
			expect(lines[0]).toEqual({
				vehicle: "VIN-123",
				bus: [0, 1],
				firmware: "1.5.0",
				date: "2026-11-23T00:00:00.000Z",
			});
			expect(lines.slice(1)).toEqual(FRAMES.map(frameLine));
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});

	it("defaults provenance and derives the date from the injected clock", async () => {
		const { stream, chunks } = collectingStream();
		const recorder = createRecorder(join(tmpdir(), "unused"), {
			stream,
			now: () => new Date("2026-01-02T03:04:05.000Z"),
		});
		await recorder.stop();

		const header = JSON.parse(chunks()[0]) as Record<string, unknown>;
		expect(Object.keys(header).sort()).toEqual(["bus", "date", "firmware", "vehicle"]);
		expect(header.vehicle).toBe("unknown");
		expect(header.firmware).toBe("unknown");
		expect(header.bus).toEqual([0, 1, 2]);
		expect(header.date).toBe("2026-01-02T03:04:05.000Z");
	});

	it("sanitizes caller-supplied file names and keeps the provenance header intact", async () => {
		expect(sanitizeFileName("capture-2026-11-23.jsonl")).toBe("capture-2026-11-23.jsonl");
		for (const bad of ["../escape.jsonl", "a\\b.jsonl", "a..b.jsonl", "/etc/x", ""]) {
			expect(() => sanitizeFileName(bad)).toThrow(/unsafe|filename/i);
		}

		const { stream, chunks } = collectingStream();
		const recorder = createRecorder(join(tmpdir(), "unused"), {
			stream,
			fileName: "capture-safe.jsonl",
			provenance: { vehicle: "VIN-9" },
		});
		await recorder.write(FRAMES[0]);
		await recorder.stop();
		expect(recorder.path).toBe(join(tmpdir(), "unused", "capture-safe.jsonl"));
		const header = JSON.parse(chunks()[0]) as Record<string, unknown>;
		expect(header.vehicle).toBe("VIN-9");
		expect(header.date).toEqual(expect.any(String));
	});

	it("rejects on a stream error and keeps the error listener attached", async () => {
		const boom = new Error("disk full");
		const stream = new Writable({
			write(_chunk, _encoding, callback) {
				callback(boom);
			},
		});
		const recorder = createRecorder(join(tmpdir(), "unused"), { stream });
		await expect(recorder.stop()).rejects.toThrow("disk full");
		// Second stop is idempotent and surfaces the same captured error.
		await expect(recorder.stop()).rejects.toThrow("disk full");
	});
});
