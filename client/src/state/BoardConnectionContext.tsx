/**
 * BoardConnectionContext  (compat layer — Phase B refactor)
 *
 * Backward-compatible wrapper that composes TransportContext, BoardStateContext,
 * and CommandContext. Existing imports of useBoardConnection and
 * BoardConnectionProvider continue to work.
 */

import { useMemo, type ReactNode } from "react";

import { type BoardState } from "@teslacanmodder/protocol";

import { DEFAULT_CONNECTION, HardwareController, type CommandName } from "../hardware/controller";
import {
	TransportProvider,
	useTransportState,
	useTransportActions,
	type ConnectionPreset,
} from "./TransportContext";
import {
	BoardStateProvider,
	useBoardInstanceState,
	useBoardInstanceActions,
} from "./BoardStateContext";
import {
	CommandProvider,
	useCommandState,
	useCommandActions,
	type CommandHistoryEntry,
} from "./CommandContext";
import type { CommandExecutionResult } from "./monitorCommandExecution";
import {
	getMonitorTransportExecutionPolicy,
	type MonitorTransportType,
} from "../hardware/transportPresentation";
import { buildMonitorTransportGateSnapshot } from "../hardware/monitorPollingPolicy";

// Re-export for backward compat
export { CONNECTION_PRESETS } from "./TransportContext";
export type { ConnectionPreset } from "./TransportContext";

export type { CommandHistoryEntry } from "./CommandContext";

export interface BoardConnectionState {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
	selectedTransportType: MonitorTransportType;
	boardState: BoardState;
	commandLifecycle: ReturnType<typeof useCommandState>["commandLifecycle"];
	statusText: string;
	lastResult: string;
	history: CommandHistoryEntry[];
	connectionBusy: boolean;
	isSelectedTransportReady: boolean;
	canFetchStatus: boolean;
}

export interface BoardConnectionActions {
	setBaseUrl: (url: string) => void;
	setCommandPath: (path: string) => void;
	setStatusPath: (path: string) => void;
	setSelectedTransportType: (type: MonitorTransportType) => void;
	applyConnection: () => Promise<void>;
	applyPreset: (preset: ConnectionPreset) => void;
	runCommand: (name: CommandName, rawArgs?: string) => Promise<CommandExecutionResult | null>;
	fetchStatus: () => Promise<void>;
	disconnectTransport: () => Promise<void>;
	pauseFrameFeed: (paused: boolean) => void;
	clearFrames: () => void;
	controller: HardwareController;
}

export type BoardConnectionContextValue = BoardConnectionState & BoardConnectionActions;

// ── Provider ───────────────────────────────────────────────────────────────────

export function BoardConnectionProvider({ children }: { children: ReactNode }) {
	const controller = useMemo(() => new HardwareController(DEFAULT_CONNECTION), []);

	return (
		<TransportProvider controller={controller}>
			<BoardStateProvider controller={controller}>
				<CommandProviderComposer controller={controller}>
					{children}
				</CommandProviderComposer>
			</BoardStateProvider>
		</TransportProvider>
	);
}

/** Inner component that bridges to CommandProvider with boardState from BoardStateContext */
function CommandProviderComposer({
	children,
	controller,
}: {
	children: ReactNode;
	controller: HardwareController;
}) {
	const transportState = useTransportState();
	const boardStateCtx = useBoardInstanceState();
	const boardActions = useBoardInstanceActions();

	return (
		<CommandProvider
			controller={controller}
			boardState={boardStateCtx.boardState}
			selectedTransportType={transportState.selectedTransportType}
			applyBoardPayload={boardActions.applyBoardPayload}
			setLastResult={boardActions.setLastResult}
		>
			{children}
		</CommandProvider>
	);
}

// ── Compat hook ────────────────────────────────────────────────────────────────

export function useBoardConnection(): BoardConnectionContextValue {
	const transportState = useTransportState();
	const transportActions = useTransportActions();
	const boardState = useBoardInstanceState();
	const boardActions = useBoardInstanceActions();
	const commandState = useCommandState();
	const commandActions = useCommandActions();

	const executionPolicy = getMonitorTransportExecutionPolicy(
		transportState.selectedTransportType,
		transportActions.controller.activeTransportType,
	);
	const gateSnapshot = buildMonitorTransportGateSnapshot(
		executionPolicy.ready,
		executionPolicy.blockReason,
	);

	return {
		// Connection config
		baseUrl: transportState.baseUrl,
		commandPath: transportState.commandPath,
		statusPath: transportState.statusPath,
		selectedTransportType: transportState.selectedTransportType,

		// Board data
		boardState: boardState.boardState,

		// Command bus lifecycle
		commandLifecycle: commandState.commandLifecycle,

		// UI state
		statusText: boardState.statusText,
		lastResult: boardState.lastResult,
		history: commandState.history,
		connectionBusy: transportState.connectionBusy,

		// Derived
		isSelectedTransportReady: transportState.isSelectedTransportReady,
		canFetchStatus: gateSnapshot.canFetchStatus,

		// Actions
		setBaseUrl: transportActions.setBaseUrl,
		setCommandPath: transportActions.setCommandPath,
		setStatusPath: transportActions.setStatusPath,
		setSelectedTransportType: transportActions.setSelectedTransportType,
		applyConnection: transportActions.applyConnection,
		applyPreset: transportActions.applyPreset,
		runCommand: commandActions.runCommand,
		fetchStatus: () => boardActions.fetchStatus(gateSnapshot.canFetchStatus),
		disconnectTransport: transportActions.disconnectTransport,
		pauseFrameFeed: boardActions.pauseFrameFeed,
		clearFrames: boardActions.clearFrames,
		controller: transportActions.controller,
	};
}
