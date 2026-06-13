/** JSON line parser — extracts JSON messages from noisy serial input. */

import type { ParsedEvent } from "./types.js";

type JsonRecord = Record<string, unknown>;

function isJsonRecord(value: unknown): value is JsonRecord {
	return typeof value === "object" && value !== null && !Array.isArray(value);
}

function assignIfDefined(target: JsonRecord, key: string, value: unknown): void {
	if (value !== undefined) {
		target[key] = value;
	}
}

function mergeObjectField(target: JsonRecord, key: string, value: JsonRecord | undefined): void {
	if (!value) {
		return;
	}

	target[key] = {
		...(isJsonRecord(target[key]) ? target[key] : {}),
		...value,
	};
}

function normalizeCanHealth(value: unknown): JsonRecord | undefined {
	if (!isJsonRecord(value)) {
		return undefined;
	}

	const normalized: JsonRecord = {};
	for (const key of ["chassis", "vehicle", "body"]) {
		const bus = value[key];
		if (isJsonRecord(bus)) {
			normalized[key] = bus;
		}
	}

	return Object.keys(normalized).length > 0 ? normalized : undefined;
}

function normalizeBusMetrics(value: unknown): JsonRecord | undefined {
	if (!isJsonRecord(value)) {
		return undefined;
	}

	const normalized: JsonRecord = {};
	for (const key of ["chassis", "vehicle", "body"]) {
		const busValue = value[key];
		if (typeof busValue === "number" && Number.isFinite(busValue)) {
			normalized[key] = busValue;
		}
	}

	return Object.keys(normalized).length > 0 ? normalized : undefined;
}

function normalizeBooleanLike(value: unknown): unknown {
	if (value === 0 || value === 1) {
		return Boolean(value);
	}

	return value;
}

function normalizeFeatureFlags(features: JsonRecord | undefined): JsonRecord | undefined {
	if (!features) {
		return undefined;
	}

	const normalized: JsonRecord = {};
	for (const [key, value] of Object.entries(features)) {
		normalized[key] = normalizeBooleanLike(value);
	}

	return normalized;
}

function normalizeConnectivitySection(
	target: JsonRecord,
	connectivity: JsonRecord | undefined,
): void {
	assignIfDefined(target, "vehicleOnline", connectivity?.vehicleOnline);
	assignIfDefined(target, "bodyOnline", connectivity?.bodyOnline);
	assignIfDefined(target, "chassisOnline", connectivity?.chassisOnline);
	assignIfDefined(target, "standby", connectivity?.standby);

	const bus = isJsonRecord(connectivity?.bus) ? connectivity.bus : undefined;
	assignIfDefined(target, "busChassis", bus?.chassis);
	assignIfDefined(target, "busVehicle", bus?.vehicle);
	assignIfDefined(target, "busBody", bus?.body);
}

function normalizeStateSection(target: JsonRecord, state: JsonRecord | undefined): void {
	assignIfDefined(target, "fsd", state?.fsd);
	assignIfDefined(target, "fsdForce", state?.fsdForce);
	assignIfDefined(target, "nag", state?.nag);
	assignIfDefined(target, "isaChime", state?.isaChime);
	assignIfDefined(target, "summonInject", state?.summonInject);
	assignIfDefined(target, "nagKiller", state?.nagKiller);
	assignIfDefined(target, "nagMode", state?.nagMode);
	assignIfDefined(target, "dasHandsOn", state?.dasHandsOn);
	assignIfDefined(target, "precondition", state?.precondition);
	assignIfDefined(target, "trackMode", state?.trackMode);
	assignIfDefined(target, "apGateEnabled", state?.apGateEnabled);
	assignIfDefined(target, "apGateOpen", state?.apGateOpen);
	assignIfDefined(target, "apGateReason", state?.apGateReason);
	assignIfDefined(target, "detectedHW", state?.detectedHW);
	assignIfDefined(target, "variantAutoDetect", state?.variantAutoDetect);
	assignIfDefined(target, "gtwAutopilotTier", state?.gtwAutopilotTier);
	assignIfDefined(target, "gtwAutopilotSeen", state?.gtwAutopilotSeen);
	assignIfDefined(target, "rawCan", state?.rawCan);

	const profile = state?.profile;
	if (isJsonRecord(profile)) {
		assignIfDefined(target, "sp", profile.value);
		assignIfDefined(target, "spPin", profile.pinned);
	} else {
		assignIfDefined(target, "sp", profile);
	}

	const offset = state?.offset;
	if (isJsonRecord(offset)) {
		assignIfDefined(target, "offset", offset.value);
		assignIfDefined(target, "offsetPin", offset.pinned);
	} else {
		assignIfDefined(target, "offset", offset);
	}

	const stream = isJsonRecord(state?.stream) ? state.stream : undefined;
	mergeObjectField(target, "stream", stream);
}

