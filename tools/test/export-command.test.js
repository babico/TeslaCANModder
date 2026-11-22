import { describe, it, expect } from "@jest/globals";
import { execFileSync } from "node:child_process";
import { mkdtempSync, writeFileSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { runExport } from "../commands/export.js";

const EXPORT_BIN = fileURLToPath(new URL("../bin/export.js", import.meta.url));

function makeOut() {
	return {
		logs: [],
		section(msg) {
			this.logs.push(["section", msg]);
		},
		info(msg) {
			this.logs.push(["info", msg]);
		},
		pass(msg) {
			this.logs.push(["pass", msg]);
		},
		fail(msg) {
			this.logs.push(["fail", msg]);
		},
	};
}

function writeDataset(dir) {
	const inputPath = path.join(dir, "dataset.json");
	const dataset = {
		dataset_source: { vehicle: "Model 3" },
		frames: [
			{
				id: 280,
				hex: "0x118",
				frame_name: "DI_systemStatus",
				signals: [
					{
						signal_name: "DI_gear",
						possible_values: [{ value_dec: 1, value_hex: "0x1", label: "P" }],
					},
				],
			},
		],
	};
	writeFileSync(inputPath, JSON.stringify(dataset), "utf8");
	return inputPath;
}

describe("runExport", () => {
	it("fails when input is missing", async () => {
		const out = makeOut();
		const code = await runExport({ format: "json" }, out);
		expect(code).toBe(1);
		expect(out.logs.some(([t]) => t === "fail")).toBe(true);
	});

	it("fails for unknown format", async () => {
		const dir = mkdtempSync(path.join(tmpdir(), "tcm-export-"));
		const inputPath = writeDataset(dir);
		const out = makeOut();
		const code = await runExport({ input: inputPath, format: "xml" }, out);
		expect(code).toBe(1);
	});

	it("fails for invalid dataset shape", async () => {
		const dir = mkdtempSync(path.join(tmpdir(), "tcm-export-"));
		const inputPath = path.join(dir, "bad.json");
		writeFileSync(inputPath, JSON.stringify({ nope: true }), "utf8");
		const out = makeOut();
		const code = await runExport({ input: inputPath, format: "json" }, out);
		expect(code).toBe(1);
	});

	it("writes output file and blocks overwrite by default", async () => {
		const dir = mkdtempSync(path.join(tmpdir(), "tcm-export-"));
		const inputPath = writeDataset(dir);
		const outputPath = path.join(dir, "out.json");
		const out = makeOut();

		const first = await runExport(
			{ input: inputPath, format: "json", output: outputPath },
			out,
		);
		expect(first).toBe(0);
		const second = await runExport(
			{ input: inputPath, format: "json", output: outputPath },
			out,
		);
		expect(second).toBe(1);
	});

	it("overwrites output when overwrite is true", async () => {
		const dir = mkdtempSync(path.join(tmpdir(), "tcm-export-"));
		const inputPath = writeDataset(dir);
		const outputPath = path.join(dir, "out.csv");
		const out = makeOut();

		const first = await runExport({ input: inputPath, format: "csv", output: outputPath }, out);
		expect(first).toBe(0);

		writeFileSync(outputPath, "old-content", "utf8");
		const second = await runExport(
			{ input: inputPath, format: "csv", output: outputPath, overwrite: true },
			out,
		);
		expect(second).toBe(0);
		expect(readFileSync(outputPath, "utf8")).toContain("frame_id_dec,frame_id_hex");
	});
});

describe("standalone export entry", () => {
	it("runs tools/bin/export.js as a standalone script", () => {
		const dir = mkdtempSync(path.join(tmpdir(), "tcm-export-bin-"));
		const inputPath = writeDataset(dir);
		try {
			const stdout = execFileSync(
				process.execPath,
				[EXPORT_BIN, "--input", inputPath, "--format", "json"],
				{ encoding: "utf8" },
			);
			expect(stdout).toContain("DI_systemStatus");
			expect(stdout).toContain('"counts"');
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});
});
