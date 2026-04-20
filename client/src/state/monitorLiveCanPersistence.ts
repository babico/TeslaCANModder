import type { CanFrame } from "@teslacanmodder/protocol";

export interface PersistedLiveCanFrame {
  key: string;
  id: number;
  bus: number;
  busName: string;
  dlc: number;
  dir: string;
  data: string;
  ts: string;
}

export interface PersistedLiveCanSession {
  version: 1;
  updatedAt: number;
  frameCount: number;
  frames: PersistedLiveCanFrame[];
}

const DB_NAME = "tesla-can-mod-live-can";
const STORE_NAME = "monitor-live-can";
const RECORD_KEY = "session-v1";
const MAX_PERSISTED_FRAMES = 5000;

function hasIndexedDb(): boolean {
  return typeof globalThis !== "undefined" && typeof (globalThis as { indexedDB?: unknown }).indexedDB !== "undefined";
}

function openDb(): Promise<any> {
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

function sanitizeFrame(frame: CanFrame): PersistedLiveCanFrame {
  return {
    key: String(frame.key ?? ""),
    id: Number(frame.id ?? 0),
    bus: Number(frame.bus ?? 0),
    busName: String(frame.busName ?? ""),
    dlc: Number(frame.dlc ?? 0),
    dir: String(frame.dir ?? "rx"),
    data: String(frame.data ?? ""),
    ts: String(frame.ts ?? ""),
  };
}

export async function saveLiveCanFramesToIndexedDb(frames: CanFrame[]): Promise<void> {
  if (!hasIndexedDb()) {
    return;
  }

  const safeFrames = Array.isArray(frames)
    ? frames.slice(-MAX_PERSISTED_FRAMES).map(sanitizeFrame)
    : [];

  const payload: PersistedLiveCanSession = {
    version: 1,
    updatedAt: Date.now(),
    frameCount: safeFrames.length,
    frames: safeFrames,
  };

  const db = await openDb();
  try {
    await new Promise<void>((resolve, reject) => {
      const tx = db.transaction(STORE_NAME, "readwrite");
      const store = tx.objectStore(STORE_NAME);
      const request = store.put(payload, RECORD_KEY);
      request.onsuccess = () => resolve();
      request.onerror = () => reject(request.error ?? new Error("indexedDB write failed"));
    });
  } finally {
    db.close();
  }
}
