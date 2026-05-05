// ── monitorLiveCanPersistence Tests ──────────────────────────────────────────
// Exercises the saveLiveCanFramesToIndexedDb writer with a hand-rolled
// in-memory IndexedDB. Verifies sanitization, the 5000-frame cap, and the
// no-op behavior when IndexedDB is unavailable.

import type { CanFrame } from "@teslacanmodder/protocol";
import { saveLiveCanFramesToIndexedDb } from "../../src/state/monitorLiveCanPersistence";

const DB_NAME = "tesla-can-mod-live-can";
const STORE_NAME = "monitor-live-can";
const RECORD_KEY = "session-v1";

// ── Minimal IndexedDB shim ───────────────────────────────────────────────────
class FakeRequest<T = unknown> {
	result: T = undefined as any;
	error: Error | null = null;
	onsuccess: (() => void) | null = null;
	onerror: (() => void) | null = null;
	onupgradeneeded: (() => void) | null = null;
	resolve(value: T) {
		this.result = value;
		queueMicrotask(() => this.onsuccess?.());
	}
	reject(err: Error) {
		this.error = err;
		queueMicrotask(() => this.onerror?.());
	}
}

class FakeObjectStore {
	constructor(public store: Map<string, unknown>) {}
	put(value: unknown, key: string) {
		const req = new FakeRequest<undefined>();
		this.store.set(key, value);
		req.resolve(undefined);
		return req;
	}
	get(key: string) {
		const req = new FakeRequest<unknown>();
		req.resolve(this.store.get(key));
		return req;
	}
}

class FakeTransaction {
	constructor(private store: Map<string, unknown>) {}
	objectStore(_: string) {
		return new FakeObjectStore(this.store);
	}
}

class FakeDatabase {
	objectStoreNames = {
		contains: (_: string) => true,
	};
	constructor(public stores: Map<string, Map<string, unknown>>) {}
	transaction(name: string, _mode: string) {
		if (!this.stores.has(name)) this.stores.set(name, new Map());
		return new FakeTransaction(this.stores.get(name)!);
	}
	close() {}
	createObjectStore(name: string) {
		this.stores.set(name, new Map());
	}
}

class FakeIndexedDB {
	databases = new Map<string, Map<string, Map<string, unknown>>>();
	open(name: string, _version: number) {
		const req = new FakeRequest<FakeDatabase>();
		const isNew = !this.databases.has(name);
		if (isNew) this.databases.set(name, new Map());
		const db = new FakeDatabase(this.databases.get(name)!);
		queueMicrotask(() => {
			req.result = db;
			if (isNew) req.onupgradeneeded?.();
			req.resolve(db);
		});
		return req;
	}
}

let fakeIdb: FakeIndexedDB;

beforeEach(() => {
	fakeIdb = new FakeIndexedDB();
	(globalThis as any).indexedDB = fakeIdb;
});

afterEach(() => {
	delete (globalThis as any).indexedDB;
});

function makeFrame(overrides: Partial<CanFrame> = {}): CanFrame {
	return {
		key: "k1",
		id: 0x100,
		bus: 0,
		busName: "chassis",
		dlc: 8,
		dir: "rx",
		data: "00 11 22 33 44 55 66 77",
		ts: "2026-01-01T00:00:00Z",
		...(overrides as any),
	} as CanFrame;
}

describe("monitorLiveCanPersistence", () => {
	it("writes a sanitized payload to the live-can store", async () => {
		await saveLiveCanFramesToIndexedDb([makeFrame({ id: 0x222 })]);
		const stored = fakeIdb.databases.get(DB_NAME)?.get(STORE_NAME)?.get(RECORD_KEY) as any;
		expect(stored.version).toBe(1);
		expect(stored.frameCount).toBe(1);
		expect(stored.frames[0].id).toBe(0x222);
		expect(typeof stored.updatedAt).toBe("number");
	});

	it("coerces missing fields with safe defaults", async () => {
		await saveLiveCanFramesToIndexedDb([{ key: "" } as any as CanFrame]);
		const stored = fakeIdb.databases.get(DB_NAME)?.get(STORE_NAME)?.get(RECORD_KEY) as any;
		expect(stored.frames[0]).toEqual({
			key: "",
			id: 0,
			bus: 0,
			busName: "",
			dlc: 0,
			dir: "rx",
			data: "",
			ts: "",
		});
	});

	it("caps persisted frames at 5000 (keeps the most recent)", async () => {
		const frames = Array.from({ length: 5500 }, (_, i) => makeFrame({ key: `k${i}`, id: i }));
		await saveLiveCanFramesToIndexedDb(frames);
		const stored = fakeIdb.databases.get(DB_NAME)?.get(STORE_NAME)?.get(RECORD_KEY) as any;
		expect(stored.frames).toHaveLength(5000);
		// last-5000 slice → first kept frame is index 500
		expect(stored.frames[0].id).toBe(500);
		expect(stored.frames[stored.frames.length - 1].id).toBe(5499);
	});

	it("is a no-op when IndexedDB is unavailable", async () => {
		delete (globalThis as any).indexedDB;
		await expect(saveLiveCanFramesToIndexedDb([makeFrame()])).resolves.toBeUndefined();
	});

	it("handles an empty/non-array input safely", async () => {
		await saveLiveCanFramesToIndexedDb(null as any);
		const stored = fakeIdb.databases.get(DB_NAME)?.get(STORE_NAME)?.get(RECORD_KEY) as any;
		expect(stored.frameCount).toBe(0);
		expect(stored.frames).toEqual([]);
	});
});
