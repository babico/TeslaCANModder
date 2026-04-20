/**
 * BoardConnectionContext
 *
 * Provider for HardwareController state, board data, and connection
 * config. Wrap the app root (or AppExperience) with <BoardConnectionProvider>
 * and consume via the useBoardConnection() hook from any screen.
 */

import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useReducer,
  useRef,
  useState,
  type ReactNode,
} from "react";

import {
  initialBoardState,
  type BoardState,
} from "@teslacanmodder/protocol";

import {
  ALL_COMMANDS,
  DEFAULT_CONNECTION,
  HardwareController,
  type CommandName,
} from "../hardware/controller";
import { reduceBoardPayload } from "./board";
import {
  commandBusReducer,
  generateCommandId,
  initialCommandBusState,
  type CommandLifecycleEntry,
} from "./commandBus";
import {
  buildCommandExecutionResult,
  resolveCommandExecutionReadiness,
  type CommandExecutionResult as MonitorCommandResult,
} from "./monitorCommandExecution";
import { getCommandGate } from "./commandGating";
import {
  buildMonitorTransportStatus,
  getMonitorTransportExecutionPolicy,
  getMonitorTransportOption,
  type MonitorTransportType,
} from "../hardware/transportPresentation";
import { buildMonitorTransportGateSnapshot } from "../hardware/monitorPollingPolicy";
import { buildApplyTransportMessage } from "../hardware/monitorTransportMessages";
import {
  connectRuntimeBlePeripheral,
  requestRuntimeSerialPort,
} from "../hardware/runtimeAdapters";
import type { HardwareConnectionConfig } from "../types/controls";

// ── Types ─────────────────────────────────────────────────────────────────────

export type ConnectionPreset = {
  name: string;
  connection: HardwareConnectionConfig;
};

export const CONNECTION_PRESETS: ConnectionPreset[] = [
  { name: "Vehicle AP",    connection: DEFAULT_CONNECTION },
  { name: "Local Bridge",  connection: { baseUrl: "http://localhost:8080",  commandPath: "/api/command", statusPath: "/api/status" } },
  { name: "Lab Rig",       connection: { baseUrl: "http://192.168.10.20",   commandPath: "/api/command", statusPath: "/api/status" } },
];

export type CommandHistoryEntry = {
  id: string;
  ts: number;
  command: string;
  ok: boolean;
  response: string;
};

export interface BoardConnectionState {
  // Connection config
  baseUrl: string;
  commandPath: string;
  statusPath: string;
  selectedTransportType: MonitorTransportType;

  // Board data
  boardState: BoardState;

  // Command bus lifecycle
  commandLifecycle: CommandLifecycleEntry[];

  // UI state
  statusText: string;
  lastResult: string;
  history: CommandHistoryEntry[];
  connectionBusy: boolean;

  // Derived
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
  runCommand: (name: CommandName, rawArgs?: string) => Promise<MonitorCommandResult | null>;
  fetchStatus: () => Promise<void>;
  pauseFrameFeed: (paused: boolean) => void;
  clearFrames: () => void;
  controller: HardwareController;
}

export type BoardConnectionContextValue = BoardConnectionState & BoardConnectionActions;

// ── Context ────────────────────────────────────────────────────────────────────

const BoardConnectionContext = createContext<BoardConnectionContextValue | null>(null);

// ── Provider ───────────────────────────────────────────────────────────────────

