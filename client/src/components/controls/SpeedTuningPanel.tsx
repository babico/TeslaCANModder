import { Pressable, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Button } from "../../ui/shadcn/button";

export interface SpeedTuningPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

const PROFILES = [0, 1, 2, 3] as const;
const PROFILE_LABELS: Record<number, string> = {
	0: "Aggressive",
	1: "Moderate",
	2: "Conservative",
	3: "Most Conservative",
};

const OFFSET_STEPS = [0, 5, 10, 15, 20, 30, 40, 50, 63, 80, 100];

export function SpeedTuningPanel({ boardState, onCommand }: SpeedTuningPanelProps) {
	const profileValue = typeof boardState.profile === "number" ? boardState.profile : 0;
	const offsetValue = typeof boardState.offset === "number" ? boardState.offset : 0;

	return (
		<Card>
			<CardHeader>
				<CardTitle>Speed Profile & Offset</CardTitle>
			</CardHeader>
			<CardContent className="gap-3">
				<View>
					<View className="flex-row items-center justify-between mb-1">
						<Text className="text-xs font-semibold uppercase text-muted-foreground">
							Profile
						</Text>
						<Text className="text-xs text-muted-foreground">
							{profileValue} · {PROFILE_LABELS[profileValue] ?? ""}{" "}
							{boardState.profilePinned ? "(pinned)" : "(auto)"}
						</Text>
					</View>
					<View className="flex-row flex-wrap gap-1">
						{PROFILES.map((p) => (
							<Pressable
								key={p}
								onPress={() => onCommand("profile", String(p))}
								className={`px-3 py-1.5 rounded-full border ${
									p === profileValue
										? "bg-primary border-primary"
										: "bg-muted border-border"
								}`}
							>
								<Text
									className={`text-xs font-semibold ${
										p === profileValue
											? "text-primary-foreground"
											: "text-foreground"
									}`}
								>
									{p}
								</Text>
							</Pressable>
						))}
						<Button
							label="Auto"
							size="sm"
							variant={!boardState.profilePinned ? "default" : "outline"}
							onPress={() => onCommand("profileAuto")}
						/>
					</View>
				</View>

				<View>
					<View className="flex-row items-center justify-between mb-1">
						<Text className="text-xs font-semibold uppercase text-muted-foreground">
							Offset
						</Text>
						<Text className="text-xs text-muted-foreground">
							{offsetValue}% {boardState.offsetPinned ? "(pinned)" : "(auto)"}
						</Text>
					</View>
					<View className="flex-row flex-wrap gap-1">
						{OFFSET_STEPS.map((o) => (
							<Pressable
								key={o}
								onPress={() => onCommand("offset", String(o))}
								className={`px-2 py-1 rounded-full border ${
									o === offsetValue
										? "bg-primary border-primary"
										: "bg-muted border-border"
								}`}
							>
								<Text
									className={`text-xs font-semibold ${
										o === offsetValue
											? "text-primary-foreground"
											: "text-foreground"
									}`}
								>
									{o}%
								</Text>
							</Pressable>
						))}
						<Button
							label="Auto"
							size="sm"
							variant={!boardState.offsetPinned ? "default" : "outline"}
							onPress={() => onCommand("offsetAuto")}
						/>
					</View>
				</View>
			</CardContent>
		</Card>
	);
}
