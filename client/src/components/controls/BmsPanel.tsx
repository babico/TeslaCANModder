import { Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Badge } from "../../ui/shadcn/badge";
import { Button } from "../../ui/shadcn/button";

export interface BmsPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

function StatCell({ label, value }: { label: string; value: string }) {
	return (
		<View className="min-w-[120px] flex-1 bg-muted rounded-md border border-border p-2 gap-0.5">
			<Text className="text-xs text-muted-foreground">{label}</Text>
			<Text className="text-sm font-semibold text-foreground">{value}</Text>
		</View>
	);
}

export function BmsPanel({ boardState, onCommand }: BmsPanelProps) {
	if (!boardState.hasBms) {
		return (
			<Card>
				<CardHeader>
					<View className="flex-row items-center justify-between">
						<CardTitle>Battery (BMS)</CardTitle>
						<Badge label="No Data" variant="outline" />
					</View>
				</CardHeader>
				<CardContent>
					<Button
						label="Query BMS"
						size="sm"
						variant="outline"
						onPress={() => onCommand("bms")}
					/>
				</CardContent>
			</Card>
		);
	}

	const voltage =
		typeof boardState.bmsVoltage === "number" ? boardState.bmsVoltage.toFixed(1) : "--";
	const current =
		typeof boardState.bmsCurrent === "number" ? boardState.bmsCurrent.toFixed(1) : "--";
	const power = typeof boardState.bmsPower === "number" ? boardState.bmsPower.toFixed(1) : "--";
	const soc = typeof boardState.bmsSoc === "number" ? boardState.bmsSoc.toFixed(1) : "--";
	const tempMin =
		typeof boardState.bmsTempMin === "number" ? boardState.bmsTempMin.toFixed(0) : "--";
	const tempMax =
		typeof boardState.bmsTempMax === "number" ? boardState.bmsTempMax.toFixed(0) : "--";

	return (
		<Card>
			<CardHeader>
				<View className="flex-row items-center justify-between">
					<CardTitle>Battery (BMS)</CardTitle>
					<Badge label="Live" variant="default" />
				</View>
			</CardHeader>
			<CardContent className="gap-2">
				<View className="flex-row flex-wrap gap-2">
					<StatCell label="Voltage" value={`${voltage}V`} />
					<StatCell label="Current" value={`${current}A`} />
					<StatCell label="Power" value={`${power}kW`} />
					<StatCell label="SoC" value={`${soc}%`} />
					<StatCell label="Temp Min" value={`${tempMin}°C`} />
					<StatCell label="Temp Max" value={`${tempMax}°C`} />
				</View>
				<Button
					label="Query BMS"
					size="sm"
					variant="outline"
					onPress={() => onCommand("bms")}
				/>
			</CardContent>
		</Card>
	);
}
