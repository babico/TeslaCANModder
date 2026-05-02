/**
 * Pure helpers for managing monitor history and frame snapshots.
 * Encapsulates logic for building, filtering, and managing session records.
 */

import type { CanFrame } from "@teslacanmodder/protocol";

/**
 * Represents a single command execution history entry.
 */
export interface HistoryEntry {
	id: string;
	ts: number;
	command: string;
	ok: boolean;
	response: string;
}

/**
 * Represents a snapshot of frame state at a point in time.
 */
export interface FrameSnapshot {
	id: string;
	ts: number;
	frameCount: number;
	busFilter: string;
	frameFilter: string;
}

export interface BuildHistoryEntryInput {
	command: string;
	ok: boolean;
	response: string;
}

/**
 * Builds a new history entry with timestamp and unique ID.
 * ID is generated from timestamp and random suffix to ensure uniqueness.
 */
export function buildHistoryEntry(input: BuildHistoryEntryInput): HistoryEntry {
	return {
		id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
		ts: Date.now(),
		command: input.command,
		ok: input.ok,
		response: input.response,
	};
}

export interface AddHistoryEntryInput {
	current: HistoryEntry[];
	entry: HistoryEntry;
	maxLength?: number;
}

/**
 * Adds a new history entry to the beginning of the history list.
 * Trims history to max length if provided.
 * Returns the new history array (immutable pattern).
 */
export function addHistoryEntry(input: AddHistoryEntryInput): HistoryEntry[] {
	const maxLength = input.maxLength ?? 20;
	return [input.entry, ...input.current].slice(0, maxLength);
}

export interface PushHistoryInput {
	current: HistoryEntry[];
	command: string;
	ok: boolean;
	response: string;
	maxLength?: number;
}

/**
 * Convenience function that builds and adds a history entry in one operation.
 */
export function pushHistory(input: PushHistoryInput): HistoryEntry[] {
	const entry = buildHistoryEntry({
		command: input.command,
		ok: input.ok,
		response: input.response,
	});
	return addHistoryEntry({
		current: input.current,
		entry,
		maxLength: input.maxLength,
	});
}

export interface BuildFrameSnapshotInput {
	busFilter: string;
	frameFilter: string;
	frameCount: number;
}

/**
 * Builds a new frame snapshot with timestamp and unique ID.
 * Captures the current state of the monitor frame view.
 */
export function buildFrameSnapshot(input: BuildFrameSnapshotInput): FrameSnapshot {
	return {
		id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
		ts: Date.now(),
		frameCount: input.frameCount,
		busFilter: input.busFilter,
		frameFilter: input.frameFilter.trim(),
	};
}

export interface AddFrameSnapshotInput {
	current: FrameSnapshot[];
	snapshot: FrameSnapshot;
	maxLength?: number;
}

/**
 * Adds a new frame snapshot to the beginning of the snapshots list.
 * Trims snapshots to max length if provided.
 * Returns the new snapshots array (immutable pattern).
 */
export function addFrameSnapshot(input: AddFrameSnapshotInput): FrameSnapshot[] {
	const maxLength = input.maxLength ?? 20;
	return [input.snapshot, ...input.current].slice(0, maxLength);
}

export interface SaveFrameSnapshotInput {
	current: FrameSnapshot[];
	busFilter: string;
	frameFilter: string;
	frameCount: number;
	maxLength?: number;
}

/**
 * Convenience function that builds and adds a frame snapshot in one operation.
 */
export function saveFrameSnapshot(input: SaveFrameSnapshotInput): FrameSnapshot[] {
	const snapshot = buildFrameSnapshot({
		busFilter: input.busFilter,
		frameFilter: input.frameFilter,
		frameCount: input.frameCount,
	});
	return addFrameSnapshot({
		current: input.current,
		snapshot,
		maxLength: input.maxLength,
	});
}

export interface FilterHistoryByCommandInput {
	history: HistoryEntry[];
	query: string;
}

/**
 * Filters history entries by command name (case-insensitive substring match).
 */
export function filterHistoryByCommand(input: FilterHistoryByCommandInput): HistoryEntry[] {
	if (!input.query.trim()) {
		return input.history;
	}
	const normalized = input.query.trim().toLowerCase();
	return input.history.filter((entry) => entry.command.toLowerCase().includes(normalized));
}

export interface GetHistorySuccessRateInput {
	history: HistoryEntry[];
}

/**
 * Computes the success rate of commands in history (0-100).
 * Returns 0 for empty history.
 */
export function getHistorySuccessRate(input: GetHistorySuccessRateInput): number {
	if (input.history.length === 0) {
		return 0;
	}
	const successes = input.history.filter((e) => e.ok).length;
	return Math.round((successes / input.history.length) * 100);
}

export interface GetRecentHistoryInput {
	history: HistoryEntry[];
	count?: number;
}

/**
 * Returns the most recent N history entries.
 * Defaults to 10 entries if not specified.
 */
export function getRecentHistory(input: GetRecentHistoryInput): HistoryEntry[] {
	const count = input.count ?? 10;
	return input.history.slice(0, count);
}

export interface GetHistoryStatsInput {
	history: HistoryEntry[];
}

export interface HistoryStats {
	total: number;
	successful: number;
	failed: number;
	successRate: number;
}

/**
 * Computes aggregate statistics for command history.
 */
export function getHistoryStats(input: GetHistoryStatsInput): HistoryStats {
	const successful = input.history.filter((e) => e.ok).length;
	const failed = input.history.length - successful;
	return {
		total: input.history.length,
		successful,
		failed,
		successRate: getHistorySuccessRate({ history: input.history }),
	};
}
