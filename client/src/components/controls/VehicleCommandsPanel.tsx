import { Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import type { CommandName } from "../../hardware/controller";
import { Card, CardHeader, CardTitle, CardContent } from "../../ui/shadcn/card";
import { Button } from "../../ui/shadcn/button";

export interface VehicleCommandsPanelProps {
	boardState: BoardState;
	onCommand: (name: CommandName, args?: string) => void;
}

interface QuickAction {
	label: string;
	cmd: CommandName;
	args?: string;
	variant?: "default" | "destructive" | "outline";
	active?: boolean;
	activeLabel?: string;
}

function QuickSection({
	title,
	commands,
	onCommand,
}: {
	title: string;
	commands: QuickAction[];
	onCommand: (name: CommandName, args?: string) => void;
}) {
	return (
		<View className="gap-2">
			<Text className="text-xs font-semibold uppercase text-muted-foreground">{title}</Text>
			<View className="flex-row flex-wrap gap-1">
				{commands.map((cmd) => (
					<Button
						key={cmd.cmd + (cmd.args ?? "")}
						label={
							cmd.activeLabel && cmd.active
								? `${cmd.label}: ${cmd.activeLabel}`
								: cmd.label
						}
						size="sm"
						variant={cmd.variant ?? (cmd.active ? "default" : "outline")}
						onPress={() => onCommand(cmd.cmd, cmd.args)}
					/>
				))}
			</View>
		</View>
	);
}

export function VehicleCommandsPanel({ boardState, onCommand }: VehicleCommandsPanelProps) {
	const lockCommands: QuickAction[] = [
		{ label: "Lock", cmd: "lock" },
		{ label: "Unlock", cmd: "unlock" },
		{ label: "Horn", cmd: "horn" },
	];

	const trunkCommands: QuickAction[] = [
		{ label: "Frunk", cmd: "frunkOpen" },
		{ label: "Trunk", cmd: "trunkOpen" },
		{ label: "Glovebox", cmd: "glovebox" },
	];

	const summonCommands: QuickAction[] = [
		{
			label: "Summon",
			active: boardState.summonActive,
			activeLabel: boardState.summonActive ? "Active" : "Idle",
			cmd: "summon",
		},
		{ label: "Fwd", cmd: "summonForward" },
		{ label: "Rev", cmd: "summonReverse" },
		{ label: "Stop", cmd: "summonStop", variant: "destructive" },
	];

	const driveCommands: QuickAction[] = [
		{
			label: "Track Mode",
			cmd: boardState.trackMode ? "trackMode" : "trackMode",
			args: boardState.trackMode ? "false" : "true",
			active: boardState.trackMode,
			activeLabel: boardState.trackMode ? "ON" : "OFF",
		},
		{
			label: "Precondition",
			cmd: boardState.precondition ? "precondition" : "precondition",
			args: boardState.precondition ? "false" : "true",
			active: boardState.precondition,
			activeLabel: boardState.precondition ? "ON" : "OFF",
		},
		{ label: "Charge Start", cmd: "chargeStart" },
		{ label: "Charge Stop", cmd: "chargeStop" },
		{ label: "Charge Port", cmd: "chargePort" },
	];

	const mirrorCommands: QuickAction[] = [
		{ label: "Fold", cmd: "mirrorFold" },
		{ label: "Unfold", cmd: "mirrorUnfold" },
		{ label: "Heat", cmd: "mirrorHeat" },
		{ label: "Auto Fold", cmd: "mirrorAutoFold" },
		{ label: "Dip", cmd: "mirrorDip" },
	];

	const windowCommands: QuickAction[] = [
		{ label: "Vent Open", cmd: "windowVentOpen" },
		{ label: "Vent Close", cmd: "windowVentClose" },
		{ label: "All Open", cmd: "ventOpen" },
		{ label: "All Close", cmd: "ventClose" },
	];

	const climateCommands: QuickAction[] = [
		{ label: "Keep", cmd: "climateKeep" },
		{ label: "Off", cmd: "climateOff" },
	];

	const lightCommands: QuickAction[] = [
		{ label: "Fog Front", cmd: "lightFogFront" },
		{ label: "Fog Rear", cmd: "lightFogRear" },
		{ label: "Ambient", cmd: "lightAmbient" },
		{ label: "Home", cmd: "lightHome" },
		{ label: "Dome On", cmd: "lightDomeOn" },
		{ label: "Dome Off", cmd: "lightDomeOff" },
	];

	const wiperCommands: QuickAction[] = [
		{ label: "Off", cmd: "wiperOff" },
		{ label: "1", cmd: "wiper1" },
		{ label: "2", cmd: "wiper2" },
		{ label: "3", cmd: "wiper3" },
	];

	const rawState = boardState as unknown as Record<string, unknown>;
	const sentryActive = Boolean(rawState.sentry);

	const seatCommands: QuickAction[] = [
		{ label: "FL 0", cmd: "seatFL", args: "0" },
		{ label: "FL 1", cmd: "seatFL", args: "1" },
		{ label: "FL 2", cmd: "seatFL", args: "2" },
		{ label: "FL 3", cmd: "seatFL", args: "3" },
	];

	const sentryCommands: QuickAction[] = [
		{
			label: "Sentry",
			cmd: sentryActive ? "sentryOff" : "sentryOn",
			active: sentryActive,
			activeLabel: sentryActive ? "ON" : "OFF",
		},
	];

	const turnCommands: QuickAction[] = [
		{ label: "Left 3", cmd: "turnLeft3" },
		{ label: "Right 3", cmd: "turnRight3" },
		{ label: "Hazard", cmd: "turnHazard" },
		{ label: "Off", cmd: "turnOff" },
	];

	const pedalCommands: QuickAction[] = [
		{ label: "Standard", cmd: "pedalStandard" },
		{ label: "Chill", cmd: "pedalChill" },
		{ label: "Sport", cmd: "pedalSport" },
	];
	const regenCommands: QuickAction[] = [
		{ label: "Off", cmd: "regenOff" },
		{ label: "Low", cmd: "regenLow" },
		{ label: "Std", cmd: "regenStd" },
		{ label: "Max", cmd: "regenMax" },
	];
	const stopCommands: QuickAction[] = [
		{ label: "Creep", cmd: "stopCreep" },
		{ label: "Roll", cmd: "stopRoll" },
		{ label: "Hold", cmd: "stopHold" },
	];

	const powerCommands: QuickAction[] = [
		{ label: "ACC On", cmd: "powerAccOn" },
		{ label: "ACC Off", cmd: "powerAccOff", variant: "destructive" },
		{ label: "Ready", cmd: "powerReady" },
		{ label: "Off", cmd: "powerOff", variant: "destructive" },
	];

	return (
		<Card>
			<CardHeader>
				<CardTitle>Vehicle Commands</CardTitle>
			</CardHeader>
			<CardContent className="gap-3">
				<QuickSection
					title="Lock & Security"
					commands={lockCommands}
					onCommand={onCommand}
				/>
				<QuickSection
					title="Trunk & Frunk"
					commands={trunkCommands}
					onCommand={onCommand}
				/>
				<QuickSection title="Summon" commands={summonCommands} onCommand={onCommand} />
				<QuickSection
					title="Drive Features"
					commands={driveCommands}
					onCommand={onCommand}
				/>
				<QuickSection title="Mirrors" commands={mirrorCommands} onCommand={onCommand} />
				<QuickSection title="Windows" commands={windowCommands} onCommand={onCommand} />
				<QuickSection title="Climate" commands={climateCommands} onCommand={onCommand} />
				<QuickSection title="Lights" commands={lightCommands} onCommand={onCommand} />
				<QuickSection title="Sentry" commands={sentryCommands} onCommand={onCommand} />
				<QuickSection title="Wipers" commands={wiperCommands} onCommand={onCommand} />
				<QuickSection title="Turn Signals" commands={turnCommands} onCommand={onCommand} />
				<QuickSection title="Pedal" commands={pedalCommands} onCommand={onCommand} />
				<QuickSection title="Regen" commands={regenCommands} onCommand={onCommand} />
				<QuickSection title="Stop Mode" commands={stopCommands} onCommand={onCommand} />
				<QuickSection title="Seat Heat FL" commands={seatCommands} onCommand={onCommand} />
				<QuickSection title="Power" commands={powerCommands} onCommand={onCommand} />
			</CardContent>
		</Card>
	);
}
