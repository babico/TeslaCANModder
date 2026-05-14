import { useMemo, useState } from "react";
import { ScrollView, Text, View } from "react-native";

import {
	MONITOR_TRANSPORT_OPTIONS,
	type MonitorTransportType,
} from "../hardware/transportPresentation";
import { useBoardConnection } from "../state/BoardConnectionContext";
import { CONNECTION_PRESETS } from "../state/TransportContext";
import { Sheet } from "../ui/shadcn/sheet";
import { TransportPicker } from "./transport/TransportPicker";
import { ConnectSheetContent } from "./transport/ConnectSheet";
import { ConnectionStatusBar } from "./transport/ConnectionStatusBar";

function normalizeTransportType(type: MonitorTransportType): MonitorTransportType {
	return type === "bluetooth-serial" ? "serial" : type;
}

function formatTransportLabel(type: MonitorTransportType): string {
	return (
		MONITOR_TRANSPORT_OPTIONS.find((option) => option.id === normalizeTransportType(type))
			?.label ?? type.toUpperCase()
	);
}

function buildTransportHint(type: MonitorTransportType, baseUrl: string): string {
	switch (normalizeTransportType(type)) {
		case "http":
			return baseUrl;
		case "ble":
			return "Web Bluetooth NUS picker";
		case "serial":
			return "USB serial or Bluetooth COM picker";
		default:
			return type;
	}
}

export function ConnectionHeader() {
	const conn = useBoardConnection();
	const [sheetOpen, setSheetOpen] = useState(false);

	const isReady = conn.isSelectedTransportReady;

	const transportLabel = useMemo(
		() => formatTransportLabel(conn.selectedTransportType),
		[conn.selectedTransportType],
	);
	const transportHint = useMemo(
		() => buildTransportHint(conn.selectedTransportType, conn.baseUrl),
		[conn.selectedTransportType, conn.baseUrl],
	);

	async function handleApplyConnection(): Promise<void> {
		await conn.applyConnection();
		setSheetOpen(false);
	}

	return (
		<>
			<ConnectionStatusBar
				isReady={isReady}
				isBusy={conn.connectionBusy}
				transportLabel={transportLabel}
				transportHint={transportHint}
				onOpenSheet={() => setSheetOpen(true)}
			/>

			<Sheet open={sheetOpen} onClose={() => setSheetOpen(false)} title="Board Connection">
				<ScrollView contentContainerStyle={{ gap: 12, paddingBottom: 12 }}>
					<View className="rounded-xl border border-border bg-muted p-3 gap-1">
						<Text className="text-sm font-bold text-card-foreground">
							{transportLabel}
						</Text>
						<Text className="text-sm text-muted-foreground leading-4">
							{transportHint}
						</Text>
					</View>

					<View className="gap-1">
						<Text className="text-xs font-bold uppercase text-muted-foreground">
							Transport
						</Text>
						<TransportPicker
							selected={conn.selectedTransportType}
							onChange={conn.setSelectedTransportType}
						/>
					</View>

					<ConnectSheetContent
						transportType={conn.selectedTransportType}
						baseUrl={conn.baseUrl}
						commandPath={conn.commandPath}
						statusPath={conn.statusPath}
						connectionBusy={conn.connectionBusy}
						lastResult={conn.lastResult}
						onChangeBaseUrl={conn.setBaseUrl}
						onChangeCommandPath={conn.setCommandPath}
						onChangeStatusPath={conn.setStatusPath}
						onApplyPreset={(preset) => {
							void conn.applyPreset(preset);
							setSheetOpen(false);
						}}
						onConnect={() => void handleApplyConnection()}
						presets={CONNECTION_PRESETS}
					/>
				</ScrollView>
			</Sheet>
		</>
	);
}
