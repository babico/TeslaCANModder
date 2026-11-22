import { describe, expect, it } from "@jest/globals";
import { Writable } from "node:stream";

import { endStream, writeWithDrain } from "../lib/streams.js";

class GatedWritable extends Writable {
	constructor() {
		super({ highWaterMark: 1 });
		this.release = null;
	}

	_write(_chunk, _encoding, callback) {
		this.release = callback;
	}
}

describe("writeWithDrain", () => {
	it("waits for drain when the write buffer is full", async () => {
		const stream = new GatedWritable();
		const pending = writeWithDrain(stream, "abc");

		expect(stream.writableLength).toBeGreaterThan(0);
		stream.release();
		await pending;
		expect(stream.writableLength).toBe(0);
	});
});

describe("endStream", () => {
	it("awaits finish before resolving", async () => {
		const stream = new Writable({
			write(_chunk, _encoding, callback) {
				setTimeout(callback, 10);
			},
		});

		await endStream(stream);
		expect(stream.writableFinished).toBe(true);
	});

	it("rejects when the stream errors before close", async () => {
		const stream = new Writable({
			write(_chunk, _encoding, callback) {
				callback(new Error("disk full"));
			},
		});
		const errors = [];
		stream.on("error", (error) => errors.push(error));
		stream.write("x");

		await expect(endStream(stream, errors)).rejects.toThrow("disk full");
	});
});
