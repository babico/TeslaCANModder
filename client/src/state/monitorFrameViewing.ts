import type { CanFrame } from "@teslacanmodder/protocol";

export type { CanFrame };
export type BusFilterType = "all" | "0" | "1" | "2";

export interface FilterFramesInput {
	frames: CanFrame[];
	busFilter: BusFilterType;
	textFilter: string;
}

export interface SelectVisibleFramesInput {
	frames: CanFrame[];
	windowSize: number;
	sampleStep: number;
}

export interface ApplyFrameViewingPipelineInput {
	frames: CanFrame[];
	busFilter: BusFilterType;
	textFilter: string;
	windowSize: number;
	sampleStep: number;
}

/**
 * Filters frames by bus and text criteria.
 *
 * Bus filter: "all" matches all buses, else matches frame.bus == parseInt(busFilter)
 * Text filter: searches frame.id (hex) and frame.data (case-insensitive)
 */
export function filterFrames(input: FilterFramesInput): CanFrame[] {
	const matchesBus = (frame: CanFrame): boolean => {
		if (input.busFilter === "all") return true;
		return String(frame.bus) === input.busFilter;
	};

	const trimmed = input.textFilter.trim();
	const matchesText = (frame: CanFrame): boolean => {
		if (!trimmed) return true;
		const normalized = trimmed.toLowerCase();
		const hexId = frame.id.toString(16).toLowerCase();
		return hexId.includes(normalized) || frame.data.toLowerCase().includes(normalized);
	};

	return input.frames.filter((frame) => matchesBus(frame) && matchesText(frame));
}

/**
 * Applies windowing and sampling to frames.
 *
 * Windowing: takes first `windowSize` frames.
 * Sampling: if sampleStep > 1, includes every Nth frame (e.g., 2 = every other).
 * Order: window first, then sample (not vice versa).
 */
export function selectVisibleFrames(input: SelectVisibleFramesInput): CanFrame[] {
	const windowed = input.frames.slice(0, input.windowSize);
	if (input.sampleStep <= 1) {
		return windowed;
	}
	return windowed.filter((_, idx) => idx % input.sampleStep === 0);
}

/**
 * Combined pipeline: filter, window, sample in one pass.
 *
 * Returns the final visible frame set ready for rendering.
 */
export function applyFrameViewingPipeline(input: ApplyFrameViewingPipelineInput): CanFrame[] {
	const filtered = filterFrames({
		frames: input.frames,
		busFilter: input.busFilter,
		textFilter: input.textFilter,
	});

	return selectVisibleFrames({
		frames: filtered,
		windowSize: input.windowSize,
		sampleStep: input.sampleStep,
	});
}
