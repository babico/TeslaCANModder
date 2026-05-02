import type { BoardState, DecoderDataset } from "@teslacanmodder/protocol";

export type BusFilter = "all" | "0" | "1" | "2";

export interface ExportProvenance {
	schemaVersion: string;
	exportedAt: string;
	dataset: {
		id: string;
		label: string;
		source: DecoderDataset["dataset_source"];
	};
	platform: {
		variant: string;
		hardware: string;
		board: string;
		driver: string;
	};
	filters: {
		bus: BusFilter;
		text: string;
		frameWindowSize: number;
		frameSampleStep: number;
		decodeEnabled: boolean;
		feedPaused: boolean;
	};
	sessionSummary: {
		totalFrames: number;
		filteredFrames: number;
		renderedFrames: number;
		snapshots: number;
		commandHistory: number;
		notifications: number;
		timeframe: {
			first?: string;
			last?: string;
		};
	};
}

export interface BuildExportProvenanceInput {
	schemaVersion: string;
	dataset: {
		id: string;
		label: string;
		source: DecoderDataset["dataset_source"];
	};
	boardState: BoardState;
	bus: BusFilter;
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
}

export function buildExportProvenance(input: BuildExportProvenanceInput): ExportProvenance {
	return {
		schemaVersion: input.schemaVersion,
		exportedAt: new Date().toISOString(),
		dataset: input.dataset,
		platform: {
			variant: input.boardState.variant,
			hardware: input.boardState.hardware,
			board: input.boardState.board,
			driver: input.boardState.driver,
		},
		filters: {
			bus: input.bus,
			text: input.textFilter.trim(),
			frameWindowSize: input.frameWindowSize,
			frameSampleStep: input.frameSampleStep,
			decodeEnabled: input.decodeEnabled,
			feedPaused: input.feedPaused,
		},
		sessionSummary: {
			totalFrames: input.boardState.frames.length,
			filteredFrames: input.filteredFrames,
			renderedFrames: input.renderedFrames,
			snapshots: input.snapshots,
			commandHistory: input.commandHistory,
			notifications: input.notifications,
			timeframe: {
				first: input.boardState.frames[input.boardState.frames.length - 1]?.ts,
				last: input.boardState.frames[0]?.ts,
			},
		},
	};
}
