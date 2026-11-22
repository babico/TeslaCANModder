import { readFileSync } from "node:fs";

/**
 * Read-only loader for Sentry USB's `drive-data.json`, used for best-effort
 * clip<->drive correlation.
 *
 * Boundaries (deliberate):
 * - Reads one file. Never writes, never creates directories, never touches
 *   Sentry's storage or configuration.
 * - No Sentry code is copied or mirrored here (Sentry USB is PolyForm
 *   Noncommercial 1.0). This is a plain JSON consumer.
 * - The real `drive-data.json` schema is UNVERIFIED. No field names are
 *   invented: joining is driven entirely by caller-supplied key hints, and
 *   with no hints the correlator returns no links.
 * - No CAN TX, no hardware access.
 */

/** Outcome of a read-only load attempt. */
export type DriveDataStatus = "loaded" | "missing" | "invalid";

export interface DriveDataLoad {
	readonly status: DriveDataStatus;
	/** Path that was read, echoed for diagnostics. */
	readonly path: string;
	/** Parsed JSON left untouched; `null` when missing or invalid. */
	readonly data: unknown;
	/** True only when the file was read and parsed. */
	readonly available: boolean;
	/** Degradation reason; absent on success. */
	readonly reason?: string;
}

export interface CorrelationOptions {
	/** Drive-record key holding its id. UNVERIFIED — supply from the real schema. */
	readonly driveIdKey?: string;
	/** Clip-record key referencing a drive id. UNVERIFIED — supply from the real schema. */
	readonly clipDriveKey?: string;
	/** Recursion limit when scanning for arrays. Defaults to 4. */
	readonly maxDepth?: number;
}

/** A clip record paired with the drive record it references. */
export interface ClipDriveLink {
	readonly clip: unknown;
	readonly drive: unknown;
	/** The clip-side key used to join; UNVERIFIED until schema is confirmed. */
	readonly via: string;
}

type NodeError = { code?: string; message?: string };

// Duck typing on purpose: native fs errors can come from another realm under
// Jest's VM modules, where `err instanceof Error` is false.
function asNodeError(err: unknown): NodeError | undefined {
	return typeof err === "object" && err !== null ? (err as NodeError) : undefined;
}

function errorMessage(err: unknown): string {
	return asNodeError(err)?.message ?? String(err);
}

/**
 * Load `drive-data.json` without ever writing to it.
 *
 * Missing files and malformed JSON degrade to a structured result rather than
 * throwing, so a partially provisioned Pi still boots.
 */
export function readDriveData(path: string | URL): DriveDataLoad {
	const echoed = String(path);
	let raw: string;
	try {
		raw = readFileSync(path, "utf8");
	} catch (err) {
		const missing = asNodeError(err)?.code === "ENOENT";
		return {
			status: missing ? "missing" : "invalid",
			path: echoed,
			data: null,
			available: false,
			reason: missing
				? "missing: drive-data.json not found"
				: `read failed: ${errorMessage(err)}`,
		};
	}

	try {
		return {
			status: "loaded",
			path: echoed,
			data: JSON.parse(raw) as unknown,
			available: true,
		};
	} catch (err) {
		return {
			status: "invalid",
			path: echoed,
			data: null,
			available: false,
			reason: `invalid JSON: ${errorMessage(err)}`,
		};
	}
}

function isRecord(value: unknown): value is Record<string, unknown> {
	return value !== null && typeof value === "object" && !Array.isArray(value);
}

function isJoinKey(value: unknown): value is string | number {
	return typeof value === "string" || typeof value === "number";
}

/** Collect arrays of object records anywhere in the parsed JSON, bounded by depth. */
function collectRecordArrays(
	data: unknown,
	maxDepth: number,
): Array<Array<Record<string, unknown>>> {
	const found: Array<Array<Record<string, unknown>>> = [];
	const seen = new Set<object>();

	function walk(node: unknown, depth: number): void {
		if (depth > maxDepth || node === null || typeof node !== "object" || seen.has(node)) return;
		seen.add(node);

		if (Array.isArray(node)) {
			const records = node.filter(isRecord);
			if (records.length > 0) found.push(records);
			for (const item of node) walk(item, depth + 1);
		} else {
			for (const child of Object.values(node as Record<string, unknown>))
				walk(child, depth + 1);
		}
	}

	walk(data, 0);
	return found;
}

/**
 * Best-effort clip<->drive pairing over arbitrary JSON.
 *
 * Both join keys must be supplied by the caller: the Sentry USB schema is
 * UNVERIFIED, so this function invents no field names. Unknown shapes,
 * missing keys and non-object records simply produce no links.
 */
export function correlateClipsToDrives(
	data: unknown,
	options: CorrelationOptions = {},
): ClipDriveLink[] {
	const driveKey = options.driveIdKey;
	const clipKey = options.clipDriveKey;
	if (!driveKey || !clipKey) return [];

	const arrays = collectRecordArrays(data, options.maxDepth ?? 4);
	const drivesById = new Map<string | number, Record<string, unknown>>();
	for (const records of arrays) {
		for (const record of records) {
			const id = record[driveKey];
			if (isJoinKey(id) && !drivesById.has(id)) drivesById.set(id, record);
		}
	}
	if (drivesById.size === 0) return [];

	const links: ClipDriveLink[] = [];
	const seenClips = new Set<unknown>();
	for (const records of arrays) {
		for (const clip of records) {
			const ref = clip[clipKey];
			if (!isJoinKey(ref) || seenClips.has(clip)) continue;
			const drive = drivesById.get(ref);
			if (!drive) continue;
			seenClips.add(clip);
			links.push({ clip, drive, via: clipKey });
		}
	}
	return links;
}
