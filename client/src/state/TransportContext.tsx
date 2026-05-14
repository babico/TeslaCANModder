/**
 * TransportContext
 *
 * Isolated transport-selection and connection-lifecycle state.
 * Extracted from BoardConnectionContext (Phase B refactor).
 */

import { createContext, useCallback, useContext, useState, type ReactNode } from "react";

import { DEFAULT_CONNECTION, HardwareController } from "../hardware/controller";
import { type MonitorTransportType } from "../hardware/transportPresentation";
import { buildMonitorTransportGateSnapshot } from "../hardware/monitorPollingPolicy";
import { getMonitorTransportExecutionPolicy } from "../hardware/transportPresentation";
import { connectRuntimeBlePeripheral, requestRuntimeSerialPort } from "../hardware/runtimeAdapters";
import type { HardwareConnectionConfig } from "../types/controls";

// ── Types ─────────────────────────────────────────────────────────────────────

export type ConnectionPreset = {
	name: string;
	connection: HardwareConnectionConfig;
};

export const CONNECTION_PRESETS: ConnectionPreset[] = [
	{ name: "Vehicle AP", connection: DEFAULT_CONNECTION },
	{
		name: "Local Bridge",
		connection: {
			baseUrl: "http://localhost:8080",
			commandPath: "/api/command",
			statusPath: "/api/status",
		},
	},
	{
		name: "Lab Rig",
		connection: {
			baseUrl: "http://192.168.10.20",
			commandPath: "/api/command",
			statusPath: "/api/status",
		},
	},
];

export interface TransportState {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
	selectedTransportType: MonitorTransportType;
	isSelectedTransportReady: boolean;
	connectionBusy: boolean;
}

export interface TransportActions {
	setBaseUrl: (url: string) => void;
	setCommandPath: (path: string) => void;
	setStatusPath: (path: string) => void;
	setSelectedTransportType: (type: MonitorTransportType) => void;
	applyConnection: () => Promise<void>;
	applyPreset: (preset: ConnectionPreset) => void;
	disconnectTransport: () => Promise<void>;
}

// ── State-only hook ────────────────────────────────────────────────────────────

export function useTransportState(): TransportState {
	const ctx = useContext(_TransportContext);
	if (!ctx) {
		throw new Error("useTransportState must be used inside <TransportProvider>");
	}
	return ctx.state;
}

// ── Actions-only hook ──────────────────────────────────────────────────────────

export function useTransportActions(): TransportActions & { controller: HardwareController } {
	const ctx = useContext(_TransportContext);
	if (!ctx) {
		throw new Error("useTransportActions must be used inside <TransportProvider>");
	}
	return ctx.actions;
}

// ── Context internals ──────────────────────────────────────────────────────────

interface TransportContextValue {
	state: TransportState;
	actions: TransportActions & { controller: HardwareController };
}

const _TransportContext = createContext<TransportContextValue | null>(null);

// ── Provider ───────────────────────────────────────────────────────────────────

export function TransportProvider({
	children,
	controller,
}: {
	children: ReactNode;
	controller: HardwareController;
}) {
	const [baseUrl, setBaseUrl] = useState(DEFAULT_CONNECTION.baseUrl);
	const [commandPath, setCommandPath] = useState(DEFAULT_CONNECTION.commandPath);
	const [statusPath, setStatusPath] = useState(DEFAULT_CONNECTION.statusPath);
	const [selectedTransportType, setSelectedTransportType] =
		useState<MonitorTransportType>("http");
	const [connectionBusy, setConnectionBusy] = useState(false);

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

	// ── Actions ────────────────────────────────────────────────────────────────

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
		} catch {
			// Error surface is handled by the caller via lastResult
		} finally {
			setConnectionBusy(false);
		}
	}, [selectedTransportType, controller, baseUrl, commandPath, statusPath]);

	const applyPreset = useCallback(
		(preset: ConnectionPreset) => {
			setBaseUrl(preset.connection.baseUrl);
			setCommandPath(preset.connection.commandPath);
			setStatusPath(preset.connection.statusPath);
			controller.connectViaRestApi(preset.connection);
			setSelectedTransportType("http");
		},
		[controller],
	);

	const disconnectTransport = useCallback(async () => {
		setConnectionBusy(true);

		try {
			await controller.disconnectTransport();
		} catch {
			// Error surface is handled by the caller via lastResult
		} finally {
			setConnectionBusy(false);
		}
	}, [controller]);

	const value: TransportContextValue = {
		state: {
			baseUrl,
			commandPath,
			statusPath,
			selectedTransportType,
			isSelectedTransportReady,
			connectionBusy,
		},
		actions: {
			setBaseUrl,
			setCommandPath,
			setStatusPath,
			setSelectedTransportType,
			applyConnection,
			applyPreset,
			disconnectTransport,
			controller,
		},
	};

	return <_TransportContext.Provider value={value}>{children}</_TransportContext.Provider>;
}
