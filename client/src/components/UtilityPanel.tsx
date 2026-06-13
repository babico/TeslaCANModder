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
			<Text className="text-sm text-muted-foreground flex-1">{label}</Text>
			<Text
				className={`text-sm font-semibold text-right flex-1 ${dimmed ? "text-muted-foreground/50" : "text-card-foreground"}`}
			>
				{value}
			</Text>
		</View>
	);
}

export interface UtilityPanelProps {
	state: BoardState;
}

export function UtilityPanel({ state }: UtilityPanelProps) {
	return (
		<View className="gap-3">
			<Section
				title="Utility Feature Status"
				description="Operational state for helper features and non-driving utility toggles."
			>
				<Row
					label="Seatbelt Emulation"
					value={state.seatbeltEmulation ? "Supported" : "Unavailable"}
					dimmed={!state.seatbeltEmulation}
				/>
				<Row
					label="Speed Alert BLE"
					value={state.speedAlert ? "Enabled" : "Disabled"}
					dimmed={!state.speedAlert}
				/>
				<Row
					label="CAN Simulation"
					value={state.canSim ? "Enabled" : "Disabled"}
					dimmed={!state.canSim}
				/>
				<Row
					label="Single-shot TX"
					value={state.singleShot ? "Enabled" : "Disabled"}
					dimmed={!state.singleShot}
				/>
				<Row
					label="Wiper Persistence"
					value={state.wiperPersist ? "Enabled" : "Disabled"}
					dimmed={!state.wiperPersist}
				/>
				<Row
					label="Mirror Auto-fold"
					value={state.mirrorAutoFold ? "Enabled" : "Disabled"}
					dimmed={!state.mirrorAutoFold}
				/>
			</Section>

			<Section
				title="Button Remapping"
				description="Current mappings for lamp and parking button short/long/double actions."
			>
				<Row
					label="Support"
					value={state.hasBtnMap ? "Available" : "Unavailable"}
					dimmed={!state.hasBtnMap}
				/>
				{state.hasBtnMap ? (
					<>
						<Row label="Lamp Short" value={state.btnMapLampShort} />
						<Row label="Lamp Long" value={state.btnMapLampLong} />
						<Row label="Lamp Double" value={state.btnMapLampDouble} />
						<Row label="Park Short" value={state.btnMapParkShort} />
						<Row label="Park Long" value={state.btnMapParkLong} />
						<Row label="Park Double" value={state.btnMapParkDouble} />
					</>
				) : null}
			</Section>

			<Section
				title="AP Injection Gate"
				description="Safety gate for write/injection paths; opens only under allowed runtime conditions."
			>
				<Row
					label="Gate Enabled"
					value={state.apGateEnabled ? "Enabled" : "Disabled"}
					dimmed={!state.apGateEnabled}
				/>
				<Row
					label="Gate Open"
					value={state.apGateOpen ? "Open" : "Closed"}
					dimmed={!state.apGateOpen}
				/>
				<Row label="Gate Reason" value={state.apGateReason || "unknown"} />
			</Section>

			<Section
				title="Advanced Utility Control Notes"
				description="Recommended command surfaces for quick actions versus parameterized control calls."
			>
				<Row
					label="High-Risk Warning"
					value="Live vehicle action commands should be used only when parked or in controlled test conditions"
				/>
				<Row
					label="Quick Actions"
					value="canSimStart/canSimStop, btnMapQuery, btnMapReset"
				/>
				<Row
					label="Parameterized"
					value="singleShot, seatbelt, speedAlert, wiperPersist, mirrorAutoFold, btnMap"
				/>
				<Row
					label="Safe Flow"
					value="Use Controls palette or Monitor command runner for parameterized commands"
				/>
			</Section>
		</View>
	);
}