function normalizePlatformSection(target: JsonRecord, platform: JsonRecord | undefined): void {
	assignIfDefined(target, "platformModel", platform?.model);
	assignIfDefined(target, "platformHwGen", platform?.hwGen);
	assignIfDefined(target, "platformSwYear", platform?.swYear);
	assignIfDefined(target, "platformSwWeek", platform?.swWeek);
	assignIfDefined(target, "platformSwRelease", platform?.swRelease);
	assignIfDefined(target, "platformFsdProto", platform?.fsdProto);
	assignIfDefined(target, "platformSwCompat", platform?.swCompat);
	assignIfDefined(target, "platformResolved", platform?.resolved);
}

function normalizeFirmwareSection(target: JsonRecord, firmware: JsonRecord | undefined): void {
	assignIfDefined(target, "fwYear", firmware?.year);
	assignIfDefined(target, "fwRelease", firmware?.release);
	assignIfDefined(target, "fwMinor", firmware?.minor);
	assignIfDefined(target, "fwBuild", firmware?.fwBuild);
	assignIfDefined(target, "fwCompat", firmware?.compat);
	assignIfDefined(target, "hasFwVersion", firmware?.hasVersion);
	assignIfDefined(target, "mqtt", firmware?.mqtt);
	assignIfDefined(target, "mqttConnected", firmware?.mqttConnected);
}

function normalizeBatterySection(target: JsonRecord, battery: JsonRecord | undefined): void {
	assignIfDefined(target, "bmsNomFullPack", battery?.nomFullPack);
	assignIfDefined(target, "bmsNomRemain", battery?.nomRemain);
	assignIfDefined(target, "bmsIdealRemain", battery?.idealRemain);
	assignIfDefined(target, "bmsCellVMax", battery?.cellVMax);
	assignIfDefined(target, "bmsCellVMin", battery?.cellVMin);
	assignIfDefined(target, "bmsMaxRegen", battery?.maxRegen);
	assignIfDefined(target, "bmsMaxDischarge", battery?.maxDischarge);
	assignIfDefined(target, "hasEnhancedBms", battery?.hasEnhanced);
}

function normalizeSafetySection(target: JsonRecord, safety: JsonRecord | undefined): void {
	assignIfDefined(target, "banShield", safety?.banShield);
	assignIfDefined(target, "banThreat", safety?.banThreat);
	assignIfDefined(target, "banDetectCount", safety?.banDetectCount);
	assignIfDefined(target, "gtwShieldArmed", safety?.gtwShieldArmed);
	assignIfDefined(target, "gtwShieldBlocks", safety?.gtwShieldBlocks);
}

function normalizeCanSection(target: JsonRecord, can: JsonRecord | undefined): void {
	assignIfDefined(target, "canClockReqMHz", can?.clockReqMHz);
	assignIfDefined(target, "canClockMHz", can?.clockMHz);

	const clock = isJsonRecord(can?.clock) ? can.clock : undefined;
	assignIfDefined(target, "canClockReqMHz", clock?.reqMHz);
	assignIfDefined(target, "canClockMHz", clock?.activeMHz);

	const canHealth = normalizeCanHealth(can?.health);
	assignIfDefined(target, "canHealth", canHealth);

	assignIfDefined(target, "canNagEchoCount", can?.nagEchoCount);
	assignIfDefined(target, "canEapModCount", can?.eapModCount);
	assignIfDefined(target, "canTxFailCount", can?.txFailCount);
	assignIfDefined(target, "canBusOffCount", can?.busOffCount);

	mergeObjectField(target, "canFrames", normalizeBusMetrics(can?.frames));
	mergeObjectField(target, "canHz", normalizeBusMetrics(can?.hz));
	mergeObjectField(target, "canHzMin", normalizeBusMetrics(can?.hzMin));
	mergeObjectField(target, "canHzMax", normalizeBusMetrics(can?.hzMax));
}

function normalizeFeaturesSection(target: JsonRecord, features: JsonRecord | undefined): void {
	const normalizedFeatures = normalizeFeatureFlags(features);
	mergeObjectField(target, "features", normalizedFeatures);
	assignIfDefined(target, "eap", normalizedFeatures?.eap);
	assignIfDefined(target, "evd", normalizedFeatures?.evd);
	assignIfDefined(target, "tlssc", normalizedFeatures?.tlssc);
}

