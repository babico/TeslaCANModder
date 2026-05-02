/**
 * Frame Ingestion Pipeline — Performance Guardrails (B-05)
 *
 * Provides a pure, testable ingestion layer for high frame-rate CAN data.
 *
 * Throughput targets:
 *   - Buffer cap:   MAX_FRAME_BUFFER (5000 frames) — hard ceiling, oldest evicted
 *   - Window:       1–200 frames rendered at a time (default 50)
 *   - Sample step:  1–20 (default 1 = no sampling; 2 = every other frame, etc.)
 *   - Pause:        feed can be frozen; paused frames buffered, not discarded
 *   - Stress target: 5000-frame ingest in < 50 ms wall time (pure array ops)
 */

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

export interface CanFrame {
	id: string;
	/** CAN bus identifier string, e.g. "vehicle" | "body" | "chassis" */
	bus: string;
	/** Unix-ms timestamp of the frame */
	ts: number;
	/** Raw hex data string */
	data: string;
	/** Optional decoded name */
	name?: string;
}

export interface FrameIngestionConfig {
	/**
	 * Hard buffer cap.
	 * Default: MAX_FRAME_BUFFER (5000).
	 * When exceeded, oldest frames are evicted.
	 */
	maxBuffer?: number;
	/**
	 * Number of frames to surface in the rendered window.
	 * Default: 50. Range: 1–200.
	 */
	windowSize?: number;
	/**
	 * Sampling stride — only every Nth frame is included in the window.
	 * Default: 1 (no sampling). Range: 1–20.
	 */
	sampleStep?: number;
	/**
	 * When true, incoming frames are buffered but not visible.
	 * Default: false.
	 */
	paused?: boolean;
	/**
	 * Bus filter — only frames matching this bus are surfaced.
	 * Default: "all" (no filter).
	 */
	busFilter?: string;
	/**
	 * Free-text filter — surfaced frames must contain this string in
	 * their id, name, or data fields (case-insensitive).
	 * Default: "" (no filter).
	 */
	textFilter?: string;
}

export interface FrameIngestionState {
	/** Full chronological buffer (newest first). */
	buffer: CanFrame[];
	/** Configuration active for this state. */
	config: Required<FrameIngestionConfig>;
	/** Monotonically increasing total frames ingested (never resets on eviction). */
	totalIngested: number;
	/** Total frames evicted due to buffer overflow. */
	totalEvicted: number;
}

// ---------------------------------------------------------------------------
// Constants / defaults
// ---------------------------------------------------------------------------

export const MAX_FRAME_BUFFER = 5000;
export const DEFAULT_WINDOW_SIZE = 50;
export const DEFAULT_SAMPLE_STEP = 1;

export const DEFAULT_INGESTION_CONFIG: Required<FrameIngestionConfig> = {
	maxBuffer: MAX_FRAME_BUFFER,
	windowSize: DEFAULT_WINDOW_SIZE,
	sampleStep: DEFAULT_SAMPLE_STEP,
	paused: false,
	busFilter: "all",
	textFilter: "",
};

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

export type FrameIngestionAction =
	| { type: "INGEST_FRAMES"; payload: { frames: CanFrame[] } }
	| { type: "SET_CONFIG"; payload: Partial<FrameIngestionConfig> }
	| { type: "CLEAR_BUFFER" }
	| { type: "PAUSE" }
	| { type: "RESUME" };

// ---------------------------------------------------------------------------
// Reducer
// ---------------------------------------------------------------------------

export function initialFrameIngestionState(config: FrameIngestionConfig = {}): FrameIngestionState {
	return {
		buffer: [],
		config: { ...DEFAULT_INGESTION_CONFIG, ...config },
		totalIngested: 0,
		totalEvicted: 0,
	};
}

export function frameIngestionReducer(
	state: FrameIngestionState,
	action: FrameIngestionAction,
): FrameIngestionState {
	switch (action.type) {
		case "INGEST_FRAMES": {
			const incoming = action.payload.frames;
			if (incoming.length === 0) return state;

			const cap = state.config.maxBuffer;
			// Prepend new frames (newest first), then trim to cap
			const merged = [...incoming, ...state.buffer];
			const evicted = Math.max(0, merged.length - cap);
			const trimmed = evicted > 0 ? merged.slice(0, cap) : merged;

			return {
				...state,
				buffer: trimmed,
				totalIngested: state.totalIngested + incoming.length,
				totalEvicted: state.totalEvicted + evicted,
			};
		}

		case "SET_CONFIG": {
			const next = { ...state.config, ...action.payload };
			// Clamp windowSize and sampleStep to valid ranges
			next.windowSize = Math.max(1, Math.min(200, next.windowSize));
			next.sampleStep = Math.max(1, Math.min(20, next.sampleStep));
			return { ...state, config: next };
		}

		case "CLEAR_BUFFER":
			return { ...state, buffer: [] };

		case "PAUSE":
			return { ...state, config: { ...state.config, paused: true } };

		case "RESUME":
			return { ...state, config: { ...state.config, paused: false } };

		default:
			return state;
	}
}

// ---------------------------------------------------------------------------
// Selectors
// ---------------------------------------------------------------------------

/**
 * Returns the visible frame window — filtered by bus + text, then
 * windowed and sampled — but only when the feed is not paused.
 *
 * When paused, returns an empty array (callers should keep their own
 * snapshot of the last visible window for rendering).
 */
export function selectVisibleFrames(state: FrameIngestionState): CanFrame[] {
	if (state.config.paused) return [];

	const { busFilter, textFilter, windowSize, sampleStep } = state.config;
	const tf = textFilter.toLowerCase();

	let frames = state.buffer;

	// Bus filter
	if (busFilter !== "all") {
		frames = frames.filter((f) => f.bus === busFilter);
	}

	// Text filter
	if (tf) {
		frames = frames.filter(
			(f) =>
				f.id.toLowerCase().includes(tf) ||
				(f.name ?? "").toLowerCase().includes(tf) ||
				f.data.toLowerCase().includes(tf),
		);
	}

	// Window
	const windowed = frames.slice(0, windowSize);

	// Sampling
	if (sampleStep <= 1) return windowed;
	return windowed.filter((_, idx) => idx % sampleStep === 0);
}

/**
 * Returns stats suitable for displaying in a performance diagnostic surface.
 */
export interface FrameIngestionStats {
	bufferSize: number;
	bufferCapacity: number;
	bufferUtilization: number; // 0–1
	totalIngested: number;
	totalEvicted: number;
	isPaused: boolean;
	windowSize: number;
	sampleStep: number;
}

export function selectIngestionStats(state: FrameIngestionState): FrameIngestionStats {
	return {
		bufferSize: state.buffer.length,
		bufferCapacity: state.config.maxBuffer,
		bufferUtilization: state.buffer.length / state.config.maxBuffer,
		totalIngested: state.totalIngested,
		totalEvicted: state.totalEvicted,
		isPaused: state.config.paused,
		windowSize: state.config.windowSize,
		sampleStep: state.config.sampleStep,
	};
}
