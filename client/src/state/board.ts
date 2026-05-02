import {
	BUS_NAMES,
	initialBoardState,
	reduceBoardMessage,
	type BoardMessage,
	type BoardState,
	type CanFrame,
	type ConsoleMessage,
} from "@teslacanmodder/protocol";

function isRecord(value: unknown): value is Record<string, unknown> {
	return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}

function normalizeFrame(value: unknown): CanFrame | null {
	if (!isRecord(value)) {
		return null;
	}

	const id = typeof value.id === "number" ? value.id : Number(value.id);
	if (!Number.isFinite(id)) {
		return null;
	}

	const bus = typeof value.bus === "number" ? value.bus : Number(value.bus ?? 0);
	const normalizedBus = Number.isFinite(bus) ? bus : 0;
	const seq = typeof value.seq === "number" ? value.seq : Number(value.seq);
	const data =
		typeof value.data === "string" ? value.data : typeof value.d === "string" ? value.d : "";
	const key =
		typeof value.key === "string"
			? value.key
			: `${Number.isFinite(seq) ? seq : Date.now()}-${id}`;

	return {
		key,
		id,
		dir: typeof value.dir === "string" ? value.dir : "rx",
		bus: normalizedBus,
		busName:
			typeof value.busName === "string"
				? value.busName
				: (BUS_NAMES[normalizedBus] ?? `Bus${normalizedBus}`),
		seq: Number.isFinite(seq) ? seq : undefined,
		dlc: typeof value.dlc === "number" ? value.dlc : Number(value.dlc ?? 0),
		data,
		ts: typeof value.ts === "string" ? value.ts : new Date().toLocaleTimeString(),
	};
}

function normalizeMessage(value: unknown): ConsoleMessage | null {
	if (!isRecord(value)) {
		return null;
	}

	const id = typeof value.id === "number" ? value.id : Number(value.id);
	if (!Number.isFinite(id)) {
		return null;
	}

	return {
		id,
		type: value.type === "error" ? "error" : "info",
		text: typeof value.text === "string" ? value.text : "",
		ts: typeof value.ts === "string" ? value.ts : new Date().toLocaleTimeString(),
	};
}

function normalizeCanHealth(value: unknown): BoardState["canHealth"] {
	if (!isRecord(value)) {
		return {};
	}

	return Object.fromEntries(
		Object.entries(value).map(([key, rawEntry]) => {
			const entry = isRecord(rawEntry) ? rawEntry : {};
			return [
				key,
				{
					on: Boolean(entry.on),
					det: Boolean(entry.det),
				},
			];
		}),
	);
}

function normalizeFeatures(value: unknown): BoardState["features"] {
	const base = initialBoardState.features;
	if (!isRecord(value)) {
		return base;
	}

	return {
		fsd: value.fsd !== undefined ? Boolean(value.fsd) : base.fsd,
		fsdForce: value.fsdForce !== undefined ? Boolean(value.fsdForce) : base.fsdForce,
		profile: value.profile !== undefined ? Boolean(value.profile) : base.profile,
		nag: value.nag !== undefined ? Boolean(value.nag) : base.nag,
		offset: value.offset !== undefined ? Boolean(value.offset) : base.offset,
		isaSpeedChime:
			value.isaSpeedChime !== undefined ? Boolean(value.isaSpeedChime) : base.isaSpeedChime,
		summon: value.summon !== undefined ? Boolean(value.summon) : base.summon,
	};
}

export function coerceBoardStateSnapshot(raw: unknown): BoardState | null {
	if (!isRecord(raw) || typeof raw.t === "string") {
		return null;
	}

	const frames = Array.isArray(raw.frames)
		? raw.frames.map(normalizeFrame).filter((frame): frame is CanFrame => frame !== null)
		: initialBoardState.frames;
	const messages = Array.isArray(raw.messages)
		? raw.messages
				.map(normalizeMessage)
				.filter((message): message is ConsoleMessage => message !== null)
		: initialBoardState.messages;

	return {
		...initialBoardState,
		...(raw as Partial<BoardState>),
		frames,
		messages,
		frameCount:
			typeof raw.frameCount === "number"
				? raw.frameCount
				: Array.isArray(frames)
					? frames.length
					: 0,
		features: normalizeFeatures(raw.features),
		canHealth: normalizeCanHealth(raw.canHealth),
	};
}

export function reduceBoardPayload(
	previous: BoardState,
	raw: unknown,
	nextId: () => number,
): BoardState | null {
	if (!isRecord(raw)) {
		return null;
	}

	if (typeof raw.t === "string") {
		return reduceBoardMessage(previous, raw as unknown as BoardMessage, nextId);
	}

	return coerceBoardStateSnapshot(raw);
}
