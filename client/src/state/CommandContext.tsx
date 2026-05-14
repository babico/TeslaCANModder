/**
 * CommandContext
 *
 * Command execution, lifecycle tracking, and history.
 * Extracted from BoardConnectionContext (Phase B refactor).
 */

import {
	createContext,
	useCallback,
	useContext,
	useReducer,
	useState,
	type ReactNode,
} from "react";

import { type BoardState } from "@teslacanmodder/protocol";
import {
	commandBusReducer,
	generateCommandId,
	initialCommandBusState,
	type CommandLifecycleEntry,
} from "./commandBus";
import {
	buildCommandExecutionResult,
	resolveCommandExecutionReadiness,
	type CommandExecutionResult,
} from "./monitorCommandExecution";
import { getCommandGate } from "./commandGating";
import {
	getMonitorTransportExecutionPolicy,
	type MonitorTransportType,
} from "../hardware/transportPresentation";
import { buildMonitorTransportGateSnapshot } from "../hardware/monitorPollingPolicy";
import { HardwareController, type CommandName } from "../hardware/controller";

// ── Types ─────────────────────────────────────────────────────────────────────

export type CommandHistoryEntry = {
	id: string;
	ts: number;
	command: string;
	ok: boolean;
	response: string;
};

export interface CommandState {
	commandLifecycle: CommandLifecycleEntry[];
	history: CommandHistoryEntry[];
}

export interface CommandActions {
	runCommand: (name: CommandName, rawArgs?: string) => Promise<CommandExecutionResult | null>;
}

// ── Hooks ──────────────────────────────────────────────────────────────────────

export function useCommandState(): CommandState {
	const ctx = useContext(_CommandContext);
	if (!ctx) {
		throw new Error("useCommandState must be used inside <CommandProvider>");
	}
	return ctx.state;
}

export function useCommandActions(): CommandActions {
	const ctx = useContext(_CommandContext);
	if (!ctx) {
		throw new Error("useCommandActions must be used inside <CommandProvider>");
	}
	return ctx.actions;
}

// ── Context internals ──────────────────────────────────────────────────────────

interface CommandContextValue {
	state: CommandState;
	actions: CommandActions;
}

const _CommandContext = createContext<CommandContextValue | null>(null);

// ── Provider ───────────────────────────────────────────────────────────────────

export function CommandProvider({
	children,
	controller,
	boardState,
	selectedTransportType,
	applyBoardPayload,
	setLastResult,
}: {
	children: ReactNode;
	controller: HardwareController;
	boardState: BoardState;
	selectedTransportType: MonitorTransportType;
	applyBoardPayload: (raw: unknown) => void;
	setLastResult: (text: string) => void;
}) {
	const [busState] = useReducer(commandBusReducer, undefined, initialCommandBusState);
	const commandLifecycle = busState.entries;
	const [history, setHistory] = useState<CommandHistoryEntry[]>([]);

	// Derived transport state
	const transportExecutionPolicy = getMonitorTransportExecutionPolicy(
		selectedTransportType,
		controller.activeTransportType,
	);
	const transportGateSnapshot = buildMonitorTransportGateSnapshot(
		transportExecutionPolicy.ready,
		transportExecutionPolicy.blockReason,
	);
	const isSelectedTransportReady = transportGateSnapshot.canExecuteCommands;

	// ── History append ────────────────────────────────────────────────────────

	const pushHistory = useCallback((entry: Omit<CommandHistoryEntry, "id" | "ts">) => {
		setHistory((current) =>
			[
				{
					id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
					ts: Date.now(),
					...entry,
				},
				...current,
			].slice(0, 40),
		);
	}, []);

	// ── Run command ────────────────────────────────────────────────────────────

	const runCommand = useCallback(
		async (name: CommandName, rawArgs = ""): Promise<CommandExecutionResult | null> => {
			const commandGate = getCommandGate(name, boardState);
			const readiness = resolveCommandExecutionReadiness({
				transportCanExecute: isSelectedTransportReady,
				transportBlockReason: transportGateSnapshot.commandBlockReason ?? undefined,
				commandAvailable: commandGate.available,
				commandBlockReason: commandGate.reason ?? undefined,
			});

			if (!readiness.canExecute) {
				setLastResult(readiness.blockReason ?? "Command blocked");
				return null;
			}

			const controllerResult = await controller.runCommand(name, rawArgs);
			const result = buildCommandExecutionResult({
				canExecute: true,
				commandName: name,
				rawArgs,
				startedAt: Date.now(),
				lifecycleId: generateCommandId(),
				controllerResponse: {
					ok: controllerResult.ok,
					responseData: controllerResult.responseData,
					error: controllerResult.error,
				},
			});

			setLastResult(result.displayMessage);
			if (result.shouldApplyBoardPayload && controllerResult.responseData) {
				applyBoardPayload(controllerResult.responseData);
			}
			if (result.historyEntry) {
				pushHistory(result.historyEntry);
			}

			return result;
		},
		[
			boardState,
			isSelectedTransportReady,
			transportGateSnapshot,
			controller,
			applyBoardPayload,
			pushHistory,
			setLastResult,
		],
	);

	const value: CommandContextValue = {
		state: {
			commandLifecycle,
			history,
		},
		actions: {
			runCommand,
		},
	};

	return <_CommandContext.Provider value={value}>{children}</_CommandContext.Provider>;
}
