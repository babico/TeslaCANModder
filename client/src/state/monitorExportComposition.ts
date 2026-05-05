/**
 * Pure helpers for composing monitor export metadata, checksums, and validation.
 * Encapsulates the logic for building consistent export provenance and row formats.
 */

import type { BoardState, CanFrame } from "@teslacanmodder/protocol";

export interface ExportRow {
	ts: string;
	bus: number;
	busName: string;
	idHex: string;
	dlc: number;
	dir: string;
	data: string;
}

export interface ExportMetadata {
	schemaVersion: string;
	dataset: {
		id: string;
		label: string;
		source: {
			vehicle: string;
			firmware: string;
			mcu: string;
			soc: string;
		};
	};
	boardState: BoardState;
	bus: string;
	textFilter: string;
	frameWindowSize: number;
	frameSampleStep: number;
	decodeEnabled: boolean;
	feedPaused: boolean;
	filteredFrames: number;
	renderedFrames: number;
	snapshots: number;
	commandHistory: number;
	notifications: number;
	exportedAt?: string;
	platform?: {
		variant: string;
		hardware: string;
		board: string;
	};
}

export interface BuildExportRowsInput {
	frames: CanFrame[];
}

/**
 * Converts CanFrame array into export row format with formatted timestamps, bus names, and hex IDs.
 */
export function buildExportRows(input: BuildExportRowsInput): ExportRow[] {
	return input.frames.map((frame) => ({
		ts: frame.ts,
		bus: frame.bus,
		busName: frame.busName,
		idHex: `0x${frame.id.toString(16).toUpperCase()}`,
		dlc: frame.dlc,
		dir: frame.dir,
		data: frame.data,
	}));
}

/**
 * Computes a FNV-1a hash of export rows for integrity checking.
 * Uses the pipe-delimited row format: ts|bus|idHex|dlc|dir|data
 */
export function computeExportChecksum(rows: ExportRow[]): number {
	let hash = 2166136261;
	for (const row of rows) {
		const line = `${row.ts}|${row.bus}|${row.idHex}|${row.dlc}|${row.dir}|${row.data}`;
		for (let i = 0; i < line.length; i += 1) {
			hash ^= line.charCodeAt(i);
			hash = Math.imul(hash, 16777619);
		}
	}
	return hash >>> 0;
}

/**
 * Verifies export integrity by checking row count and checksum.
 * Returns true only if both match expected values.
 */
export function verifyExportIntegrity(
	rows: ExportRow[],
	expectedChecksum: number,
	expectedCount: number,
): boolean {
	if (rows.length !== expectedCount) {
		return false;
	}
	return computeExportChecksum(rows) === expectedChecksum;
}

export interface FormatExportRowsAsCsvInput {
	rows: ExportRow[];
	metadata: ExportMetadata;
}

/**
 * Formats export rows as CSV with metadata header.
 * Includes schema version, export timestamp, dataset info, and filter/window configuration.
 */
export function formatExportRowsAsCsv(input: FormatExportRowsAsCsvInput): string {
	const { rows, metadata } = input;
	const checksum = computeExportChecksum(rows);
	const rowCount = rows.length;

	const header = "ts,bus,busName,idHex,dlc,dir,data";
	const body = rows
		.map((frame) =>
			[frame.ts, frame.bus, frame.busName, frame.idHex, frame.dlc, frame.dir, frame.data]
				.map((part) => `"${String(part).replace(/"/g, '""')}"`)
				.join(","),
		)
		.join("\n");

	const meta = [
		`# schema=${metadata.schemaVersion}`,
		`# exportedAt=${metadata.exportedAt ?? new Date().toISOString()}`,
		`# dataset=${metadata.dataset.id}`,
		`# platform=${metadata.platform?.variant}/${metadata.platform?.hardware}/${metadata.platform?.board}`,
		`# rows=${rowCount} checksum=${checksum} bus=${metadata.bus} filter="${metadata.textFilter}"`,
	].join("\n");

	return `${meta}\n${header}\n${body}`;
}

/**
 * Checks if a row format conversion is valid for a given mode.
 * Mode can be "csv", "json", "raw-json", "raw-jsonl", or "decoded".
 */
export function validateExportMode(mode: string): boolean {
	const validModes = ["csv", "json", "raw-json", "raw-jsonl", "decoded"];
	return validModes.includes(mode);
}

export interface ComputeExportStatisticsInput {
	rowCount: number;
	checksum: number;
	valid: boolean;
	mode: string;
	schemaVersion: string;
}

/**
 * Builds a summary message for a completed export operation.
 */
export function buildExportSummary(input: ComputeExportStatisticsInput): string {
	const modeText =
		input.mode === "json" ? "JSON" : input.mode === "csv" ? "CSV" : input.mode.toUpperCase();
	return `Exported ${input.rowCount} visible frames as ${modeText} (${input.schemaVersion}, checksum=${input.checksum}, valid=${input.valid}).`;
}

export interface BuildExportHistoryEntryInput {
	command: string;
	rowCount: number;
	checksum: number;
	schemaVersion: string;
	valid: boolean;
}

/**
 * Builds a history entry for export operations.
 * Used to track all export actions in monitoring session.
 */
export function buildExportHistoryEntry(input: BuildExportHistoryEntryInput): {
	command: string;
	ok: boolean;
	response: string;
} {
	return {
		command: input.command,
		ok: input.valid,
		response: `rows=${input.rowCount} checksum=${input.checksum} schema=${input.schemaVersion}`,
	};
}
