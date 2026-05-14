import { lazy, Suspense, useEffect, useMemo, useReducer, useRef, useState } from "react";
import { StyleSheet } from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import { describeDecodedFrame, initialBoardState, type BoardState } from "@teslacanmodder/protocol";

import { useBleConfig } from "./hooks/useBleConfig";
import { useDecoderDatasets } from "./hooks/useDecoderDatasets";
import { useDiagnostics } from "./hooks/useDiagnostics";
import { type DiagnosticsCategory } from "./state/monitorDiagnostics";

import { ALL_COMMANDS, type CommandName } from "./hardware/controller";
import {
	buildMonitorTransportStatus,
	getMonitorTransportExecutionPolicy,
	getMonitorTransportOption,
	type MonitorTransportType,
} from "./hardware/transportPresentation";
import {
	buildMonitorTransportGateSnapshot,
	getAutoPollPolicy,
} from "./hardware/monitorPollingPolicy";
import { connectRuntimeBlePeripheral, requestRuntimeSerialPort } from "./hardware/runtimeAdapters";
import { reduceBoardPayload } from "./state/board";
import { commandBusReducer, generateCommandId, initialCommandBusState } from "./state/commandBus";
import { buildExportProvenance } from "./state/monitorExport";
import { saveLiveCanFramesToIndexedDb } from "./state/monitorLiveCanPersistence";
import { applyFrameViewingPipeline, type BusFilterType } from "./state/monitorFrameViewing";
import {
	buildCommandExecutionResult,
	resolveCommandExecutionReadiness,
} from "./state/monitorCommandExecution";
import { getCommandGate } from "./state/commandGating";
import { useBoardConnection } from "./state/BoardConnectionContext";
import { DashboardScreen } from "./screens/DashboardScreen";
import { ControlsScreen } from "./screens/ControlsScreen";
import { ConsoleScreen } from "./screens/ConsoleScreen";
const DocsScreen = lazy(() =>
	import("./screens/DocsScreen").then((m) => ({ default: m.DocsScreen })),
);
const FlasherScreen = lazy(() =>
	import("./screens/FlasherScreen").then((m) => ({ default: m.FlasherScreen })),
);
import { ConnectionHeader } from "./components/ConnectionHeader";
import { MenuHeader } from "./components/MenuHeader";
import { colors } from "./design/tokens";
import { type AppTabRoute, useAppRouteState } from "./state/appRoute";

type HistoryEntry = {
	id: string;
	ts: number;
	command: string;
	ok: boolean;
	response: string;
};

type FrameSnapshot = {
	id: string;
	ts: number;
	frameCount: number;
	busFilter: BusFilter;
	frameFilter: string;
};

type ExportRow = {
	ts: string;
	bus: number;
	busName: string;
	idHex: string;
	dlc: number;
	dir: string;
	data: string;
};

const EXPORT_SCHEMA_VERSION = "monitor-export.v1";

const POLL_SECONDS = [0, 2, 5, 10];

type BusFilter = "all" | "0" | "1" | "2";

type Tab = AppTabRoute;

const TABS: Array<{ id: Tab; label: string }> = [
	{ id: "dashboard", label: "Dashboard" },
	{ id: "controls", label: "Controls" },
	{ id: "console", label: "Console" },
	{ id: "flasher", label: "Flasher" },
	{ id: "docs", label: "Docs" },
];

function isSerialFamilyTransport(type: string): boolean {
	return type === "serial" || type === "bluetooth-serial";
}

function matchesSelectedTransport(selected: MonitorTransportType, active: string): boolean {
	if (selected === "serial") {
		return isSerialFamilyTransport(active);
	}
	return selected === active;
}

