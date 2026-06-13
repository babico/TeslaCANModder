import { Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";

function Section({
	title,
	description,
	children,
}: {
	title: string;
	description?: string;
	children: React.ReactNode;
}) {
	return (
		<View className="bg-card border border-border rounded-xl p-3 gap-2">
			<Text className="text-xs font-bold uppercase tracking-wider text-muted-foreground">
				{title}
			</Text>
			{description ? (
				<Text className="text-xs text-muted-foreground/70 leading-4">{description}</Text>
			) : null}
			<View className="gap-1">{children}</View>
		</View>
	);
}

function Row({ label, value, dimmed }: { label: string; value: string; dimmed?: boolean }) {
	return (
		<View className="flex-row justify-between items-center">
			<Text className="text-sm text-muted-foreground">{label}</Text>
			<Text
				className={`text-sm font-semibold text-right flex-1 ${dimmed ? "text-muted-foreground/50" : "text-card-foreground"}`}
			>
				{value}
			</Text>
		</View>
	);
}

export interface IntegrationPanelProps {
	state: BoardState;
}

export function IntegrationPanel({ state }: IntegrationPanelProps) {
	return (
		<View className="gap-3">
			<Section
				title="Connectivity & Integrations"
				description="Live bridge and transport links between the board and external tools."
			>
				<Row
					label="MQTT"
					value={
						state.mqtt
							? state.mqttConnected
								? "Enabled · Connected"
								: "Enabled · Disconnected"
							: "Disabled"
					}
					dimmed={!state.mqtt}
				/>
				<Row
					label="Tesla BLE"
					value={
						state.teslaBle
							? state.teslaBleConnected
								? state.teslaBleAuth
									? "Connected · Authenticated"
									: "Connected · No auth"
								: "Enabled · Disconnected"
							: "Disabled"
					}
					dimmed={!state.teslaBle}
				/>
				<Row
					label="Home Assistant"
					value={
						state.homeAssistant
							? state.haConnected
								? `Enabled · Connected (${state.haEntities} entities)`
								: `Enabled · Disconnected (${state.haEntities} entities)`
							: "Disabled"
					}
					dimmed={!state.homeAssistant}
				/>
				<Row
					label="ESP-NOW"
					value={
						state.espNow
							? `Enabled · CH ${state.espNowChannel} · ${state.espNowPeers} peers`
							: "Disabled"
					}
					dimmed={!state.espNow}
				/>
				<Row
					label="GVRET"
					value={
						state.gvret
							? `Enabled · Port ${state.gvretPort} · ${state.gvretClients} clients`
							: "Disabled"
					}
					dimmed={!state.gvret}
				/>
				<Row
					label="ScanMyTesla"
					value={state.scanMyTesla ? "Enabled" : "Disabled"}
					dimmed={!state.scanMyTesla}
				/>
				<Row
					label="ELM327"
					value={state.elm327 ? "Enabled" : "Disabled"}
					dimmed={!state.elm327}
				/>
			</Section>

			<Section
				title="Integration Notes"
				description="How to interpret integration health and where each signal originates."
			>
				<Row label="MQTT / Home Assistant" value="Bridge state shown from firmware" />
				<Row label="Tesla BLE" value="Auth and link status surfaced separately" />
				<Row
					label="GVRET / ESP-NOW"
					value="Runtime peers/clients indicate active consumers"
				/>
				<Row
					label="ScanMyTesla / ELM327"
					value="Compatibility modes are read from board state"
				/>
			</Section>
		</View>
	);
}
