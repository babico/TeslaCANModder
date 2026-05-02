import { lazy, Suspense, useEffect, useMemo, useReducer, useRef, useState } from "react";
import { StyleSheet } from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import {
	buildDecoderIndex,
	describeDecodedFrame,
	initialBoardState,
	type BoardState,
	type DecoderDataset,
} from "@teslacanmodder/protocol";

import { ALL_COMMANDS, type CommandName } from "./hardware/controller";
import legacyMcu2Payload from "./assets/can-decoder/legacy_mcu2.json";
import legacyMcu3Payload from "./assets/can-decoder/legacy_mcu3.json";
import legacyModelSXIntelPayload from "./assets/can-decoder/legacy_modelsx_intel.json";
import legacyModelSXAmdPayload from "./assets/can-decoder/legacy_modelsx_amd.json";
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
import {
	buildDiagnosticsEvents,
	filterDiagnosticsEvents,
	type DiagnosticsCategory,
	type DiagnosticsEvent,
} from "./state/monitorDiagnostics";
import {
	loadPersistedDiagnosticsState,
	savePersistedDiagnosticsState,
	type PersistedDiagnosticsArchiveEntry,
} from "./state/monitorDiagnosticsPersistence";
import { saveLiveCanFramesToIndexedDb } from "./state/monitorLiveCanPersistence";
import { applyFrameViewingPipeline, type BusFilterType } from "./state/monitorFrameViewing";
import {
	buildCommandExecutionResult,
	resolveCommandExecutionReadiness,
} from "./state/monitorCommandExecution";
import { getCommandGate } from "./state/commandGating";
import { useBoardConnection } from "./state/BoardConnectionContext";
import { DashboardScreen, ControlsScreen, ConsoleScreen } from "./screens";
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

type DecoderDatasetEntry = {
	id: string;
	label: string;
	dataset: DecoderDataset;
};

type LegacyDecodedFrame = {
	address_dec?: number;
	frame_name?: string;
	signals?: Array<{
		signal_name?: string;
		possible_values?: Array<{ value_dec: number; value_hex: string; label: string }>;
	}>;
};

type LegacyDecodedPayload = {
	dataset_source?: { vehicle?: string; firmware?: string; mcu?: string; soc?: string };
	frames?: LegacyDecodedFrame[];
};

const EXPORT_SCHEMA_VERSION = "monitor-export.v1";

const LEGACY_DECODED_DATASET_SOURCES: Array<{
	id: string;
	label: string;
	payload: LegacyDecodedPayload;
}> = [
	{
		id: "legacy-mcu2",
		label: "Legacy Explorer MCU2",
		payload: legacyMcu2Payload as LegacyDecodedPayload,
	},
	{
		id: "legacy-mcu3",
		label: "Legacy Explorer MCU3",
		payload: legacyMcu3Payload as LegacyDecodedPayload,
	},
	{
		id: "legacy-modelsx-intel",
		label: "Legacy Explorer Model S/X Intel",
		payload: legacyModelSXIntelPayload as LegacyDecodedPayload,
	},
	{
		id: "legacy-modelsx-amd",
		label: "Legacy Explorer Model S/X AMD",
		payload: legacyModelSXAmdPayload as LegacyDecodedPayload,
	},
];

const DEFAULT_DECODER_DATASET_ID = LEGACY_DECODED_DATASET_SOURCES[0]?.id ?? "";

const POLL_SECONDS = [0, 2, 5, 10];

const BUS_FILTERS = ["all", "0", "1", "2"] as const;

type BusFilter = (typeof BUS_FILTERS)[number];

type Tab = AppTabRoute;

const TABS: Array<{ id: Tab; label: string }> = [
	{ id: "dashboard", label: "Dashboard" },
	{ id: "controls", label: "Controls" },
	{ id: "console", label: "Console" },
	{ id: "flasher", label: "Flasher" },
	{ id: "docs", label: "Docs" },
];

