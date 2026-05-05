import type { CanFrame } from "@teslacanmodder/protocol";
import { hasIndexedDb, writeIndexedDbRecord } from "./indexedDbStore";

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

	await writeIndexedDbRecord(DB_NAME, STORE_NAME, RECORD_KEY, payload);
}
