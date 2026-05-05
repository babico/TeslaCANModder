// ── monitorDiagnosticsPersistence Tests ──────────────────────────────────────
// Exercises the localStorage fallback path (no IndexedDB) and the sanitizer
// behavior on malformed input. IndexedDB read/write paths are covered
// indirectly by their public sanitization contract.

import {
	loadPersistedDiagnosticsState,
	savePersistedDiagnosticsState,
} from "../../src/state/monitorDiagnosticsPersistence";

const LOCAL_STORAGE_KEY = "tcm:monitor-diagnostics:v1";

// In-memory localStorage mock; IndexedDB intentionally absent so the module
// takes the localStorage fallback branch end-to-end.
class MemoryStorage {
	private map = new Map<string, string>();
	getItem(k: string) {
		return this.map.has(k) ? (this.map.get(k) as string) : null;
	}
	setItem(k: string, v: string) {
		this.map.set(k, v);
	}
	removeItem(k: string) {
		this.map.delete(k);
	}
	clear() {
		this.map.clear();
	}
	key(_: number) {
		return null;
	}
	get length() {
		return this.map.size;
	}
}

beforeEach(() => {
	(globalThis as any).localStorage = new MemoryStorage();
	delete (globalThis as any).indexedDB;
});

describe("monitorDiagnosticsPersistence (localStorage fallback)", () => {
	it("returns null when no persisted state exists", async () => {
		const result = await loadPersistedDiagnosticsState();
		expect(result).toBeNull();
	});

	it("round-trips a valid state through localStorage", async () => {
		await savePersistedDiagnosticsState({
			history: [{ id: "h1", ts: 1000, command: "status", ok: true, response: "OK" }],
			frameSnapshots: [
				{ id: "s1", ts: 2000, frameCount: 42, busFilter: "all", frameFilter: "" },
			],
			archive: [
				{
					id: "a1",
					ts: 3000,
					category: "command",
					summary: "lock",
					detail: "ok",
					ok: true,
				},
			],
		});

		const loaded = await loadPersistedDiagnosticsState();
		expect(loaded).not.toBeNull();
		expect(loaded!.version).toBe(1);
		expect(loaded!.history).toHaveLength(1);
		expect(loaded!.history[0].command).toBe("status");
		expect(loaded!.frameSnapshots[0].frameCount).toBe(42);
		expect(loaded!.archive[0].category).toBe("command");
	});

	it("coerces missing/invalid fields with safe defaults on load", async () => {
		(globalThis as any).localStorage.setItem(
			LOCAL_STORAGE_KEY,
			JSON.stringify({
				history: [{ id: "h1" }, "not-an-object", null],
				frameSnapshots: "garbage",
				archive: [{ id: "a1", category: "bogus", ok: "yes" }, { category: "snapshot" }],
			}),
		);

		const loaded = await loadPersistedDiagnosticsState();
		expect(loaded).not.toBeNull();
		expect(loaded!.history).toHaveLength(1);
		expect(loaded!.history[0].command).toBe("unknown");
		expect(loaded!.history[0].ok).toBe(false);
		expect(loaded!.frameSnapshots).toEqual([]);
		expect(loaded!.archive).toHaveLength(2);
		expect(loaded!.archive[0].category).toBe("system"); // bogus → system
		expect(loaded!.archive[0].ok).toBe(true); // "yes" → truthy
		expect(loaded!.archive[1].category).toBe("snapshot");
	});

	it("returns null when localStorage value is unparseable", async () => {
		(globalThis as any).localStorage.setItem(LOCAL_STORAGE_KEY, "{not-json");
		const loaded = await loadPersistedDiagnosticsState();
		expect(loaded).toBeNull();
	});

	it("caps history at 200 and archive at 400 entries", async () => {
		const bigHistory = Array.from({ length: 250 }, (_, i) => ({
			id: `h${i}`,
			ts: i,
			command: "x",
			ok: true,
			response: "",
		}));
		const bigArchive = Array.from({ length: 500 }, (_, i) => ({
			id: `a${i}`,
			ts: i,
			category: "system" as const,
			summary: "",
			detail: "",
			ok: true,
		}));

		await savePersistedDiagnosticsState({
			history: bigHistory,
			frameSnapshots: [],
			archive: bigArchive,
		});

		const loaded = await loadPersistedDiagnosticsState();
		expect(loaded!.history).toHaveLength(200);
		expect(loaded!.archive).toHaveLength(400);
	});

	it("does not throw when localStorage is unavailable", async () => {
		delete (globalThis as any).localStorage;
		await expect(
			savePersistedDiagnosticsState({ history: [], frameSnapshots: [], archive: [] }),
		).resolves.toBeUndefined();
		await expect(loadPersistedDiagnosticsState()).resolves.toBeNull();
	});
});
