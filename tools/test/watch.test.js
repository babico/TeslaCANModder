import { describe, it, expect } from "@jest/globals";
import { parseHexData, compareBits } from "../commands/watch.js";

describe("parseHexData", () => {
	it("parses hex bytes", () => {
		expect(parseHexData("0A1BFF")).toEqual([0x0a, 0x1b, 0xff]);
	});

	it("handles lowercase", () => {
		expect(parseHexData("ab cd")).toEqual([0xab, 0xcd]);
	});

	it("strips whitespace", () => {
		expect(parseHexData(" 0A  1B ")).toEqual([0x0a, 0x1b]);
	});

	it("returns empty for empty string", () => {
		expect(parseHexData("")).toEqual([]);
	});

	it("handles null/undefined", () => {
		expect(parseHexData(null)).toEqual([]);
		expect(parseHexData(undefined)).toEqual([]);
	});
});

describe("compareBits", () => {
	it("returns empty for identical arrays", () => {
		expect(compareBits([0x0a, 0xff], [0x0a, 0xff])).toEqual([]);
	});

	it("detects single byte change", () => {
		const changes = compareBits([0x00], [0x01]);
		expect(changes).toHaveLength(1);
		expect(changes[0].byteIdx).toBe(0);
		expect(changes[0].prevByte).toBe(0x00);
		expect(changes[0].currByte).toBe(0x01);
		expect(changes[0].changedBits).toEqual([{ bit: 0, from: 0, to: 1 }]);
	});

	it("handles different length arrays", () => {
		const changes = compareBits([0xff], [0xff, 0x01]);
		expect(changes).toHaveLength(1);
		expect(changes[0].byteIdx).toBe(1);
		expect(changes[0].prevByte).toBe(0);
		expect(changes[0].currByte).toBe(1);
	});

	it("detects multiple bit changes in one byte", () => {
		const changes = compareBits([0b00000000], [0b10000001]);
		expect(changes).toHaveLength(1);
		expect(changes[0].changedBits).toHaveLength(2);
		const bits = changes[0].changedBits.map((b) => b.bit).sort();
		expect(bits).toEqual([0, 7]);
	});

	it("returns empty for two empty arrays", () => {
		expect(compareBits([], [])).toEqual([]);
	});
});
