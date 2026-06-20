import { Pressable, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Badge } from "../../ui/shadcn/badge";
import { Button } from "../../ui/shadcn/button";

export interface FsdPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

const NAG_MODES = [
	{ label: "Off", value: "off" },
	{ label: "Bit19", value: "bit19" },
	{ label: "Legacy", value: "legacy" },
	{ label: "Safe", value: "safe" },
	{ label: "Natural", value: "natural" },
	{ label: "Organic", value: "organic" },
	{ label: "Full", value: "full" },
	{ label: "Feifan", value: "feifan" },
];

export function FsdPanel({ boardState, onCommand }: FsdPanelProps) {
	const raw = boardState as unknown as Record<string, unknown>;
	const nagModeRaw = typeof raw.nagMode === "string" ? (raw.nagMode as string) : "";
	const alcVal = typeof raw.alc === "number" ? (raw.alc as number) : 0;

	return (
		<Card>
			<CardHeader>
				<View className="flex-row items-center justify-between">
					<CardTitle>FSD & Autopilot</CardTitle>
					<Badge
						label={boardState.fsd ? "ON" : "OFF"}
						variant={boardState.fsd ? "default" : "outline"}
					/>
				</View>
			</CardHeader>
			<CardContent className="gap-3">
				<View className="flex-row flex-wrap gap-2">
					<Button
						label="Enable FSD"
						size="sm"
						variant={boardState.fsd ? "default" : "outline"}
						onPress={() => onCommand("fsd", "true")}
					/>
					<Button
						label="Disable FSD"
						size="sm"
						variant={!boardState.fsd ? "default" : "outline"}
						onPress={() => onCommand("fsd", "false")}
					/>
					<Button
						label={boardState.fsdForce ? "Force: ON" : "Force: OFF"}
						size="sm"
						variant={boardState.fsdForce ? "destructive" : "outline"}
						onPress={() =>
							onCommand("fsdForce", boardState.fsdForce ? "false" : "true")
						}
					/>
				</View>

				<Text className="text-xs font-semibold uppercase text-muted-foreground">
					Nag Mode
				</Text>
				<View className="flex-row flex-wrap gap-1">
					{NAG_MODES.map((m) => {
						const active =
							(nagModeRaw && nagModeRaw === m.value) ||
							(m.value === "bit19" && boardState.nag && !nagModeRaw) ||
							(m.value === "off" && !boardState.nag && !nagModeRaw);
						return (
							<Pressable
								key={m.value}
								onPress={() => onCommand("nagMode", m.value)}
								className={`px-2 py-1 rounded-full border ${
									active ? "bg-primary border-primary" : "bg-muted border-border"
								}`}
							>
								<Text
									className={`text-xs font-semibold ${
										active ? "text-primary-foreground" : "text-foreground"
									}`}
								>
									{m.label}
								</Text>
							</Pressable>
						);
					})}
				</View>

				<View className="flex-row flex-wrap gap-2">
					<Button
						label={boardState.eceR79 ? "ECE R79: ON" : "ECE R79: OFF"}
						size="sm"
						variant={boardState.eceR79 ? "default" : "outline"}
						onPress={() => onCommand("eceR79", boardState.eceR79 ? "false" : "true")}
					/>
					<Button
						label={boardState.isaChime ? "ISA: OFF" : "ISA: ON"}
						size="sm"
						variant={boardState.isaChime ? "default" : "outline"}
						onPress={() =>
							onCommand("isaChime", boardState.isaChime ? "false" : "true")
						}
					/>
					{alcVal > 0 || typeof raw.alc === "number" ? (
						<Button
							label={alcVal ? "ALC: ON" : "ALC: OFF"}
							size="sm"
							variant={alcVal ? "default" : "outline"}
							onPress={() => onCommand("alc", alcVal ? "false" : "true")}
						/>
					) : null}
				</View>

				<View className="flex-row flex-wrap gap-2">
					<Button
						label="Region: NA"
						size="sm"
						variant="outline"
						onPress={() => onCommand("regionSpoof", "na")}
					/>
					<Button
						label="Region: EU"
						size="sm"
						variant="outline"
						onPress={() => onCommand("regionSpoof", "eu")}
					/>
					<Button
						label="Region: Off"
						size="sm"
						variant="outline"
						onPress={() => onCommand("regionSpoof", "off")}
					/>
				</View>
			</CardContent>
		</Card>
	);
}
