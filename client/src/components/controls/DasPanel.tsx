import { useState } from "react";
import { Text, TextInput, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Badge } from "../../ui/shadcn/badge";
import { Button } from "../../ui/shadcn/button";

export interface DasPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

export function DasPanel({ boardState, onCommand }: DasPanelProps) {
	const raw = boardState as unknown as Record<string, unknown>;
	const enabled = Boolean(raw.dasDriveEnabled);
	const liveLimit = Number(raw.dasSpeedLimitKph) || 0;
	const liveCap = Number(raw.dasSpeedCapKph) || 0;
	const capMax = Number(raw.dasSpeedCapMaxKph) || 200;

	const [limitInput, setLimitInput] = useState(String(liveLimit || 25));
	const [capInput, setCapInput] = useState(String(liveCap || 25));

	const submitLimit = () => {
		const n = Math.max(1, Math.floor(Number(limitInput)));
		if (!Number.isFinite(n)) return;
		onCommand("driveSpeed", String(n));
	};
	const submitCap = () => {
		const n = Math.max(1, Math.min(capMax || 200, Math.floor(Number(capInput))));
		if (!Number.isFinite(n)) return;
		onCommand("driveCap", String(n));
	};

	return (
		<Card>
			<CardHeader>
				<View className="flex-row items-center justify-between">
					<CardTitle>DAS Drive (Gamepad CAN)</CardTitle>
					<Badge
						label={enabled ? "ARMED" : "OFF"}
						variant={enabled ? "default" : "outline"}
					/>
				</View>
			</CardHeader>
			<CardContent className="gap-3">
				<Text className="text-xs text-muted-foreground leading-4">
					Manual remote-control. Gamepad sticks become steering and pedals — not
					Autopilot. Hard cap {capMax} km/h. Use only on a bench or private property.
				</Text>

				<View className="flex-row gap-2">
					<Button
						label="Enable"
						size="sm"
						variant={enabled ? "default" : "outline"}
						onPress={() => onCommand("drive", "true")}
					/>
					<Button
						label="Disable"
						size="sm"
						variant={!enabled ? "default" : "outline"}
						onPress={() => onCommand("drive", "false")}
					/>
				</View>

				<View className="flex-row items-end gap-2">
					<View className="flex-1 gap-1">
						<Text className="text-xs text-muted-foreground">User Limit (km/h)</Text>
						<TextInput
							className="min-h-[38px] rounded-md border border-border bg-muted px-3 text-sm text-foreground"
							keyboardType="numeric"
							value={limitInput}
							onChangeText={setLimitInput}
							onSubmitEditing={submitLimit}
							placeholder={String(liveLimit || 25)}
							placeholderTextColor="#5c7ea0"
						/>
						<Text className="text-xs text-muted-foreground">
							live: {liveLimit} km/h
						</Text>
					</View>
					<Button label="Apply" size="sm" onPress={submitLimit} />
				</View>

				<View className="flex-row items-end gap-2">
					<View className="flex-1 gap-1">
						<Text className="text-xs text-muted-foreground">
							Hard Cap (km/h, max {capMax})
						</Text>
						<TextInput
							className="min-h-[38px] rounded-md border border-border bg-muted px-3 text-sm text-foreground"
							keyboardType="numeric"
							value={capInput}
							onChangeText={setCapInput}
							onSubmitEditing={submitCap}
							placeholder={String(liveCap || 25)}
							placeholderTextColor="#5c7ea0"
						/>
						<Text className="text-xs text-muted-foreground">live: {liveCap} km/h</Text>
					</View>
					<Button label="Set Cap" size="sm" variant="destructive" onPress={submitCap} />
				</View>
			</CardContent>
		</Card>
	);
}