function normalizeBootStatusMessage(message: JsonRecord): JsonRecord {
	const meta = isJsonRecord(message.meta) ? message.meta : undefined;
	const connectivity = isJsonRecord(message.connectivity) ? message.connectivity : undefined;
	const state = isJsonRecord(message.state) ? message.state : undefined;
	const driverAssist = isJsonRecord(message.driverAssist) ? message.driverAssist : undefined;
	const vehicle = isJsonRecord(message.vehicle) ? message.vehicle : undefined;
	const platform = isJsonRecord(message.platform) ? message.platform : undefined;
	const firmware = isJsonRecord(message.firmware) ? message.firmware : undefined;
	const battery = isJsonRecord(message.battery) ? message.battery : undefined;
	const safety = isJsonRecord(message.safety) ? message.safety : undefined;
	const can = isJsonRecord(message.can) ? message.can : undefined;
	const features = isJsonRecord(message.features) ? message.features : undefined;

	if (
		!meta &&
		!connectivity &&
		!state &&
		!driverAssist &&
		!vehicle &&
		!platform &&
		!firmware &&
		!battery &&
		!safety &&
		!can &&
		!features
	) {
		return message;
	}

	const normalized: JsonRecord = { ...message };

	assignIfDefined(normalized, "variant", meta?.variant);
	assignIfDefined(normalized, "hw", meta?.hw);
	assignIfDefined(normalized, "drv", meta?.drv);
	assignIfDefined(normalized, "up", meta?.up);

	normalizeConnectivitySection(normalized, connectivity);
	normalizeStateSection(normalized, state);

	Object.entries(driverAssist ?? {}).forEach(([key, value]) =>
		assignIfDefined(normalized, key, value),
	);
	Object.entries(vehicle ?? {}).forEach(([key, value]) =>
		assignIfDefined(normalized, key, value),
	);

	normalizePlatformSection(normalized, platform);
	normalizeFirmwareSection(normalized, firmware);
	normalizeBatterySection(normalized, battery);
	normalizeSafetySection(normalized, safety);
	normalizeCanSection(normalized, can);
	normalizeFeaturesSection(normalized, features);

	return normalized;
}

function normalizeStatusStateMessage(message: JsonRecord): JsonRecord {
	const state = isJsonRecord(message.state) ? message.state : undefined;
	if (!state) {
		return message;
	}

	const normalized: JsonRecord = { ...message };
	normalizeStateSection(normalized, state);
	return normalized;
}

function normalizeStatusCanMessage(message: JsonRecord): JsonRecord {
	const normalized: JsonRecord = { ...message };
	const can: JsonRecord = {};

	if (message.clock !== undefined) {
		can.clock = message.clock;
	}
	if (message.health !== undefined) {
		can.health = message.health;
	}

	normalizeCanSection(normalized, can);
	return normalized;
}

function normalizeStatusFeaturesMessage(message: JsonRecord): JsonRecord {
	const features = isJsonRecord(message.features) ? message.features : undefined;
	if (!features) {
		return message;
	}

	const normalized: JsonRecord = { ...message };
	normalizeFeaturesSection(normalized, features);
	return normalized;
}

function normalizeStatusCompactMessage(message: JsonRecord): JsonRecord {
	const meta = isJsonRecord(message.meta) ? message.meta : undefined;
	const connectivity = isJsonRecord(message.connectivity) ? message.connectivity : undefined;
	const state = isJsonRecord(message.state) ? message.state : undefined;
	const features = isJsonRecord(message.features) ? message.features : undefined;
	const can = isJsonRecord(message.can) ? message.can : undefined;

	if (!meta && !connectivity && !state && !features && !can && !isJsonRecord(message.stream)) {
		return message;
	}

	const normalized: JsonRecord = { ...message };
	assignIfDefined(normalized, "variant", meta?.variant);
	assignIfDefined(normalized, "hw", meta?.hw);
	assignIfDefined(normalized, "up", meta?.up);
	normalizeConnectivitySection(normalized, connectivity);
	normalizeStateSection(normalized, state);
	normalizeFeaturesSection(normalized, features);
	normalizeCanSection(normalized, can);
	mergeObjectField(
		normalized,
		"stream",
		isJsonRecord(message.stream) ? message.stream : undefined,
	);

	return normalized;
}

function normalizeMessage(message: JsonRecord): JsonRecord {
	switch (message.t) {
		case "boot":
		case "status":
			return normalizeBootStatusMessage(message);
		case "status_state":
			return normalizeStatusStateMessage(message);
		case "status_can":
			return normalizeStatusCanMessage(message);
		case "status_features":
			return normalizeStatusFeaturesMessage(message);
		case "status_compact":
			return normalizeStatusCompactMessage(message);
		default:
			return message;
	}
}

/** Parse a single serial line, extracting JSON if present. */
export function parseSerialLine(line: string): ParsedEvent[] {
	const cleaned = line.replace(/\0/g, "").trim();
	if (!cleaned) return [];

	const start = cleaned.indexOf("{");
	const end = cleaned.lastIndexOf("}");

	if (start >= 0 && end > start) {
		try {
			const parsed = JSON.parse(cleaned.slice(start, end + 1));
			const msg = isJsonRecord(parsed) ? normalizeMessage(parsed) : parsed;
			return [{ type: "message", message: msg, raw: cleaned }];
		} catch (err) {
			return [
				{
					type: "parse-error",
					raw: cleaned,
					reason: err instanceof Error ? err.message : "Invalid JSON",
				},
			];
		}
	}

	return [{ type: "ignore", raw: cleaned }];
}

/** Parse a chunk of serial data, returning events and leftover buffer. */
export function parseSerialChunk(
	remainder: string,
	chunk: string,
): { remainder: string; events: ParsedEvent[] } {
	const combined = remainder + chunk;
	const lines = combined.split("\n");
	const newRemainder = lines.pop() || "";
	const events: ParsedEvent[] = [];

	for (const line of lines) {
		events.push(...parseSerialLine(line));
	}

	return { remainder: newRemainder, events };
}
