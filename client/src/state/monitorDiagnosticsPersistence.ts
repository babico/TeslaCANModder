export interface PersistedHistoryEntry {
  id: string;
  ts: number;
  command: string;
  ok: boolean;
  response: string;
}

export interface PersistedFrameSnapshot {
  id: string;
  ts: number;
  frameCount: number;
  busFilter: string;
  frameFilter: string;
}

export interface PersistedDiagnosticsArchiveEntry {
  id: string;
  ts: number;
  category: "system" | "command" | "board" | "snapshot";
  summary: string;
  detail: string;
  ok: boolean;
}

export interface PersistedDiagnosticsState {
  version: 1;
  history: PersistedHistoryEntry[];
  frameSnapshots: PersistedFrameSnapshot[];
  archive: PersistedDiagnosticsArchiveEntry[];
}

const DB_NAME = "tesla-can-mod-client";
const STORE_NAME = "monitor-diagnostics";
const RECORD_KEY = "state-v1";
const LOCAL_STORAGE_KEY = "tcm:monitor-diagnostics:v1";
type ArchiveCategory = PersistedDiagnosticsArchiveEntry["category"];

function hasIndexedDb(): boolean {
  return typeof globalThis !== "undefined" && typeof (globalThis as { indexedDB?: unknown }).indexedDB !== "undefined";
}

function hasLocalStorage(): boolean {
  return typeof globalThis !== "undefined" && typeof (globalThis as { localStorage?: unknown }).localStorage !== "undefined";
}

function sanitizeHistory(input: unknown): PersistedHistoryEntry[] {
  if (!Array.isArray(input)) {
    return [];
  }
  return input
    .filter((item) => item && typeof item === "object")
    .map((item) => {
      const row = item as Record<string, unknown>;
      return {
        id: String(row.id ?? `${Date.now()}-history`),
        ts: Number(row.ts ?? Date.now()),
        command: String(row.command ?? "unknown"),
        ok: Boolean(row.ok),
        response: String(row.response ?? ""),
      };
    })
    .slice(0, 200);
}

function sanitizeSnapshots(input: unknown): PersistedFrameSnapshot[] {
  if (!Array.isArray(input)) {
    return [];
  }
  return input
    .filter((item) => item && typeof item === "object")
    .map((item) => {
      const row = item as Record<string, unknown>;
      return {
        id: String(row.id ?? `${Date.now()}-snapshot`),
        ts: Number(row.ts ?? Date.now()),
        frameCount: Number(row.frameCount ?? 0),
        busFilter: String(row.busFilter ?? "all"),
        frameFilter: String(row.frameFilter ?? ""),
      };
    })
    .slice(0, 200);
}

function sanitizeArchive(input: unknown): PersistedDiagnosticsArchiveEntry[] {
  if (!Array.isArray(input)) {
    return [];
  }
  return input
    .filter((item) => item && typeof item === "object")
    .map((item) => {
      const row = item as Record<string, unknown>;
      const rawCategory = String(row.category ?? "system");
      const category: ArchiveCategory =
        rawCategory === "command" || rawCategory === "board" || rawCategory === "snapshot"
          ? rawCategory
          : "system";
      return {
        id: String(row.id ?? `${Date.now()}-archive`),
        ts: Number(row.ts ?? Date.now()),
        category,
        summary: String(row.summary ?? ""),
        detail: String(row.detail ?? ""),
        ok: Boolean(row.ok),
      };
    })
    .slice(0, 400);
}

function sanitizeState(input: unknown): PersistedDiagnosticsState {
  const row = (input && typeof input === "object" ? input : {}) as Record<string, unknown>;
  return {
    version: 1,
    history: sanitizeHistory(row.history),
    frameSnapshots: sanitizeSnapshots(row.frameSnapshots),
    archive: sanitizeArchive(row.archive),
  };
}

function openDb(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const indexedDb = (globalThis as { indexedDB?: IDBFactory }).indexedDB;
    if (!indexedDb) {
      reject(new Error("indexedDB unavailable"));
      return;
    }

    const request = indexedDb.open(DB_NAME, 1);
    request.onupgradeneeded = () => {
      const db = request.result;
      if (!db.objectStoreNames.contains(STORE_NAME)) {
        db.createObjectStore(STORE_NAME);
      }
    };
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error ?? new Error("failed to open indexedDB"));
  });
}

async function loadFromIndexedDb(): Promise<PersistedDiagnosticsState | null> {
  const db = await openDb();
  try {
    const result = await new Promise<unknown>((resolve, reject) => {
      const tx = db.transaction(STORE_NAME, "readonly");
      const store = tx.objectStore(STORE_NAME);
      const request = store.get(RECORD_KEY);
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error ?? new Error("indexedDB read failed"));
    });
    if (!result) {
      return null;
    }
    return sanitizeState(result);
  } finally {
    db.close();
  }
}

async function saveToIndexedDb(state: PersistedDiagnosticsState): Promise<void> {
  const db = await openDb();
  try {
    await new Promise<void>((resolve, reject) => {
      const tx = db.transaction(STORE_NAME, "readwrite");
      const store = tx.objectStore(STORE_NAME);
      const request = store.put(state, RECORD_KEY);
      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error ?? new Error("indexedDB write failed"));
    });
  } finally {
    db.close();
  }
}

function loadFromLocalStorage(): PersistedDiagnosticsState | null {
  if (!hasLocalStorage()) {
    return null;
  }
  try {
    const raw = (globalThis as { localStorage?: Storage }).localStorage?.getItem(LOCAL_STORAGE_KEY);
    if (!raw) {
      return null;
    }
    const parsed = JSON.parse(raw) as unknown;
    return sanitizeState(parsed);
  } catch {
    return null;
  }
}

function saveToLocalStorage(state: PersistedDiagnosticsState): void {
  if (!hasLocalStorage()) {
    return;
  }
  try {
    (globalThis as { localStorage?: Storage }).localStorage?.setItem(LOCAL_STORAGE_KEY, JSON.stringify(state));
  } catch {
    // Ignore quota/serialization failures on best-effort local fallback.
  }
}

export async function loadPersistedDiagnosticsState(): Promise<PersistedDiagnosticsState | null> {
  if (hasIndexedDb()) {
    try {
      const indexed = await loadFromIndexedDb();
      if (indexed) {
        return indexed;
      }
    } catch {
      // Fallback to localStorage below.
    }
  }
  return loadFromLocalStorage();
}

export async function savePersistedDiagnosticsState(input: Omit<PersistedDiagnosticsState, "version">): Promise<void> {
  const state: PersistedDiagnosticsState = {
    version: 1,
    history: sanitizeHistory(input.history),
    frameSnapshots: sanitizeSnapshots(input.frameSnapshots),
    archive: sanitizeArchive(input.archive),
  };

  if (hasIndexedDb()) {
    try {
      await saveToIndexedDb(state);
      return;
    } catch {
      // Fallback to localStorage below.
    }
  }

  saveToLocalStorage(state);
}
