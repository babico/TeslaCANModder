import type { BoardState, CanFrame, DecoderDataset } from "@teslacanmodder/protocol";
import type { CommandName } from "../../../hardware/controller";
import type {
	MonitorTransportOption,
	MonitorTransportStatus,
} from "../../../hardware/transportPresentation";

export interface MonitorScreenProps {
	availableCommands: CommandName[];
	boardState: BoardState;
	selectedTransportOption: MonitorTransportOption;
	transportStatus: MonitorTransportStatus;

	frameCount: number;
	visibleFrames: CanFrame[];
	frameFilter: string;
	busFilter: string;
	frameFeedPaused: boolean;
	boardInfoFeedPaused: boolean;
	frameWindowSize: number;
	frameSampleStep: number;

	selectedDecoderDataset: { id: string; label: string; dataset: DecoderDataset };
	decoderDatasets: Array<{ id: string; label: string }>;
	liveDecodedFeed: Array<{
		frameKey: string;
		idHex: string;
		frameName: string;
		signalCount: number;
	}>;
	frameDecodedNameByKey: Record<string, string>;

	diagnosticsQuery: string;
	diagnosticsCategory: string;
	diagnosticsEvents: Array<{
		id: string;
		tsLabel: string;
		category: string;
		summary: string;
		detail: string;
		ok: boolean;
	}>;
	statusText: string;
	lastResult: string;
	history: Array<{ id: string; ts: number; command: string; ok: boolean; response: string }>;

	bleDeviceName: string;
	bleConfigBusy: boolean;

	onFrameFilterChange: (filter: string) => void;
	onBusFilterChange: (filter: string) => void;
	onFrameWindowSizeChange: (size: number) => void;
	onFrameSampleStepChange: (step: number) => void;
	onFrameFeedPausedChange: (paused: boolean) => void;
	onBoardInfoFeedPausedChange: (paused: boolean) => void;
	onDiagnosticsQueryChange: (query: string) => void;
	onDiagnosticsCategoryChange: (category: string) => void;
	onDatasetChange: (datasetId: string) => void;
	onRunCommand: (name: CommandName, args?: string) => Promise<void>;
	onRunRawCommand: (command: string) => Promise<string>;
	onFetchStatus: () => Promise<void>;
	onRefreshBleStatus: () => Promise<void>;
	onBleDeviceNameInputChange: (name: string) => void;
	onApplyBleDeviceName: () => Promise<void>;
	onExportJson: () => void;
	onExportCsv: () => void;
	onExportRawJson: () => void;
	onExportRawJsonl: () => void;
	onExportDecodedJson: () => void;
	onExportDecodedCsv: () => void;
	onExportDatasetDbc: () => void;
	onExportSessionPackage: () => void;
	onSaveSnapshot: () => void;
	onClearFeed: () => void;
}
