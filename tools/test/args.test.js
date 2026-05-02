import { describe, it, expect } from "@jest/globals";
import { parseArgs, resolveOptions } from "../lib/args.js";

describe("parseArgs", () => {
	it("parses --key value pairs", () => {
		const result = parseArgs(["--port", "COM3", "--baud", "9600"]);
		expect(result.port).toBe("COM3");
		expect(result.baud).toBe("9600");
	});

	it("parses boolean flags", () => {
		const result = parseArgs(["--json", "--no-color"]);
		expect(result.json).toBe(true);
		expect(result["no-color"]).toBe(true);
	});

	it("collects positional args into _", () => {
		const result = parseArgs(["smoke", "--port", "COM3"]);
		expect(result._).toEqual(["smoke"]);
	});

	it("treats --flag followed by --other as boolean", () => {
		const result = parseArgs(["--erase", "--port", "COM3"]);
		expect(result.erase).toBe(true);
		expect(result.port).toBe("COM3");
	});

	it("handles empty argv", () => {
		const result = parseArgs([]);
		expect(result._).toEqual([]);
	});

	it("handles --key at end of argv as boolean", () => {
		const result = parseArgs(["--verbose"]);
		expect(result.verbose).toBe(true);
	});

	it("parses short options with values", () => {
		const result = parseArgs(["-i", "in.json", "-f", "csv", "-o", "out.csv"]);
		expect(result.i).toBe("in.json");
		expect(result.f).toBe("csv");
		expect(result.o).toBe("out.csv");
	});

	it("parses short option without value as boolean", () => {
		const result = parseArgs(["-x"]);
		expect(result.x).toBe(true);
	});
});

describe("resolveOptions", () => {
	it("applies defaults", () => {
		const opts = resolveOptions({ _: [] });
		expect(opts.command).toBe("smoke");
		expect(opts.baud).toBe(115200);
		expect(opts.timeoutMs).toBe(3000);
		expect(opts.warmupMs).toBe(2000);
		expect(opts.port).toBeNull();
		expect(opts.variant).toBeNull();
		expect(opts.jsonOutput).toBe(false);
		expect(opts.noColor).toBe(false);
	});

	it("extracts command from positional", () => {
		const opts = resolveOptions({ _: ["watch"] });
		expect(opts.command).toBe("watch");
	});

	it("maps port and baud", () => {
		const opts = resolveOptions({ _: [], port: "COM5", baud: "9600" });
		expect(opts.port).toBe("COM5");
		expect(opts.baud).toBe(9600);
	});

	it("parses filterIds from comma-separated string", () => {
		const opts = resolveOptions({ _: [], filter: "69,281,1021" });
		expect(opts.filterIds).toEqual([69, 281, 1021]);
	});

	it("filters out invalid ids", () => {
		const opts = resolveOptions({ _: [], filter: "69,abc,-1,0,281" });
		expect(opts.filterIds).toEqual([69, 281]);
	});

	it("returns empty filterIds for no filter", () => {
		const opts = resolveOptions({ _: [] });
		expect(opts.filterIds).toEqual([]);
	});

	it("maps flash options", () => {
		const opts = resolveOptions({ _: ["flash"], hex: "firmware.hex", erase: true });
		expect(opts.flashHex).toBe("firmware.hex");
		expect(opts.eraseChip).toBe(true);
	});

	it("maps vehicle command", () => {
		const opts = resolveOptions({ _: ["vehicle"], "vehicle-cmd": "mirror-fold" });
		expect(opts.vehicleCmd).toBe("mirror-fold");
	});

	it("maps drive-context strict options", () => {
		const opts = resolveOptions({
			_: ["drive-context"],
			"drive-duration": "60000",
			"drive-output": "artifacts/report.json",
			"drive-note-output": "artifacts/roadmap-note.txt",
			"drive-min-samples": "10",
			"drive-expect-full": true,
		});

		expect(opts.command).toBe("drive-context");
		expect(opts.driveCtxDurMs).toBe(60000);
		expect(opts.driveCtxOutput).toBe("artifacts/report.json");
		expect(opts.driveCtxNoteOutput).toBe("artifacts/roadmap-note.txt");
		expect(opts.driveCtxMinSamples).toBe(10);
		expect(opts.driveCtxExpectFull).toBe(true);
	});
});
