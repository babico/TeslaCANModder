import { Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";

function pressureBar(raw: number): string {
	return (raw / 100).toFixed(2);
}

const HV_STATE_LABELS: Record<number, string> = {
	0: "Idle",
	1: "Pre-charge",
	2: "Active",
	3: "Discharge",
	4: "Fault",
	5: "Sleep",
};
function hvStateLabel(val: number): string {
	return HV_STATE_LABELS[val] ?? `State ${val}`;
}

const CONTACTOR_LABELS: Record<number, string> = {
	0: "Open",
	1: "Closed",
	2: "Pre-charge",
	3: "Fault",
};
function contactorLabel(val: number): string {
	return CONTACTOR_LABELS[val] ?? `State ${val}`;
}

function formatBusLabel(value: string): string {
	if (!value) return value;
	return `${value.charAt(0).toUpperCase()}${value.slice(1)}`;
}

// ── Sub-components ─────────────────────────────────────────────────────────

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
			<Text className="text-xs font-bold uppercase tracking-wider text-muted-foreground mb-0.5">
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
				className={`text-sm font-semibold ${dimmed ? "text-muted-foreground/50" : "text-card-foreground"}`}
			>
				{value}
			</Text>
		</View>
	);
}

// ── TelemetryPanel ────────────────────────────────────────────────────────

export interface TelemetryPanelProps {
	state: BoardState;
}

export function TelemetryPanel({ state }: TelemetryPanelProps) {
	const canEntries = Object.entries(state.canHealth);

	return (
		<View className="gap-3">
			{state.hasBms ? (
				<Section
					title="BMS Extended"
					description="Battery pack thermal, contactor, and power-limit telemetry."
				>
					<Row
						label="Temp Range"
						value={`${state.bmsTempMin.toFixed(0)}–${state.bmsTempMax.toFixed(0)} °C`}
					/>
					<Row label="HV State" value={hvStateLabel(state.bmsHvState)} />
					<Row label="Contactor" value={contactorLabel(state.bmsContactorState)} />
					<Row label="Max Regen" value={`${state.bmsMaxRegenPower.toFixed(0)} kW`} />
					<Row
						label="Max Discharge"
						value={`${state.bmsMaxDischargePower.toFixed(0)} kW`}
					/>
					<Row
						label="Bus Voltage"
						value={`${state.bmsMinBusVoltage.toFixed(0)}–${state.bmsMaxBusVoltage.toFixed(0)} V`}
					/>
				</Section>
			) : null}

			{state.hasTpms ? (
				<Section
					title="TPMS"
					description="Per-wheel pressure and temperature diagnostics from live CAN decode."
				>
					<View className="flex-row gap-1">
						{(["FL", "FR", "RL", "RR"] as const).map((pos) => {
							const pressure = pressureBar(
								(state as unknown as Record<string, number>)[
									`tpmsPressure${pos}`
								] as number,
							);
							const temp = (
								(state as unknown as Record<string, number>)[
									`tpmsTemp${pos}`
								] as number
							).toFixed(0);
							const low = parseFloat(pressure) < 1.8;
							return (
								<View
									key={pos}
									className="flex-1 items-center flex-row justify-between p-1 bg-muted rounded-md border border-border gap-0.5"
								>
									<Text className="text-xs font-bold text-muted-foreground w-6">
										{pos}
									</Text>
									<Text
										className={`text-sm font-semibold ${low ? "text-destructive" : "text-card-foreground"}`}
									>
										{pressure} bar
									</Text>
									<Text className="text-xs text-muted-foreground">{temp} °C</Text>
								</View>
							);
						})}
					</View>
				</Section>
			) : null}

			{state.hasPowertrain ? (
				<Section
					title="Powertrain"
					description="Drive-state output and battery power response while moving."
				>
					<Row label="Drive Mode" value={String(state.driveMode)} />
					<Row label="Power" value={`${state.bmsPower.toFixed(1)} kW`} />
				</Section>
			) : null}

			<Section
				title="Firmware"
				description="Compatibility, steering-mode decode, and ingest-rate heartbeat."
			>
				<Row label="FW Compat" value={String(state.fwCompat)} />
				<Row label="Uptime" value={`${state.uptime} s`} />
				<Row label="Rate" value={`${state.rate} msg/s`} />
			</Section>

			{canEntries.length > 0 ? (
				<Section
					title="CAN Health"
					description="Per-bus online and controller-detection status for Chassis, Vehicle, and Body."
				>
					{canEntries.map(([bus, health]) => (
						<Row
							key={bus}
							label={formatBusLabel(bus)}
							value={
								health.on
									? health.det
										? "On · Detected"
										: "On · Not detected"
									: "Offline"
							}
							dimmed={!health.on}
						/>
					))}
				</Section>
			) : null}
		</View>
	);
}
