import { describe, expect, it, jest } from "@jest/globals";
import fs from "node:fs";
import path from "node:path";

import { parseReplayLines, runReplay } from "../commands/replay.js";

function makeOut() {
	return {
		section: jest.fn(),
		info: jest.fn(),
		pass: jest.fn(),
		warn: jest.fn(),
		fail: jest.fn(),
	};
}

function makeSession() {
	return {
		send: jest.fn(),
	};
}

const SAMPLE = [
	"timestamp,id,dir,dlc,data,bus,seq",
	'{"ts":"12:00","id":69,"dir":"rx","dlc":8,"data":"0102","bus":0,"seq":1}',
	"not-json",
	'{"unrelated":true}',
	"",
].join("\n");

describe("parseReplayLines", () => {
	it("separates valid frames from skipped header and malformed lines", () => {
		const { frames, skipped } = parseReplayLines(SAMPLE);

		expect(frames).toHaveLength(1);
		expect(frames[0].id).toBe(69);
		expect(skipped).toBe(3);
	});
});

describe("runReplay", () => {
	it("reports how many lines were skipped", async () => {
		const tempDir = fs.mkdtempSync(path.join(process.cwd(), "tcm-replay-"));
		const inputPath = path.join(tempDir, "frames.jsonl");
		fs.writeFileSync(inputPath, SAMPLE);
		const out = makeOut();
		const session = makeSession();

		try {
			await runReplay(session, { replayFile: inputPath, replaySpeed: 0 }, out);

			expect(session.send).toHaveBeenCalledTimes(1);
			expect(out.info).toHaveBeenCalledWith(expect.stringContaining("Skipped: 3"));
			expect(out.pass).toHaveBeenCalledWith(expect.stringContaining("skipped: 3"));
		} finally {
			fs.rmSync(tempDir, { recursive: true, force: true });
		}
	});
});