export function BoardConnectionProvider({ children }: { children: ReactNode }) {
  const controller = useMemo(() => new HardwareController(DEFAULT_CONNECTION), []);

  const [baseUrl, setBaseUrl] = useState(DEFAULT_CONNECTION.baseUrl);
  const [commandPath, setCommandPath] = useState(DEFAULT_CONNECTION.commandPath);
  const [statusPath, setStatusPath] = useState(DEFAULT_CONNECTION.statusPath);
  const [selectedTransportType, setSelectedTransportType] = useState<MonitorTransportType>("http");
  const [boardState, setBoardState] = useState<BoardState>(initialBoardState);
  const [frameFeedPaused, setFrameFeedPaused] = useState(false);
  const [statusText, setStatusText] = useState("No status fetched");
  const [lastResult, setLastResult] = useState("Ready");
  const [history, setHistory] = useState<CommandHistoryEntry[]>([]);
  const [connectionBusy, setConnectionBusy] = useState(false);
  const [busState, busDispatch] = useReducer(commandBusReducer, undefined, initialCommandBusState);
  const commandLifecycle = busState.entries;
  const nextMessageId = useRef(1);

  // ── Derived transport state ─────────────────────────────────────────────────

  const selectedTransportOption = getMonitorTransportOption(selectedTransportType);
  const transportExecutionPolicy = getMonitorTransportExecutionPolicy(
    selectedTransportType,
    controller.activeTransportType,
  );
  const transportGateSnapshot = buildMonitorTransportGateSnapshot(
    transportExecutionPolicy.ready,
    transportExecutionPolicy.blockReason,
  );
  const isSelectedTransportReady = transportGateSnapshot.canExecuteCommands;
  const canFetchStatus = transportGateSnapshot.canFetchStatus;

  // ── Board payload reducer ───────────────────────────────────────────────────

  const applyBoardPayload = useCallback((raw: unknown) => {
    setBoardState((current) => {
      const next = reduceBoardPayload(current, raw, () => nextMessageId.current++);
      if (!next) return current;
      if (frameFeedPaused) {
        return { ...next, frames: current.frames, frameCount: current.frameCount };
      }
      return next;
    });
  }, [frameFeedPaused]);

  // ── Connection actions ──────────────────────────────────────────────────────

  const applyConnection = useCallback(async () => {
    setConnectionBusy(true);

    try {
      if (selectedTransportType === "http") {
        controller.connectViaRestApi({ baseUrl, commandPath, statusPath });
      } else if (selectedTransportType === "ble") {
        const peripheral = await connectRuntimeBlePeripheral();
        controller.connectViaBle(peripheral);
      } else if (selectedTransportType === "bluetooth-serial") {
        const port = await requestRuntimeSerialPort();
        controller.connectViaBluetoothComPort(port);
      } else {
        const port = await requestRuntimeSerialPort();
        controller.connectViaComPort(port);
      }

      const message = buildApplyTransportMessage(
        selectedTransportType,
        selectedTransportOption,
        baseUrl,
        commandPath,
      );
      setLastResult(message);
      setStatusText(`Connection ready via ${selectedTransportOption.label}`);
    } catch (error) {
      const message = error instanceof Error ? error.message : "Failed to apply connection.";
      setLastResult(message);
      setStatusText(message);
    } finally {
      setConnectionBusy(false);
    }
  }, [selectedTransportType, controller, baseUrl, commandPath, statusPath, selectedTransportOption]);

  const applyPreset = useCallback((preset: ConnectionPreset) => {
    setBaseUrl(preset.connection.baseUrl);
    setCommandPath(preset.connection.commandPath);
    setStatusPath(preset.connection.statusPath);
    controller.connectViaRestApi(preset.connection);
    setSelectedTransportType("http");
    setLastResult(`Preset applied: ${preset.name}`);
  }, [controller]);

  // ── History append ──────────────────────────────────────────────────────────

  const pushHistory = useCallback((entry: Omit<CommandHistoryEntry, "id" | "ts">) => {
    setHistory((current) => [
      { id: `${Date.now()}-${Math.random().toString(16).slice(2)}`, ts: Date.now(), ...entry },
      ...current,
    ].slice(0, 40));
  }, []);

  // ── Run command ─────────────────────────────────────────────────────────────

  const runCommand = useCallback(async (
    name: CommandName,
    rawArgs = "",
  ): Promise<MonitorCommandResult | null> => {
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
  }, [boardState, isSelectedTransportReady, transportGateSnapshot, controller, applyBoardPayload, pushHistory]);

  // ── Fetch status ─────────────────────────────────────────────────────────────

  const fetchStatus = useCallback(async () => {
    if (!canFetchStatus) {
      setStatusText("Transport not ready for status fetch");
      return;
    }
    try {
      const status = await controller.readStatus();
      setStatusText(JSON.stringify(status, null, 2));
      if (status) applyBoardPayload(status);
    } catch (err) {
      setStatusText(`Error: ${String(err)}`);
    }
  }, [canFetchStatus, controller, applyBoardPayload]);

  // ── Frame feed controls ───────────────────────────────────────────────────────

  const pauseFrameFeed = useCallback((paused: boolean) => {
    setFrameFeedPaused(paused);
  }, []);

  const clearFrames = useCallback(() => {
    setBoardState((current) => ({ ...current, frames: [], frameCount: 0 }));
  }, []);

  // ── Context value ─────────────────────────────────────────────────────────────

  const value: BoardConnectionContextValue = {
    baseUrl,
    commandPath,
    statusPath,
    selectedTransportType,
    boardState,
    commandLifecycle,
    statusText,
    lastResult,
    history,
    connectionBusy,
    isSelectedTransportReady,
    canFetchStatus,
    setBaseUrl,
    setCommandPath,
    setStatusPath,
    setSelectedTransportType,
    applyConnection,
    applyPreset,
    runCommand,
    fetchStatus,
    pauseFrameFeed,
    clearFrames,
    controller,
  };

  return (
    <BoardConnectionContext.Provider value={value}>
      {children}
    </BoardConnectionContext.Provider>
  );
}

// ── Hook ───────────────────────────────────────────────────────────────────────

export function useBoardConnection(): BoardConnectionContextValue {
  const ctx = useContext(BoardConnectionContext);
  if (!ctx) {
    throw new Error("useBoardConnection must be used inside <BoardConnectionProvider>");
  }
  return ctx;
}
