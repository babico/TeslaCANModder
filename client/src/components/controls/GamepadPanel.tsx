import { Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Badge } from "../../ui/shadcn/badge";
import { Button } from "../../ui/shadcn/button";

export interface GamepadPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

interface GamepadDevice {
	addr?: string;
	name?: string;
}

export function GamepadPanel({ boardState, onCommand }: GamepadPanelProps) {
	const raw = boardState as unknown as Record<string, unknown>;
	const gp = (raw.gamepad as Record<string, unknown> | undefined) ?? undefined;
	const enabled = Boolean(gp?.enabled);
	const bleDistanceMode = String(raw.bleDistanceMode ?? "formula");
	const bleDistanceMeters = Number(raw.bleDistanceMeters ?? -1);
	const bleRssi = Number(raw.bleRssi ?? 0);
	const bleDistanceFactor = Number(raw.bleDistanceFactor ?? 2.0);
	const bleDistanceCalibrated = Boolean(raw.bleDistanceCalibrated);
	const connected = Boolean(gp?.connected);
	const scanning = Boolean(gp?.scanning);
	const pairedAddr = typeof gp?.pairedAddr === "string" ? (gp.pairedAddr as string) : "";
	const pairedName = typeof gp?.pairedName === "string" ? (gp.pairedName as string) : "";
	const rssi = Number(gp?.rssi);
	const battery = Number(gp?.battery);
	const devices: GamepadDevice[] = Array.isArray(gp?.devices)
		? (gp!.devices as GamepadDevice[])
		: [];

	const status = !enabled
		? "OFF"
		: !connected
			? scanning
				? "SCANNING"
				: pairedAddr
					? "DISCONNECTED"
					: "UNPAIRED"
			: "CONNECTED";

	const statusVariant =
		status === "CONNECTED" || status === "SCANNING" ? "default" : ("outline" as const);

	return (
		<Card>
			<CardHeader>
				<View className="flex-row items-center justify-between">
					<CardTitle>Gamepad (BLE HID)</CardTitle>
					<Badge label={status} variant={statusVariant} />
				</View>
			</CardHeader>
			<CardContent className="gap-3">
				<Text className="text-xs text-muted-foreground leading-4">
					Pair a Bluetooth controller to drive DAS sticks and trigger bound commands. Use
					Console for gamepad:bind:&lt;n&gt;:&lt;cmd&gt; and per-axis tuning.
				</Text>

				{pairedAddr ? (
					<View className="flex-row flex-wrap gap-2">
						<View className="min-w-[130px] flex-1 gap-0.5">
							<Text className="text-xs text-muted-foreground">Paired</Text>
							<Text className="text-sm font-semibold text-foreground">
								{pairedName || pairedAddr}
							</Text>
						</View>
						{Number.isFinite(battery) && battery >= 0 && battery <= 100 ? (
							<View className="min-w-[130px] flex-1 gap-0.5">
								<Text className="text-xs text-muted-foreground">Battery</Text>
								<Text className="text-sm font-semibold text-foreground">
									{battery}%
								</Text>
							</View>
						) : null}
						{Number.isFinite(rssi) && rssi !== 0 ? (
							<View className="min-w-[130px] flex-1 gap-0.5">
								<Text className="text-xs text-muted-foreground">RSSI</Text>
								<Text className="text-sm font-semibold text-foreground">
									{rssi} dBm
								</Text>
							</View>
						) : null}
					</View>
				) : null}

				<View className="flex-row flex-wrap gap-2">
					<Button
						label="Enable"
						size="sm"
						variant={enabled ? "default" : "outline"}
						onPress={() => onCommand("gamepad", "true")}
					/>
					<Button
						label="Disable"
						size="sm"
						variant={!enabled ? "default" : "outline"}
						onPress={() => onCommand("gamepad", "false")}
					/>
					<Button
						label={scanning ? "Scanning..." : "Scan"}
						size="sm"
						variant="outline"
						onPress={() => onCommand("gamepadScan")}
					/>
					<Button
						label="Cancel Burst"
						size="sm"
						variant="destructive"
						onPress={() => onCommand("gamepadCancel")}
					/>
					{pairedAddr ? (
						<Button
							label="Unpair"
							size="sm"
							variant="outline"
							onPress={() => onCommand("gamepadUnpair")}
						/>
					) : null}
				</View>

				<View className="gap-2 rounded-md bg-muted p-3">
					<View className="flex-row items-center justify-between">
						<Text className="text-xs text-muted-foreground">BLE key distance</Text>
						<Badge
							label={bleDistanceMode}
							variant={bleDistanceMode === "off" ? "outline" : "default"}
						/>
					</View>
					<View className="flex-row flex-wrap gap-2">
						<View className="min-w-[100px] flex-1 gap-0.5">
							<Text className="text-xs text-muted-foreground">RSSI</Text>
							<Text className="text-sm font-semibold text-foreground">
								{bleRssi !== 0 ? `${bleRssi} dBm` : "—"}
							</Text>
						</View>
						<View className="min-w-[100px] flex-1 gap-0.5">
							<Text className="text-xs text-muted-foreground">Distance</Text>
							<Text className="text-sm font-semibold text-foreground">
								{bleDistanceMeters < 0 ? "—" : `${bleDistanceMeters.toFixed(1)} m`}
								{bleDistanceCalibrated ? " *" : ""}
							</Text>
						</View>
						<View className="min-w-[100px] flex-1 gap-0.5">
							<Text className="text-xs text-muted-foreground">Factor</Text>
							<Text className="text-sm font-semibold text-foreground">
								{bleDistanceFactor.toFixed(2)}
							</Text>
						</View>
					</View>
					<View className="flex-row flex-wrap gap-2">
						<Button
							label="Off"
							size="sm"
							variant={bleDistanceMode === "off" ? "default" : "outline"}
							onPress={() => onCommand("bleDistanceMode", "off")}
						/>
						<Button
							label="Threshold"
							size="sm"
							variant={bleDistanceMode === "threshold" ? "default" : "outline"}
							onPress={() => onCommand("bleDistanceMode", "threshold")}
						/>
						<Button
							label="Formula"
							size="sm"
							variant={bleDistanceMode === "formula" ? "default" : "outline"}
							onPress={() => onCommand("bleDistanceMode", "formula")}
						/>
						<Button
							label="Kalman"
							size="sm"
							variant={bleDistanceMode === "kalman" ? "default" : "outline"}
							onPress={() => onCommand("bleDistanceMode", "kalman")}
						/>
					</View>
				</View>

				{devices.length > 0 ? (
					<View className="gap-1">
						<Text className="text-xs text-muted-foreground">
							Discovered ({devices.length})
						</Text>
						{devices.slice(0, 8).map((d, i) => {
							const addr = d.addr ?? "";
							const name = d.name ?? "(unnamed)";
							return (
								<View
									key={`${addr}-${i}`}
									className="flex-row items-center justify-between gap-2 py-1 px-3 rounded-md bg-muted"
								>
									<View className="flex-1 gap-0.5">
										<Text className="text-sm font-semibold text-foreground">
											{name}
										</Text>
										<Text className="text-xs text-muted-foreground">
											{addr}
										</Text>
									</View>
									<Button
										label="Pair"
										size="sm"
										variant="outline"
										onPress={() => {
											if (!addr) return;
											onCommand("gamepadPair", addr);
										}}
									/>
								</View>
							);
						})}
					</View>
				) : null}
			</CardContent>
		</Card>
	);
}
