import { mkdtempSync, readFileSync, rmSync, statSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { describe, expect, it } from "@jest/globals";
import { correlateClipsToDrives, readDriveData } from "../src/sentry/connector.js";

/**
 * Neutral fixture keys. The real Sentry USB `drive-data.json` schema is
 * UNVERIFIED in this repo, so these are test-only join hints — not a claim
 * about Sentry's actual field names. The parser never hardcodes them.
 */
const FIXTURE = {
	drives: [{ id: "drive-2026-11-20", label: "evening" }],
	clips: [
		{ file: "clip-001.mp4", driveId: "drive-2026-11-20" },
		{ file: "clip-002.mp4", driveId: "no-such-drive" },
		{ file: "clip-003.mp4" },
	],
};

function tempDir(): string {
	return mkdtempSync(join(tmpdir(), "tcm-sentry-"));
}

function writeFixture(dir: string, contents: string): string {
	const path = join(dir, "drive-data.json");
	writeFileSync(path, contents, "utf8");
	return path;
}

describe("Sentry USB drive-data connector (read-only)", () => {
	it("loads a valid fixture and correlates clips to drives using caller-supplied keys", () => {
		const dir = tempDir();
		try {
			const path = writeFixture(dir, JSON.stringify(FIXTURE));
			const load = readDriveData(path);

			expect(load.status).toBe("loaded");
			expect(load.available).toBe(true);
			expect(load.data).toEqual(FIXTURE);
			expect(load.reason).toBeUndefined();

			const links = correlateClipsToDrives(load.data, {
				driveIdKey: "id",
				clipDriveKey: "driveId",
			});
			expect(links).toEqual([
				{
					clip: { file: "clip-001.mp4", driveId: "drive-2026-11-20" },
					drive: { id: "drive-2026-11-20", label: "evening" },
					via: "driveId",
				},
			]);
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});

	it("degrades gracefully when drive-data.json is missing", () => {
		const dir = tempDir();
		try {
			const load = readDriveData(join(dir, "drive-data.json"));
			expect(load.status).toBe("missing");
			expect(load.available).toBe(false);
			expect(load.data).toBeNull();
			expect(load.reason).toMatch(/missing|ENOENT/i);
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});

	it("degrades gracefully on invalid JSON", () => {
		const dir = tempDir();
		try {
			const load = readDriveData(writeFixture(dir, "not-json{"));
			expect(load.status).toBe("invalid");
			expect(load.available).toBe(false);
			expect(load.data).toBeNull();
			expect(load.reason).toMatch(/JSON|parse/i);
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});

	it("is tolerant of unknown shapes and invents no fields", () => {
		const unknown = { unexpected: [1, 2, 3], nested: { empty: [], nope: null } };
		// No key hints -> no invented schema -> no links.
		expect(correlateClipsToDrives(unknown)).toEqual([]);
		// With hints, unrecognised data still yields no links and never throws.
		expect(
			correlateClipsToDrives(unknown, { driveIdKey: "id", clipDriveKey: "driveId" }),
		).toEqual([]);
		// Non-object roots are accepted as-is by the loader.
		const dir = tempDir();
		try {
			expect(readDriveData(writeFixture(dir, "[1,2,3]")).data).toEqual([1, 2, 3]);
			expect(readDriveData(writeFixture(dir, "null")).data).toBeNull();
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});

	it("does not link a record to itself", () => {
		const self = { id: "self", driveId: "self" };
		const links = correlateClipsToDrives(
			{ records: [self] },
			{
				driveIdKey: "id",
				clipDriveKey: "driveId",
			},
		);
		expect(links).toEqual([]);
	});

	it("never writes to Sentry storage", () => {
		const dir = tempDir();
		try {
			const path = writeFixture(dir, JSON.stringify(FIXTURE));
			const before = readFileSync(path, "utf8");
			const beforeStat = statSync(path);

			readDriveData(path);
			correlateClipsToDrives(FIXTURE, { driveIdKey: "id", clipDriveKey: "driveId" });

			expect(readFileSync(path, "utf8")).toBe(before);
			expect(statSync(path).mtimeMs).toBe(beforeStat.mtimeMs);
			expect(statSync(path).size).toBe(beforeStat.size);
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	});
});
