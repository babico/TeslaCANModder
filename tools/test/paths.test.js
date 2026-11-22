import { describe, expect, it } from "@jest/globals";
import path from "node:path";

import { resolveInsideCwd } from "../lib/paths.js";

const BASE = path.resolve("tools");

describe("resolveInsideCwd", () => {
	it("resolves a relative path inside the base directory", () => {
		expect(resolveInsideCwd("test/sample.jsonl", BASE)).toBe(
			path.join(BASE, "test", "sample.jsonl"),
		);
	});

	it("accepts an absolute path inside the base directory", () => {
		const target = path.join(BASE, "lib", "session.js");
		expect(resolveInsideCwd(target, BASE)).toBe(target);
	});

	it("rejects parent traversal", () => {
		expect(resolveInsideCwd("../outside.jsonl", BASE)).toBeNull();
	});

	it("rejects an absolute path outside the base directory", () => {
		expect(resolveInsideCwd(path.resolve("package.json"), BASE)).toBeNull();
	});

	it("rejects a sibling directory with a matching prefix", () => {
		expect(resolveInsideCwd("../tools-evil/sample.jsonl", BASE)).toBeNull();
	});

	it("rejects empty, null-byte and directory-as-file inputs", () => {
		expect(resolveInsideCwd("", BASE)).toBeNull();
		expect(resolveInsideCwd(null, BASE)).toBeNull();
		expect(resolveInsideCwd("bad\0name", BASE)).toBeNull();
		expect(resolveInsideCwd(".", BASE)).toBeNull();
	});
});