function coerceLegacyDecoderDataset(
	payload: LegacyDecodedPayload,
	fallbackLabel: string,
): DecoderDataset | null {
	if (!payload || !Array.isArray(payload.frames)) {
		return null;
	}

	const frames = payload.frames
		.filter((frame) => typeof frame.address_dec === "number")
		.map((frame) => ({
			id: Number(frame.address_dec),
			hex: `0x${Number(frame.address_dec).toString(16).toUpperCase()}`,
			frame_name:
				frame.frame_name ?? `Frame_${Number(frame.address_dec).toString(16).toUpperCase()}`,
			signals: Array.isArray(frame.signals)
				? frame.signals.map((signal) => ({
						signal_name: signal.signal_name ?? "UnknownSignal",
						possible_values: Array.isArray(signal.possible_values)
							? signal.possible_values
							: [],
					}))
				: [],
		}));

	if (frames.length === 0) {
		return null;
	}

	return {
		dataset_source: {
			vehicle: payload.dataset_source?.vehicle ?? "Tesla",
			firmware: payload.dataset_source?.firmware ?? fallbackLabel,
			mcu: payload.dataset_source?.mcu ?? "unknown",
			soc: payload.dataset_source?.soc ?? "unknown",
		},
		frames,
	};
}

function fmtTime(epochMs: number): string {
	return new Date(epochMs).toLocaleTimeString();
}

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
	const [decoderDatasetId, setDecoderDatasetId] = useState<string>(DEFAULT_DECODER_DATASET_ID);
	const [diagnosticsQuery, setDiagnosticsQuery] = useState("");
	const [diagnosticsCategory, setDiagnosticsCategory] = useState<DiagnosticsCategory>("all");
	const [diagnosticsArchive, setDiagnosticsArchive] = useState<
		PersistedDiagnosticsArchiveEntry[]
	>([]);
	const [hasHydratedDiagnostics, setHasHydratedDiagnostics] = useState(false);
	const [bleDeviceName, setBleDeviceName] = useState("TeslaCANModder");
	const [bleConfigBusy, setBleConfigBusy] = useState(false);
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

	const legacyDecoderDatasets = useMemo(
		() =>
			LEGACY_DECODED_DATASET_SOURCES.map((source) => {
				const dataset = coerceLegacyDecoderDataset(source.payload, source.label);
				if (!dataset) {
					return null;
				}
				return {
					id: source.id,
					label: source.label,
					dataset,
				} as DecoderDatasetEntry;
			}).filter((entry): entry is DecoderDatasetEntry => Boolean(entry)),
		[],
	);

	const allDecoderDatasets = useMemo(() => legacyDecoderDatasets, [legacyDecoderDatasets]);

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

	const selectedDecoderDataset = useMemo(
		() =>
			allDecoderDatasets.find((entry) => entry.id === decoderDatasetId) ??
			allDecoderDatasets[0] ?? {
				id: "none",
				label: "No Decoder Dataset",
				dataset: {
					dataset_source: {
						vehicle: "Tesla",
						firmware: "none",
						mcu: "none",
						soc: "none",
					},
					frames: [],
				},
			},
		[allDecoderDatasets, decoderDatasetId],
	);

	const decoderIndex = useMemo(
		() => buildDecoderIndex(selectedDecoderDataset.dataset),
		[selectedDecoderDataset],
	);

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

	const diagnosticsEvents = useMemo<DiagnosticsEvent[]>(
		() =>
			buildDiagnosticsEvents({
				commandLifecycle,
				history,
				boardMessages: boardState.messages,
				frameSnapshots,
				formatTime: fmtTime,
			}),
		[commandLifecycle, history, boardState.messages, frameSnapshots],
	);

	const archivedDiagnosticsEvents = useMemo<DiagnosticsEvent[]>(
		() =>
			diagnosticsArchive.map((entry) => ({
				id: `archive-${entry.id}`,
				ts: entry.ts,
				tsLabel: fmtTime(entry.ts),
				category: entry.category,
				summary: entry.summary,
				detail: entry.detail,
				ok: entry.ok,
			})),
		[diagnosticsArchive],
	);

	const visibleDiagnostics = useMemo(
		() =>
			filterDiagnosticsEvents({
				events: [...diagnosticsEvents, ...archivedDiagnosticsEvents],
				query: diagnosticsQuery,
				category: diagnosticsCategory,
			}),
		[diagnosticsEvents, archivedDiagnosticsEvents, diagnosticsQuery, diagnosticsCategory],
	);

	useEffect(() => {
		let cancelled = false;

		const hydrateDiagnostics = async () => {
			const persisted = await loadPersistedDiagnosticsState();
			if (!persisted || cancelled) {
				setHasHydratedDiagnostics(true);
				return;
			}

			setHistory(persisted.history);
			setFrameSnapshots(
				persisted.frameSnapshots.map((snapshot) => ({
					...snapshot,
					busFilter: BUS_FILTERS.includes(snapshot.busFilter as BusFilter)
						? (snapshot.busFilter as BusFilter)
						: "all",
				})),
			);
			setDiagnosticsArchive(persisted.archive);
			setHasHydratedDiagnostics(true);
		};

		void hydrateDiagnostics();

		return () => {
			cancelled = true;
		};
	}, []);

	useEffect(() => {
		if (!hasHydratedDiagnostics) {
			return;
		}

		void savePersistedDiagnosticsState({
			history,
			frameSnapshots,
			archive: diagnosticsArchive,
		});
	}, [hasHydratedDiagnostics, history, frameSnapshots, diagnosticsArchive]);

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

	const pushDiagnosticsArchive = (
		category: PersistedDiagnosticsArchiveEntry["category"],
		summary: string,
		detail: string,
		ok = true,
	) => {
		setDiagnosticsArchive((current) =>
			[
				{
					id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
					ts: Date.now(),
					category,
					summary,
					detail,
					ok,
				},
				...current,
			].slice(0, 400),
		);
	};

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

	const refreshBleStatus = async () => {
		const trimmedBase = baseUrl.trim().replace(/\/+$/, "");
		if (!trimmedBase) {
			const message = "Base URL is required to read BLE status.";
			pushDiagnosticsArchive("system", "BLE status failed", message, false);
			setLastResult(message);
			return;
		}

		setBleConfigBusy(true);
		try {
			const response = await fetch(`${trimmedBase}/api/ble/status`);
			const raw = await response.text();
			let payload: Record<string, unknown> | null = null;
			try {
				payload = JSON.parse(raw) as Record<string, unknown>;
			} catch {
				payload = null;
			}

			if (!response.ok) {
				throw new Error(
					(payload?.error as string) || raw || `BLE status failed (${response.status})`,
				);
			}

			if (payload && typeof payload.deviceName === "string") {
				setBleDeviceName(payload.deviceName);
			}

			const enabled = payload?.enabled === true ? "on" : "off";
			const connected = payload?.connected === true ? "yes" : "no";
			pushDiagnosticsArchive(
				"system",
				"BLE status fetched",
				`enabled=${enabled} connected=${connected}`,
			);
			setLastResult(`BLE status loaded (enabled=${enabled}, connected=${connected}).`);
		} catch (error) {
			const message = error instanceof Error ? error.message : "Failed to read BLE status.";
			pushDiagnosticsArchive("system", "BLE status failed", message, false);
			setLastResult(message);
		} finally {
			setBleConfigBusy(false);
		}
	};

	const applyBleDeviceName = async () => {
		const trimmedBase = baseUrl.trim().replace(/\/+$/, "");
		const nextName = bleDeviceName.trim();

		if (!trimmedBase) {
			const message = "Base URL is required to update BLE name.";
			pushDiagnosticsArchive("system", "BLE name update failed", message, false);
			setLastResult(message);
			return;
		}

		if (!nextName || nextName.length > 32) {
			const message = "BLE name must be between 1 and 32 characters.";
			pushDiagnosticsArchive("system", "BLE name update failed", message, false);
			setLastResult(message);
			return;
		}

		setBleConfigBusy(true);
		try {
			const response = await fetch(`${trimmedBase}/api/ble/config`, {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify({ name: nextName }),
			});

			const raw = await response.text();
			let payload: Record<string, unknown> | null = null;
			try {
				payload = JSON.parse(raw) as Record<string, unknown>;
			} catch {
				payload = null;
			}

			if (!response.ok) {
				throw new Error(
					(payload?.error as string) ||
						raw ||
						`BLE name update failed (${response.status})`,
				);
			}

			if (payload && typeof payload.deviceName === "string") {
				setBleDeviceName(payload.deviceName);
			}

			pushDiagnosticsArchive("system", "BLE name updated", `name=${nextName}`);
			setLastResult(`BLE name updated to "${nextName}".`);
		} catch (error) {
			const message = error instanceof Error ? error.message : "Failed to update BLE name.";
			pushDiagnosticsArchive("system", "BLE name update failed", message, false);
			setLastResult(message);
		} finally {
			setBleConfigBusy(false);
		}
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
	tabBar: {
		flexDirection: "row",
		backgroundColor: colors.dashCard,
		borderBottomWidth: 1,
		borderBottomColor: colors.dashCardBorder,
	},
	tabItem: {
		flex: 1,
		paddingVertical: 10,
		alignItems: "center",
	},
	tabItemActive: {
		borderBottomWidth: 2,
		borderBottomColor: colors.dashPrimary,
	},
	tabLabel: {
		color: colors.dashLabel,
		fontSize: 13,
		fontWeight: "600",
	},
	tabLabelActive: {
		color: colors.dashValue,
	},
	page: {
		padding: 16,
		paddingBottom: 28,
		gap: 14,
	},
	monitorHero: {
		borderRadius: 20,
		padding: 18,
		borderWidth: 1,
		borderColor: "#1f4c72",
		backgroundColor: "#0b1c2f",
		elevation: 4,
		gap: 10,
	},
	heroFeatureRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	monitorFeaturePill: {
		minWidth: 128,
		flexGrow: 1,
		borderRadius: 14,
		paddingHorizontal: 12,
		paddingVertical: 10,
		backgroundColor: "rgba(36, 76, 112, 0.45)",
		borderWidth: 1,
		borderColor: "rgba(92, 143, 184, 0.45)",
		gap: 3,
	},
	monitorFeatureTitle: {
		color: "#d8efff",
		fontSize: 11,
		fontWeight: "800",
		textTransform: "uppercase",
		letterSpacing: 0.7,
	},
	monitorFeatureDetail: {
		color: "#9fc4e3",
		fontSize: 12,
		fontWeight: "600",
	},
	sectionHeader: {
		gap: 4,
		marginTop: 2,
	},
	sectionTitle: {
		color: "#e3f0ff",
		fontSize: 18,
		fontWeight: "800",
	},
	sectionDetail: {
		color: "#9ab4d0",
		fontSize: 12,
	},
	heroGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 10,
	},
	metricCard: {
		minWidth: 160,
		flexGrow: 1,
		borderRadius: 14,
		padding: 12,
		backgroundColor: "#10263d",
		borderWidth: 1,
		borderColor: "#295075",
	},
	metricLabel: {
		color: "#9ec5ea",
		fontSize: 12,
		fontWeight: "700",
		textTransform: "uppercase",
	},
	metricValue: {
		color: "#ffffff",
		fontSize: 20,
		fontWeight: "800",
		marginTop: 6,
	},
	metricDetail: {
		color: "#bfd8f1",
		fontSize: 12,
		marginTop: 2,
	},
	eyebrow: {
		color: "#89c8f0",
		fontSize: 12,
		fontWeight: "700",
		letterSpacing: 1,
	},
	frameDecode: {
		marginTop: 4,
		color: "#8ca6c0",
		fontSize: 11,
	},
	title: {
		color: "#ffffff",
		fontSize: 28,
		fontWeight: "800",
	},
	subtitle: {
		color: "#9ab4d0",
		fontSize: 14,
		marginBottom: 2,
	},
	subtitleHero: {
		color: "#b8d7ef",
		fontSize: 14,
		lineHeight: 20,
	},
	grid: {
		gap: 12,
	},
	gridWide: {
		flexDirection: "row",
		flexWrap: "wrap",
	},
	gridStack: {
		flexDirection: "column",
	},
	card: {
		backgroundColor: "#0d1f33",
		borderRadius: 14,
		borderWidth: 1,
		borderColor: "#26486b",
		padding: 12,
		gap: 8,
		minWidth: 300,
		flexGrow: 1,
		elevation: 2,
	},
	cardBanner: {
		alignSelf: "flex-start",
		borderRadius: 999,
		paddingHorizontal: 10,
		paddingVertical: 4,
		backgroundColor: "#10344e",
		borderWidth: 1,
		borderColor: "#2d5f80",
	},
	cardBannerAlt: {
		alignSelf: "flex-start",
		borderRadius: 999,
		paddingHorizontal: 10,
		paddingVertical: 4,
		backgroundColor: "#1b2e52",
		borderWidth: 1,
		borderColor: "#334a81",
	},
	cardBannerText: {
		color: "#b6ecff",
		fontSize: 10,
		fontWeight: "800",
		letterSpacing: 0.7,
	},
	cardBannerTextAlt: {
		color: "#c4d4ff",
		fontSize: 10,
		fontWeight: "800",
		letterSpacing: 0.7,
	},
	fullWidth: {
		width: "100%",
	},
	telemetryGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	statBlock: {
		minWidth: 124,
		flexGrow: 1,
		borderRadius: 10,
		borderWidth: 1,
		borderColor: "#2c4f72",
		backgroundColor: "#122a42",
		paddingHorizontal: 10,
		paddingVertical: 8,
	},
	statLabel: {
		color: "#9ec5ea",
		fontSize: 11,
		fontWeight: "700",
		textTransform: "uppercase",
	},
	statValue: {
		color: "#e5f4ff",
		fontSize: 15,
		fontWeight: "700",
		marginTop: 3,
	},
	cardTitle: {
		color: "#e3f0ff",
		fontSize: 17,
		fontWeight: "700",
	},
	label: {
		color: "#a8c2de",
		fontSize: 12,
		fontWeight: "600",
	},
	blockLabel: {
		marginTop: 4,
	},
	smallLabel: {
		color: "#a8c2de",
		fontSize: 12,
	},
	input: {
		borderWidth: 1,
		borderColor: "#34597f",
		borderRadius: 8,
		paddingHorizontal: 10,
		paddingVertical: 8,
		color: "#e6f3ff",
		backgroundColor: "#0a1726",
	},
	buttonRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
		marginTop: 2,
	},
	button: {
		borderRadius: 8,
		backgroundColor: "#0a9ab7",
		paddingHorizontal: 12,
		paddingVertical: 9,
	},
	buttonSecondary: {
		backgroundColor: "#16324d",
		borderWidth: 1,
		borderColor: "#2d5a86",
	},
	buttonDisabled: {
		backgroundColor: "#30465f",
	},
	buttonText: {
		color: "#ffffff",
		fontWeight: "700",
	},
	buttonTextSecondary: {
		color: "#d6ecff",
	},
	buttonTextDisabled: {
		color: "#8a9aab",
	},
	presetRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	presetChip: {
		borderRadius: 999,
		backgroundColor: "#17324d",
		paddingHorizontal: 10,
		paddingVertical: 6,
		borderWidth: 1,
		borderColor: "#2b557f",
	},
	presetChipText: {
		color: "#d8ebff",
		fontSize: 12,
		fontWeight: "600",
	},
	transportChipRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	transportChip: {
		minWidth: 112,
		flexGrow: 1,
		borderRadius: 12,
		borderWidth: 1,
		borderColor: "#31577d",
		backgroundColor: "#11263d",
		paddingHorizontal: 10,
		paddingVertical: 9,
		gap: 2,
	},
	transportChipActive: {
		borderColor: "#22d3ee",
		backgroundColor: "#0f3a52",
	},
	transportChipTitle: {
		color: "#d6ebff",
		fontSize: 12,
		fontWeight: "700",
	},
	transportChipTitleActive: {
		color: "#bcf2ff",
	},
	transportChipDetail: {
		color: "#8faecb",
		fontSize: 11,
		textTransform: "uppercase",
	},
	transportChipDetailActive: {
		color: "#9fe7f8",
	},
	transportStatusCard: {
		borderRadius: 10,
		borderWidth: 1,
		paddingHorizontal: 10,
		paddingVertical: 9,
		gap: 4,
	},
	transportStatusReady: {
		backgroundColor: "#eefaf4",
		borderColor: "#b5dbc6",
	},
	transportStatusPending: {
		backgroundColor: "#fff6e8",
		borderColor: "#e5cd9c",
	},
	transportStatusTitle: {
		color: "#d6ebff",
		fontSize: 12,
		fontWeight: "700",
	},
	transportStatusDetail: {
		color: "#b9cde2",
		fontSize: 12,
		lineHeight: 17,
	},
	transportHelper: {
		color: "#a8c0d8",
		fontSize: 12,
		lineHeight: 18,
	},
	quickChip: {
		borderRadius: 8,
		backgroundColor: "#f2f8ed",
		paddingHorizontal: 10,
		paddingVertical: 6,
		borderWidth: 1,
		borderColor: "#cedfbe",
	},
	quickChipGated: {
		backgroundColor: "#f6f1eb",
		borderColor: "#d9c4a8",
		opacity: 0.7,
	},
	quickChipText: {
		color: "#294426",
		fontSize: 12,
		fontWeight: "600",
	},
	quickChipTextGated: {
		color: "#8a6a3a",
	},
	commandList: {
		maxHeight: 148,
		borderWidth: 1,
		borderColor: "#2b4f74",
		borderRadius: 10,
		backgroundColor: "#0a1726",
	},
	commandListContent: {
		padding: 8,
		gap: 6,
	},
	commandChip: {
		borderRadius: 8,
		borderWidth: 1,
		borderColor: "#2d5379",
		paddingHorizontal: 9,
		paddingVertical: 6,
		backgroundColor: "#11263d",
	},
	commandChipActive: {
		borderColor: "#0a9ab7",
		backgroundColor: "#ddf7fc",
	},
	commandChipGated: {
		borderColor: "#d9c4a8",
		backgroundColor: "#fdf6ec",
		opacity: 0.72,
	},
	commandChipText: {
		color: "#d5eaff",
		fontSize: 12,
	},
	commandChipTextActive: {
		color: "#0d5160",
		fontWeight: "700",
	},
	commandChipTextGated: {
		color: "#8a6a3a",
	},
	chipPressed: {
		opacity: 0.78,
	},
	gateWarning: {
		borderRadius: 8,
		backgroundColor: "#fef3dc",
		borderWidth: 1,
		borderColor: "#e9c47a",
		paddingHorizontal: 10,
		paddingVertical: 7,
	},
	gateWarningText: {
		color: "#7a4b0a",
		fontSize: 12,
	},
	inlineRow: {
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "space-between",
		gap: 8,
	},
	pollChip: {
		borderRadius: 999,
		borderWidth: 1,
		borderColor: "#cad7e6",
		paddingHorizontal: 10,
		paddingVertical: 5,
		backgroundColor: "#ffffff",
	},
	pollChipActive: {
		borderColor: "#3c6ea0",
		backgroundColor: "#e8f0fa",
	},
	pollChipText: {
		color: "#284665",
		fontSize: 12,
		fontWeight: "600",
	},
	pollChipTextActive: {
		color: "#1d3551",
	},
	panelLabel: {
		color: "#a8c2de",
		fontSize: 12,
		fontWeight: "700",
		marginTop: 4,
	},
	outputPanel: {
		maxHeight: 130,
		borderRadius: 10,
		backgroundColor: "#081422",
		padding: 10,
	},
	outputPanelLarge: {
		maxHeight: 250,
		borderRadius: 10,
		backgroundColor: "#081422",
		padding: 10,
	},
	outputText: {
		color: "#d8f1ff",
		fontSize: 12,
		fontFamily: "Courier",
	},
	stateChip: {
		borderRadius: 999,
		paddingHorizontal: 10,
		paddingVertical: 6,
		borderWidth: 1,
	},
	stateChipActive: {
		backgroundColor: "#def6e8",
		borderColor: "#a6d9bc",
	},
	stateChipInactive: {
		backgroundColor: "#14263a",
		borderColor: "#2d4d70",
	},
	stateChipText: {
		fontSize: 12,
		fontWeight: "600",
		color: "#b7cce1",
	},
	stateChipTextActive: {
		color: "#215030",
	},
	feedList: {
		gap: 8,
	},
	feedRow: {
		flexDirection: "row",
		gap: 8,
		alignItems: "flex-start",
		padding: 8,
		borderRadius: 10,
		borderWidth: 1,
		borderColor: "#2c4f72",
		backgroundColor: "#10263d",
	},
	feedTextWrap: {
		flex: 1,
		gap: 2,
	},
	feedTypeInfo: {
		minWidth: 48,
		color: "#12597f",
		fontSize: 11,
		fontWeight: "800",
	},
	feedTypeError: {
		minWidth: 48,
		color: "#9c1625",
		fontSize: 11,
		fontWeight: "800",
	},
	feedText: {
		color: "#d6ebff",
		fontSize: 12,
	},
	feedTime: {
		color: "#9db7d2",
		fontSize: 11,
	},
	frameList: {
		gap: 8,
	},
	frameListContainer: {
		maxHeight: 320,
	},
	perfStatText: {
		color: "#a9c2de",
		fontSize: 12,
	},
	perfPass: {
		color: "#1f6b2c",
	},
	perfFail: {
		color: "#8a1523",
	},
	frameRow: {
		borderRadius: 10,
		borderWidth: 1,
		borderColor: "#2c4f72",
		backgroundColor: "#10263d",
		padding: 8,
		gap: 6,
	},
	frameRowActive: {
		borderColor: "#3c6ea0",
		backgroundColor: "#e8f0fa",
	},
	frameMeta: {
		flexDirection: "row",
		justifyContent: "space-between",
		gap: 8,
	},
	framePrimary: {
		color: "#d9edff",
		fontSize: 12,
		fontWeight: "700",
	},
	frameSecondary: {
		color: "#9eb7d2",
		fontSize: 11,
	},
	framePayload: {
		color: "#e5f3ff",
		fontSize: 12,
		fontFamily: "Courier",
	},
	decodedCard: {
		borderRadius: 10,
		borderWidth: 1,
		borderColor: "#2d5379",
		backgroundColor: "#122a42",
		padding: 10,
		gap: 8,
	},
	decodedTitle: {
		color: "#d5eaff",
		fontSize: 13,
		fontWeight: "700",
	},
	decodedList: {
		gap: 8,
	},
	decodedItem: {
		gap: 4,
		paddingBottom: 6,
		borderBottomWidth: 1,
		borderBottomColor: "#2b4f74",
	},
	decodedFrameName: {
		color: "#d6ebff",
		fontSize: 12,
		fontWeight: "700",
	},
	decodedSignalList: {
		gap: 4,
	},
	decodedSignalRow: {
		gap: 2,
	},
	decodedSignal: {
		color: "#c3dbf4",
		fontSize: 12,
		fontWeight: "600",
	},
	decodedValues: {
		color: "#97b2cd",
		fontSize: 11,
	},
	historyList: {
		gap: 6,
	},
	historyRow: {
		borderRadius: 8,
		borderWidth: 1,
		borderColor: "#2b4f74",
		backgroundColor: "#10263d",
		padding: 8,
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "space-between",
		gap: 8,
	},
	historyMeta: {
		flexShrink: 1,
		gap: 2,
	},
	historyCommand: {
		color: "#d6ebff",
		fontSize: 12,
		fontWeight: "600",
	},
	historyTime: {
		color: "#9db7d2",
		fontSize: 11,
	},
	okBadge: {
		color: "#1f6b2c",
		backgroundColor: "#ddf5dd",
		borderWidth: 1,
		borderColor: "#a8d9af",
		borderRadius: 999,
		paddingHorizontal: 8,
		paddingVertical: 3,
		fontWeight: "700",
		fontSize: 11,
	},
	errBadge: {
		color: "#8a1523",
		backgroundColor: "#ffe7eb",
		borderWidth: 1,
		borderColor: "#f2b8c0",
		borderRadius: 999,
		paddingHorizontal: 8,
		paddingVertical: 3,
		fontWeight: "700",
		fontSize: 11,
	},
	emptyText: {
		color: "#9db7d2",
		fontSize: 12,
	},
});