function computeExportChecksum(rows: ExportRow[]): number {
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

function verifyExportIntegrity(
	rows: ExportRow[],
	expectedChecksum: number,
	expectedCount: number,
): boolean {
	if (rows.length !== expectedCount) {
		return false;
	}
	return computeExportChecksum(rows) === expectedChecksum;
}

export default function AppExperience() {
	const connection = useBoardConnection();
	const controller = connection.controller;
	const baseUrl = connection.baseUrl;
	const commandPath = connection.commandPath;
	const statusPath = connection.statusPath;
	const selectedTransportType = connection.selectedTransportType;
	const [_selectedCommand, _setSelectedCommand] = useState<CommandName>(ALL_COMMANDS[0].name);
	const [autoPoll, setAutoPoll] = useState(false);
	const [pollIndex, _setPollIndex] = useState(2);
	const [lastResult, setLastResult] = useState("Ready");
	const [statusText, setStatusText] = useState("No status fetched");
	const [history, setHistory] = useState<HistoryEntry[]>([]);
	const [busState, busDispatch] = useReducer(
		commandBusReducer,
		undefined,
		initialCommandBusState,
	);
	const commandLifecycle = busState.entries;
	const [boardState, setBoardState] = useState<BoardState>(initialBoardState);
	const [frameFilter, setFrameFilter] = useState("");
	const [busFilter, setBusFilter] = useState<BusFilter>("all");
	const [frameFeedPaused, setFrameFeedPaused] = useState(false);
	const [boardInfoFeedPaused, setBoardInfoFeedPaused] = useState(false);
	const [decodeFrames, _setDecodeFrames] = useState(true);
	const [frameSnapshots, setFrameSnapshots] = useState<FrameSnapshot[]>([]);
	const [frameWindowSize, setFrameWindowSize] = useState<number>(50);
	const [frameSampleStep, setFrameSampleStep] = useState<number>(1);
	const nextMessageId = useRef(1);
	const { route, navigateToTab, navigateToDoc } = useAppRouteState();
	const activeTab = route.tab;

	const selectedTransportOption = getMonitorTransportOption(selectedTransportType);
	const transportStatus = buildMonitorTransportStatus(
		selectedTransportType,
		controller.activeTransportType,
		baseUrl,
	);
	const transportExecutionPolicy = getMonitorTransportExecutionPolicy(
		selectedTransportType,
		controller.activeTransportType,
	);
	const transportGateSnapshot = buildMonitorTransportGateSnapshot(
		transportExecutionPolicy.ready,
		transportExecutionPolicy.blockReason,
	);
	const canFetchStatus = transportGateSnapshot.canFetchStatus;

	const { allDecoderDatasets, selectedDecoderDataset, decoderIndex, setDecoderDatasetId } =
		useDecoderDatasets();

	const visibleFrames = useMemo(
		() =>
			applyFrameViewingPipeline({
				frames: boardState.frames,
				busFilter: busFilter as BusFilterType,
				textFilter: frameFilter,
				windowSize: frameWindowSize,
				sampleStep: frameSampleStep,
			}),
		[boardState.frames, busFilter, frameFilter, frameWindowSize, frameSampleStep],
	);

	const sampledVisibleFrames = useMemo(() => visibleFrames, [visibleFrames]);

	const liveDecodedFeed = useMemo(
		() =>
			sampledVisibleFrames
				.map((frame) => {
					const decoded = describeDecodedFrame(decoderIndex, frame.id);
					if (decoded.length === 0) {
						return null;
					}
					return {
						frameKey: frame.key,
						idHex: `0x${frame.id.toString(16).toUpperCase()}`,
						frameName: decoded[0].frameName,
						signalCount: decoded.reduce(
							(total, entry) => total + entry.signals.length,
							0,
						),
					};
				})
				.filter(
					(
						entry,
					): entry is {
						frameKey: string;
						idHex: string;
						frameName: string;
						signalCount: number;
					} => Boolean(entry),
				),
		[decoderIndex, sampledVisibleFrames],
	);

	const frameDecodedNameByKey = useMemo(
		() =>
			Object.fromEntries(
				sampledVisibleFrames.map((frame) => {
					const decoded = describeDecodedFrame(decoderIndex, frame.id);
					return [frame.key, decoded.length > 0 ? decoded[0].frameName : ""];
				}),
			) as Record<string, string>,
		[decoderIndex, sampledVisibleFrames],
	);

	useEffect(() => {
		if (boardState.frames.length === 0) {
			return;
		}

		const handle = setTimeout(() => {
			void saveLiveCanFramesToIndexedDb(boardState.frames);
		}, 500);

		return () => {
			clearTimeout(handle);
		};
	}, [boardState.frames]);

	const {
		diagnosticsQuery,
		setDiagnosticsQuery,
		diagnosticsCategory,
		setDiagnosticsCategory,
		pushDiagnosticsArchive,
		visibleDiagnostics,
	} = useDiagnostics({
		commandLifecycle,
		history,
		boardMessages: boardState.messages,
		frameSnapshots,
		setHistory,
		setFrameSnapshots,
	});

	const { bleDeviceName, setBleDeviceName, bleConfigBusy, refreshBleStatus, applyBleDeviceName } =
		useBleConfig({
			baseUrl,
			onLog: pushDiagnosticsArchive,
			onSetLastResult: setLastResult,
		});

	const applyBoardPayload = (raw: unknown) => {
		setBoardState((current) => {
			const next = reduceBoardPayload(current, raw, () => nextMessageId.current++);
			if (!next) {
				return current;
			}
			if (frameFeedPaused) {
				// Keep non-frame status updates while freezing the frame feed.
				const pausedFrameState: BoardState = {
					...next,
					frames: current.frames,
					frameCount: current.frameCount,
				};
				if (boardInfoFeedPaused) {
					return {
						...pausedFrameState,
						messages: current.messages,
					};
				}
				return pausedFrameState;
			}

			if (boardInfoFeedPaused) {
				return {
					...next,
					messages: current.messages,
				};
			}
			return next;
		});
	};

	const appendBoardConsoleMessage = (text: string, type: "info" | "error" = "info") => {
		if (boardInfoFeedPaused) {
			return;
		}
		const trimmed = text.trim();
		if (!trimmed) {
			return;
		}
		setBoardState((current) => ({
			...current,
			messages: [
				{
					id: nextMessageId.current++,
					type,
					text: trimmed,
					ts: new Date().toLocaleTimeString(),
				},
				...current.messages,
			].slice(0, 200),
		}));
	};

	const ensureSharedTransportReady = async (): Promise<void> => {
		if (selectedTransportType === "http") {
			connection.controller.connectViaRestApi({
				baseUrl: connection.baseUrl,
				commandPath: connection.commandPath,
				statusPath: connection.statusPath,
			});
			return;
		}

		if (
			matchesSelectedTransport(
				selectedTransportType,
				connection.controller.activeTransportType,
			)
		) {
			return;
		}

		if (selectedTransportType === "ble") {
			const peripheral = await connectRuntimeBlePeripheral();
			connection.controller.connectViaBle(peripheral);
			return;
		}

		if (selectedTransportType === "bluetooth-serial") {
			const port = await requestRuntimeSerialPort();
			connection.controller.connectViaBluetoothComPort(port);
			return;
		}

		const port = await requestRuntimeSerialPort();
		connection.controller.connectViaComPort(port);
	};

	const pushHistory = (entry: Omit<HistoryEntry, "id" | "ts">) => {
		setHistory((current) =>
			[
				{
					id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
					ts: Date.now(),
					...entry,
				},
				...current,
			].slice(0, 20),
		);
	};

	const runCommand = async (name: CommandName, rawArgs = "") => {
		const lifecycleId = generateCommandId();
		const startedAt = Date.now();
		const commandGate = getCommandGate(name, boardState);

		// Align controller transport with the shared selection before gating/execution.
		try {
			await ensureSharedTransportReady();
		} catch (error) {
			const message =
				error instanceof Error ? error.message : "Failed to initialize selected transport.";
			pushDiagnosticsArchive("command", `${name} blocked`, message, false);
			setLastResult(message);
			return;
		}

		// Resolve execution readiness using transport and command gates
		const readiness = resolveCommandExecutionReadiness({
			transportCanExecute: true,
			transportBlockReason: undefined,
			commandAvailable: commandGate.available,
			commandBlockReason: commandGate.reason ?? undefined,
		});

		// If command cannot execute, display block reason
		if (!readiness.canExecute) {
			pushDiagnosticsArchive(
				"command",
				`${name} blocked`,
				readiness.blockReason ?? "Command blocked",
				false,
			);
			setLastResult(readiness.blockReason ?? "Command blocked");
			return;
		}

		// Run the command and build execution result
		const controllerResult = await controller.runCommand(name, rawArgs);
		const executionResult = buildCommandExecutionResult({
			canExecute: true,
			commandName: name,
			rawArgs,
			lifecycleId,
			startedAt,
			controllerResponse: controllerResult,
		});

		// Dispatch all lifecycle actions
		for (const action of executionResult.dispatchActions) {
			busDispatch(action);
		}

		// Apply board payload if needed
		if (
			executionResult.shouldApplyBoardPayload &&
			controllerResult.responseData !== undefined
		) {
			applyBoardPayload(controllerResult.responseData);
		}

		// Update UI with result message
		setLastResult(executionResult.displayMessage);

		// Add to history if provided
		if (executionResult.historyEntry) {
			pushHistory(executionResult.historyEntry);
		}

		pushDiagnosticsArchive(
			"command",
			`${name} executed`,
			executionResult.displayMessage,
			executionResult.historyEntry?.ok ?? true,
		);
	};

	const runRawConsoleCommand = async (rawCommand: string): Promise<string> => {
		const trimmed = rawCommand.trim();
		if (!trimmed) {
			const message = "Console command cannot be empty.";
			pushDiagnosticsArchive("command", "Raw command blocked", message, false);
			setLastResult(message);
			throw new Error(message);
		}

		appendBoardConsoleMessage(`> ${trimmed}`, "info");

		const selectedTransport = connection.selectedTransportType;

		try {
			await ensureSharedTransportReady();
		} catch (error) {
			const message =
				error instanceof Error ? error.message : "Failed to initialize selected transport.";
			pushDiagnosticsArchive("command", "Raw command blocked", message, false);
			setLastResult(message);
			throw new Error(message);
		}

		const result = await connection.controller.runRawCommand(trimmed);

		if (result.responseData !== undefined) {
			applyBoardPayload(result.responseData);
		}

		const routeLabel =
			selectedTransport === "http"
				? "WiFi REST API"
				: selectedTransport === "ble"
					? "BLE"
					: "COM CLI";
		const responseText = result.ok
			? (result.responseText ?? "ok")
			: (result.error ?? "unknown failure");

		setLastResult(
			result.ok
				? `Console sent via ${routeLabel}: ${result.command}`
				: `Console send failed via ${routeLabel}: ${responseText}`,
		);
		pushHistory({
			command: `console:raw:${trimmed}`,
			ok: result.ok,
			response: responseText,
		});
		pushDiagnosticsArchive(
			"command",
			`Raw console command via ${routeLabel}: ${trimmed}`,
			responseText,
			result.ok,
		);

		if (!result.ok) {
			appendBoardConsoleMessage(`< ${responseText}`, "error");
			throw new Error(responseText);
		}

		appendBoardConsoleMessage(`< ${responseText}`, "info");
		return responseText;
	};

	const handleBoardInfoFeedPausedChange = (paused: boolean) => {
		setBoardInfoFeedPaused(paused);

		void (async () => {
			try {
				await ensureSharedTransportReady();
				const command = paused ? "status:live:off" : "status:live:on";
				const result = await connection.controller.runRawCommand(command);
				if (!result.ok) {
					const reason = result.error ?? "failed";
					pushDiagnosticsArchive(
						"system",
						"Board info live toggle failed",
						reason,
						false,
					);
					return;
				}

				pushDiagnosticsArchive(
					"system",
					paused ? "Board info live paused" : "Board info live enabled",
					`cmd=${command}`,
				);
			} catch (error) {
				const message =
					error instanceof Error ? error.message : "Board info live toggle failed.";
				pushDiagnosticsArchive("system", "Board info live toggle failed", message, false);
			}
		})();
	};

	const readStatus = async () => {
		try {
			await ensureSharedTransportReady();
		} catch (error) {
			const reason =
				error instanceof Error ? error.message : "Failed to initialize selected transport.";
			setStatusText(reason);
			setLastResult(reason);
			pushDiagnosticsArchive("system", "Status fetch blocked", reason, false);
			return;
		}

		try {
			const status = await controller.readStatus();
			applyBoardPayload(status.raw);
			const text = JSON.stringify(status.raw, null, 2);
			setStatusText(text);
			pushDiagnosticsArchive(
				"system",
				"Status fetched",
				"Board status updated from transport.",
			);
		} catch (error) {
			const message = error instanceof Error ? error.message : "status failed";
			setStatusText(message);
			pushDiagnosticsArchive("system", "Status fetch failed", message, false);
		}
	};

	useEffect(() => {
		if (activeTab === "flasher") {
			return;
		}

		const autoPollPolicy = getAutoPollPolicy({
			autoPoll,
			canFetchStatus,
			pollSeconds: POLL_SECONDS[pollIndex],
			blockReason: transportExecutionPolicy.blockReason,
		});

		if (autoPollPolicy.action === "idle") {
			return;
		}

		if (autoPollPolicy.action === "disable") {
			setAutoPoll(false);
			const reason = autoPollPolicy.reason;
			setStatusText(reason);
			setLastResult(reason);
			return;
		}

		const handle = setInterval(() => {
			void readStatus();
		}, autoPollPolicy.everySeconds * 1000);

		return () => {
			clearInterval(handle);
		};
	}, [
		activeTab,
		autoPoll,
		pollIndex,
		baseUrl,
		commandPath,
		statusPath,
		canFetchStatus,
		transportExecutionPolicy.blockReason,
	]);

	const clearMonitorFeed = () => {
		setBoardState((current) => ({
			...current,
			frames: [],
			frameCount: 0,
			messages: [],
		}));
		pushDiagnosticsArchive(
			"snapshot",
			"Feed cleared",
			"Cleared live frame feed and notifications.",
		);
	};

	const saveFrameSnapshot = () => {
		setFrameSnapshots((current) =>
			[
				{
					id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
					ts: Date.now(),
					frameCount: visibleFrames.length,
					busFilter,
					frameFilter: frameFilter.trim(),
				},
				...current,
			].slice(0, 20),
		);
		pushDiagnosticsArchive(
			"snapshot",
			"Snapshot saved",
			`frames=${visibleFrames.length} bus=${busFilter} filter=${frameFilter.trim() || "none"}`,
		);
	};

	const buildExportRows = (): ExportRow[] => {
		return visibleFrames.map((frame) => ({
			ts: frame.ts,
			bus: frame.bus,
			busName: frame.busName,
			idHex: `0x${frame.id.toString(16).toUpperCase()}`,
			dlc: frame.dlc,
			dir: frame.dir,
			data: frame.data,
		}));
	};

	const exportVisibleFramesJson = () => {
		const rows = buildExportRows();
		const checksum = computeExportChecksum(rows);
		const rowCount = rows.length;
		const provenance = buildExportProvenance({
			schemaVersion: EXPORT_SCHEMA_VERSION,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const payload = JSON.stringify(
			{
				provenance,
				rowCount,
				checksum,
				rows,
			},
			null,
			2,
		);
		const valid = verifyExportIntegrity(rows, checksum, rowCount);
		setStatusText(payload);
		setLastResult(
			`Exported ${rowCount} visible frames as JSON (${EXPORT_SCHEMA_VERSION}, checksum=${checksum}, valid=${valid}).`,
		);
		pushHistory({
			command: "monitor:export:json",
			ok: valid,
			response: `rows=${rowCount} checksum=${checksum} schema=${EXPORT_SCHEMA_VERSION}`,
		});
	};

	const exportVisibleFramesCsv = () => {
		const rows = buildExportRows();
		const checksum = computeExportChecksum(rows);
		const rowCount = rows.length;
		const provenance = buildExportProvenance({
			schemaVersion: EXPORT_SCHEMA_VERSION,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const header = "ts,bus,busName,idHex,dlc,dir,data";
		const body = rows
			.map((frame) =>
				[frame.ts, frame.bus, frame.busName, frame.idHex, frame.dlc, frame.dir, frame.data]
					.map((part) => `"${String(part).replace(/"/g, '""')}"`)
					.join(","),
			)
			.join("\n");
		const meta = [
			`# schema=${provenance.schemaVersion}`,
			`# exportedAt=${provenance.exportedAt}`,
			`# dataset=${provenance.dataset.id}`,
			`# platform=${provenance.platform.variant}/${provenance.platform.hardware}/${provenance.platform.board}`,
			`# rows=${rowCount} checksum=${checksum} bus=${busFilter} filter="${frameFilter.trim()}"`,
		].join("\n");
		const payload = `${meta}\n${header}\n${body}`;
		const valid = verifyExportIntegrity(rows, checksum, rowCount);
		setStatusText(payload);
		setLastResult(
			`Exported ${rowCount} visible frames as CSV (${EXPORT_SCHEMA_VERSION}, checksum=${checksum}, valid=${valid}).`,
		);
		pushHistory({
			command: "monitor:export:csv",
			ok: valid,
			response: `rows=${rowCount} checksum=${checksum} schema=${EXPORT_SCHEMA_VERSION}`,
		});
	};

	const exportRawSessionJson = () => {
		const rows = boardState.frames.map((frame) => ({
			ts: frame.ts,
			idHex: `0x${frame.id.toString(16).toUpperCase()}`,
			bus: frame.bus,
			busName: frame.busName,
			dir: frame.dir,
			dlc: frame.dlc,
			data: frame.data,
		}));
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.raw`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const payload = JSON.stringify(
			{
				provenance,
				mode: "raw-json",
				rowCount: rows.length,
				rows,
			},
			null,
			2,
		);
		setStatusText(payload);
		setLastResult(`Exported raw session JSON (${rows.length} rows).`);
		pushHistory({
			command: "monitor:export:raw-json",
			ok: true,
			response: `rows=${rows.length}`,
		});
	};

	const exportRawSessionJsonl = () => {
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.raw-jsonl`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const lines = boardState.frames.map((frame) =>
			JSON.stringify({
				ts: frame.ts,
				idHex: `0x${frame.id.toString(16).toUpperCase()}`,
				bus: frame.bus,
				busName: frame.busName,
				dir: frame.dir,
				dlc: frame.dlc,
				data: frame.data,
			}),
		);
		const payload = [JSON.stringify({ __meta: provenance }), ...lines].join("\n");
		setStatusText(payload);
		setLastResult(`Exported raw session JSONL (${lines.length} rows).`);
		pushHistory({
			command: "monitor:export:raw-jsonl",
			ok: true,
			response: `rows=${lines.length}`,
		});
	};

	const exportDecodedSessionJson = () => {
		const rows = visibleFrames.map((frame) => ({
			ts: frame.ts,
			idHex: `0x${frame.id.toString(16).toUpperCase()}`,
			dataset: selectedDecoderDataset.id,
			decoded: describeDecodedFrame(decoderIndex, frame.id),
		}));
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.decoded`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const payload = JSON.stringify(
			{
				provenance,
				mode: "decoded-json",
				dataset: selectedDecoderDataset.id,
				rowCount: rows.length,
				rows,
			},
			null,
			2,
		);
		setStatusText(payload);
		setLastResult(
			`Exported decoded session JSON (${rows.length} rows, dataset=${selectedDecoderDataset.id}).`,
		);
		pushHistory({
			command: "monitor:export:decoded-json",
			ok: true,
			response: `rows=${rows.length} dataset=${selectedDecoderDataset.id}`,
		});
	};

	const exportDecodedSessionCsv = () => {
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.decoded-csv`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const header = "ts,idHex,dataset,frameName,signalName,enumValues";
		const lines: string[] = [];
		for (const frame of visibleFrames) {
			const decoded = describeDecodedFrame(decoderIndex, frame.id);
			if (decoded.length === 0) {
				lines.push(
					[
						frame.ts,
						`0x${frame.id.toString(16).toUpperCase()}`,
						selectedDecoderDataset.id,
						"",
						"",
						"",
					]
						.map((part) => `"${String(part).replace(/"/g, '""')}"`)
						.join(","),
				);
				continue;
			}
			for (const entry of decoded) {
				if (entry.signals.length === 0) {
					lines.push(
						[
							frame.ts,
							`0x${frame.id.toString(16).toUpperCase()}`,
							selectedDecoderDataset.id,
							entry.frameName,
							"",
							"",
						]
							.map((part) => `"${String(part).replace(/"/g, '""')}"`)
							.join(","),
					);
					continue;
				}
				for (const signal of entry.signals) {
					const enumValues = signal.values
						.map((value) => `${value.value}:${value.label}`)
						.join(" | ");
					lines.push(
						[
							frame.ts,
							`0x${frame.id.toString(16).toUpperCase()}`,
							selectedDecoderDataset.id,
							entry.frameName,
							signal.name,
							enumValues,
						]
							.map((part) => `"${String(part).replace(/"/g, '""')}"`)
							.join(","),
					);
				}
			}
		}
		const meta = [
			`# schema=${provenance.schemaVersion}`,
			`# exportedAt=${provenance.exportedAt}`,
			`# dataset=${provenance.dataset.id}`,
			`# platform=${provenance.platform.variant}/${provenance.platform.hardware}/${provenance.platform.board}`,
		].join("\n");
		const payload = `${meta}\n${header}\n${lines.join("\n")}`;
		setStatusText(payload);
		setLastResult(
			`Exported decoded session CSV (${lines.length} rows, dataset=${selectedDecoderDataset.id}).`,
		);
		pushHistory({
			command: "monitor:export:decoded-csv",
			ok: true,
			response: `rows=${lines.length} dataset=${selectedDecoderDataset.id}`,
		});
	};

	const exportDecoderDatasetDbc = () => {
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.dataset-dbc`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});
		const lines: string[] = [];
		lines.push(`VERSION "${selectedDecoderDataset.id}"`);
		lines.push("NS_ :");
		lines.push("BS_:");
		lines.push("BU_: MONITOR");
		lines.push(
			`CM_ "Schema ${provenance.schemaVersion} exportedAt ${provenance.exportedAt} dataset ${provenance.dataset.id}";`,
		);
		lines.push(
			'CM_ "Dataset-derived DBC preview. Signal bit layout is not inferred from live traffic; names/enums come from selected dataset metadata.";',
		);
		for (const frame of selectedDecoderDataset.dataset.frames) {
			lines.push(`BO_ ${frame.id} ${frame.frame_name}: 8 MONITOR`);
			if (frame.signals.length === 0) {
				lines.push(
					` CM_ BO_ ${frame.id} "No signal metadata available in selected dataset.";`,
				);
				continue;
			}
			frame.signals.forEach((signal, idx) => {
				const startBit = idx * 8;
				lines.push(
					` SG_ ${signal.signal_name} : ${startBit}|8@1+ (1,0) [0|255] "" MONITOR`,
				);
				if (signal.possible_values && signal.possible_values.length > 0) {
					const enumDefs = signal.possible_values
						.map((value) => `${value.value_dec} "${value.label.replace(/"/g, '\\"')}"`)
						.join(" ");
					lines.push(` VAL_ ${frame.id} ${signal.signal_name} ${enumDefs} ;`);
				}
			});
		}
		const payload = lines.join("\n");
		setStatusText(payload);
		setLastResult(
			`Exported dataset-derived DBC preview (${selectedDecoderDataset.dataset.frames.length} frames, dataset=${selectedDecoderDataset.id}).`,
		);
		pushHistory({
			command: "monitor:export:dataset-dbc",
			ok: true,
			response: `frames=${selectedDecoderDataset.dataset.frames.length} dataset=${selectedDecoderDataset.id}`,
		});
	};

	const exportSessionPackage = () => {
		const rawRows = boardState.frames.map((frame) => ({
			ts: frame.ts,
			idHex: `0x${frame.id.toString(16).toUpperCase()}`,
			bus: frame.bus,
			busName: frame.busName,
			dir: frame.dir,
			dlc: frame.dlc,
			data: frame.data,
		}));
		const decodedRows = visibleFrames.map((frame) => ({
			ts: frame.ts,
			idHex: `0x${frame.id.toString(16).toUpperCase()}`,
			decoded: describeDecodedFrame(decoderIndex, frame.id),
		}));
		const provenance = buildExportProvenance({
			schemaVersion: `${EXPORT_SCHEMA_VERSION}.session-package`,
			dataset: {
				id: selectedDecoderDataset.id,
				label: selectedDecoderDataset.label,
				source: selectedDecoderDataset.dataset.dataset_source,
			},
			boardState,
			bus: busFilter,
			textFilter: frameFilter,
			frameWindowSize,
			frameSampleStep,
			decodeEnabled: decodeFrames,
			feedPaused: frameFeedPaused,
			filteredFrames: visibleFrames.length,
			renderedFrames: sampledVisibleFrames.length,
			snapshots: frameSnapshots.length,
			commandHistory: history.length,
			notifications: boardState.messages.length,
		});

		const packagePayload = {
			provenance,
			raw: {
				format: "json",
				rowCount: rawRows.length,
				rows: rawRows,
			},
			decoded: {
				format: "json",
				rowCount: decodedRows.length,
				rows: decodedRows,
			},
			snapshots: frameSnapshots,
			diagnostics: visibleDiagnostics.slice(0, 40),
		};
		setStatusText(JSON.stringify(packagePayload, null, 2));
		setLastResult(
			`Exported session package (raw=${rawRows.length}, decoded=${decodedRows.length}, snapshots=${frameSnapshots.length}).`,
		);
		pushHistory({
			command: "monitor:export:session-package",
			ok: true,
			response: `raw=${rawRows.length} decoded=${decodedRows.length} snapshots=${frameSnapshots.length}`,
		});
	};

	return (
		<SafeAreaView style={styles.safeArea}>
			{/* Shared connection header */}
			<ConnectionHeader />

			{/* Dashboard tab */}
			{activeTab === "dashboard" ? <DashboardScreen boardState={boardState} /> : null}

			{/* Controls tab — grouped command surface */}
			{activeTab === "controls" ? (
				<ControlsScreen
					boardState={boardState}
					onRunCommand={(name) => void runCommand(name)}
				/>
			) : null}

			{/* Console tab */}
			{activeTab === "console" ? (
				<ConsoleScreen
					availableCommands={ALL_COMMANDS.map((entry) => entry.name)}
					boardState={boardState}
					selectedTransportOption={selectedTransportOption}
					transportStatus={transportStatus}
					frameCount={boardState.frameCount}
					visibleFrames={sampledVisibleFrames}
					frameFilter={frameFilter}
					busFilter={busFilter}
					frameFeedPaused={frameFeedPaused}
					boardInfoFeedPaused={boardInfoFeedPaused}
					frameWindowSize={frameWindowSize}
					frameSampleStep={frameSampleStep}
					selectedDecoderDataset={selectedDecoderDataset}
					decoderDatasets={allDecoderDatasets.map((entry) => ({
						id: entry.id,
						label: entry.label,
					}))}
					liveDecodedFeed={liveDecodedFeed}
					frameDecodedNameByKey={frameDecodedNameByKey}
					diagnosticsQuery={diagnosticsQuery}
					diagnosticsCategory={diagnosticsCategory}
					diagnosticsEvents={visibleDiagnostics}
					statusText={statusText}
					lastResult={lastResult}
					history={history}
					bleDeviceName={bleDeviceName}
					bleConfigBusy={bleConfigBusy}
					onFrameFilterChange={setFrameFilter}
					onBusFilterChange={(value) => setBusFilter(value as BusFilter)}
					onFrameWindowSizeChange={setFrameWindowSize}
					onFrameSampleStepChange={setFrameSampleStep}
					onFrameFeedPausedChange={setFrameFeedPaused}
					onBoardInfoFeedPausedChange={handleBoardInfoFeedPausedChange}
					onDiagnosticsQueryChange={setDiagnosticsQuery}
					onDiagnosticsCategoryChange={(value) =>
						setDiagnosticsCategory(value as DiagnosticsCategory)
					}
					onDatasetChange={setDecoderDatasetId}
					onRunCommand={(name, args) => runCommand(name, args)}
					onRunRawCommand={runRawConsoleCommand}
					onFetchStatus={readStatus}
					onRefreshBleStatus={refreshBleStatus}
					onBleDeviceNameInputChange={setBleDeviceName}
					onApplyBleDeviceName={applyBleDeviceName}
					onExportJson={exportVisibleFramesJson}
					onExportCsv={exportVisibleFramesCsv}
					onExportRawJson={exportRawSessionJson}
					onExportRawJsonl={exportRawSessionJsonl}
					onExportDecodedJson={exportDecodedSessionJson}
					onExportDecodedCsv={exportDecodedSessionCsv}
					onExportDatasetDbc={exportDecoderDatasetDbc}
					onExportSessionPackage={exportSessionPackage}
					onSaveSnapshot={saveFrameSnapshot}
					onClearFeed={clearMonitorFeed}
				/>
			) : null}

			{activeTab === "flasher" ? (
				<Suspense fallback={null}>
					<FlasherScreen />
				</Suspense>
			) : null}

			{/* Docs tab */}
			{activeTab === "docs" ? (
				<Suspense fallback={null}>
					<DocsScreen activeDocRoute={route.docRoute} onNavigateDoc={navigateToDoc} />
				</Suspense>
			) : null}

			{/* Separate menu header (bottom navigation) */}
			<MenuHeader tabs={TABS} activeTab={activeTab} onSelectTab={navigateToTab} />
		</SafeAreaView>
	);
}

const styles = StyleSheet.create({
	safeArea: {
		flex: 1,
		backgroundColor: colors.dashBackground,
	},
});
