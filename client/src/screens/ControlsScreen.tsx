import React, { useMemo, useState } from "react";
import { ScrollView, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import {
	FEATURE_IDS,
	FEATURE_SETTINGS_BY_ID,
	type FeatureId,
	type FeatureSpecKind,
} from "../../../packages/protocol/src/featureSettings";
import { getCommandGate } from "../state/commandGating";
import { ALL_COMMANDS, type CommandName } from "../hardware/controller";
import { useBoardInstanceState } from "../state/BoardStateContext";
import { useCommandActions } from "../state/CommandContext";
import {
	BusStatusBar,
	CommandGroupCard,
	CommandPaletteModal,
} from "../components/controls/ControlsBlocks";
import { FsdPanel } from "../components/controls/FsdPanel";
import { BmsPanel } from "../components/controls/BmsPanel";
import { DasPanel } from "../components/controls/DasPanel";
import { GamepadPanel } from "../components/controls/GamepadPanel";
import { SpeedTuningPanel } from "../components/controls/SpeedTuningPanel";
import { VehicleCommandsPanel } from "../components/controls/VehicleCommandsPanel";

interface CommandItem {
	name: CommandName;
	label: string;
	requiresArgs?: boolean;
}

type BusField = "chassisOnline" | "vehicleOnline" | "bodyOnline";

interface CommandGroup {
	title: string;
	kind: FeatureSpecKind;
	busLabel: string;
	busColor: string;
	busField?: BusField;
	commands: CommandItem[];
}

const ACTIVE_PRESET = "performance" as const;

function humanizeCommandName(name: CommandName): string {
	return name.replace(/([a-z])([A-Z])/g, "$1 $2").replace(/\b\w/g, (m) => m.toUpperCase());
}

function buildFeatureCommandItems(featureId: FeatureId): CommandItem[] {
	const spec = FEATURE_SETTINGS_BY_ID[featureId];
	const deduped = new Map<CommandName, CommandItem>();

	for (const setting of spec.settings) {
		setting.commandNames.forEach((commandName, index) => {
			const descriptor = ALL_COMMANDS.find((entry) => entry.name === commandName);
			if (!descriptor || deduped.has(commandName)) {
				return;
			}

			const optionLabel =
				setting.control === "enum" ? setting.options[index]?.label : undefined;
			const label = optionLabel
				? `${setting.label}: ${optionLabel}`
				: setting.commandNames.length > 1
					? `${setting.label}: ${humanizeCommandName(commandName)}`
					: setting.label;

			deduped.set(commandName, {
				name: commandName,
				label,
				requiresArgs: descriptor.argCount > 0,
			});
		});
	}

	return [...deduped.values()];
}

const FEATURE_BUS_HINTS: Partial<
	Record<FeatureId, { busLabel: string; busColor: string; busField?: BusField }>
> = {
	fsd: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	nagControl: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	speedProfile: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	speedOffset: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	isaChime: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	banShield: { busLabel: "Chassis", busColor: "#22d3ee", busField: "chassisOnline" },
	windowVent: { busLabel: "Body", busColor: "#fbbf24", busField: "bodyOnline" },
	turnSignal: { busLabel: "Body", busColor: "#fbbf24", busField: "bodyOnline" },
	airRecirc: { busLabel: "Body", busColor: "#fbbf24", busField: "bodyOnline" },
	lockControl: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	mirrorControl: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	lights: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	wiper: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	seatHeating: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	powerState: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	driveMode: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	trackMode: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
	summon: { busLabel: "Vehicle", busColor: "#06b6d4", busField: "vehicleOnline" },
};

const BOOLEAN_TOGGLE_STATE_KEYS: Partial<Record<CommandName, keyof BoardState>> = {
	fsd: "fsd",
	fsdForce: "fsdForce",
	isaChime: "isaChime",
	banShield: "banShield",
	tlssc: "tlsscRestore",
	eap: "enhancedAutopilot",
	evd: "evdEnabled",
	precondition: "precondition",
	summonInject: "summonInject",
	trackMode: "trackMode",
	eceR79: "eceR79",
	singleShot: "singleShot",
	seatbelt: "seatbeltEmulation",
	wiperPersist: "wiperPersist",
	mirrorAutoFold: "mirrorAutoFold",
	canSimStart: "canSim",
	canSimStop: "canSim",
};

const ADVANCED_CONFIRMATION: Partial<Record<CommandName, string>> = {
	fsdForce: "FSD Force overrides normal UI gating.",
	powerAccOff: "ACC Off can disable accessory systems.",
	powerOff: "Power Off can interrupt active vehicle systems.",
};

const COMMAND_ARG_COUNTS = new Map<CommandName, number>(
	ALL_COMMANDS.map((entry) => [entry.name, entry.argCount]),
);

const GROUPS: CommandGroup[] = FEATURE_IDS.map((featureId) => {
	const spec = FEATURE_SETTINGS_BY_ID[featureId];
	const commands = buildFeatureCommandItems(featureId);
	const busHint = FEATURE_BUS_HINTS[featureId] ?? {
		busLabel: "Mixed",
		busColor: "#8baec8",
	};

	return {
		title: spec.title,
		kind: spec.kind,
		busLabel: busHint.busLabel,
		busColor: busHint.busColor,
		busField: busHint.busField,
		commands,
	};
})
	.filter((group) => group.commands.length > 0)
	.sort((a, b) => {
		const kindRank = (kind: FeatureSpecKind): number => {
			if (kind === "command") return 0;
			if (kind === "query") return 1;
			return 2;
		};
		return kindRank(a.kind) - kindRank(b.kind) || a.title.localeCompare(b.title);
	});

const GROUP_COMMAND_MAP = new Map<CommandName, CommandItem>();
for (const group of GROUPS) {
	for (const command of group.commands) {
		GROUP_COMMAND_MAP.set(command.name, command);
	}
}

export function ControlsScreen() {
	const { boardState } = useBoardInstanceState();
	const { runCommand: onRunCommand } = useCommandActions();
	const [tooltip, setTooltip] = useState<string | null>(null);
	const [pendingConfirm, setPendingConfirm] = useState<CommandName | null>(null);
	const [paletteOpen, setPaletteOpen] = useState(false);
	const [paletteQuery, setPaletteQuery] = useState("");
	const [pinned, setPinned] = useState<CommandName[]>(["lock", "unlock", "horn", "chargePort"]);

	const commandSearch = paletteQuery.trim().toLowerCase();

	const paletteItems = useMemo(() => {
		return ALL_COMMANDS.map((descriptor) => {
			const fromGroup = GROUP_COMMAND_MAP.get(descriptor.name);
			const item: CommandItem = fromGroup
				? fromGroup
				: {
						name: descriptor.name,
						label: humanizeCommandName(descriptor.name),
						requiresArgs: descriptor.argCount > 0,
					};

			const gate = getCommandGate(item.name, boardState);
			const gateReason = gate.available ? null : gate.reason;
			const searchText = `${item.label} ${item.name}`.toLowerCase();

			return {
				item,
				searchText,
				gateReason,
				pinned: pinned.includes(item.name),
			};
		})
			.filter((entry) => (commandSearch ? entry.searchText.includes(commandSearch) : true))
			.sort((a, b) => a.item.label.localeCompare(b.item.label));
	}, [boardState, commandSearch, pinned]);

	const togglePin = (name: CommandName) => {
		setPinned((current) => {
			if (current.includes(name)) {
				return current.filter((entry) => entry !== name);
			}
			return [...current, name].slice(-8);
		});
	};

	const runWithGuards = (cmd: CommandItem, gateReason?: string | null) => {
		if (gateReason) {
			setPendingConfirm(null);
			setTooltip(gateReason);
			return;
		}

		const argCount = COMMAND_ARG_COUNTS.get(cmd.name) ?? 0;
		const toggleStateKey = BOOLEAN_TOGGLE_STATE_KEYS[cmd.name];
		const hasToggleDefault = typeof toggleStateKey === "string";

		if (argCount > 0 && !hasToggleDefault) {
			setPendingConfirm(null);
			setTooltip(
				`${cmd.label} requires input. Use Monitor > Command Runner for this command.`,
			);
			return;
		}

		const confirmationText = ADVANCED_CONFIRMATION[cmd.name];
		if (confirmationText && pendingConfirm !== cmd.name) {
			setPendingConfirm(cmd.name);
			setTooltip(`${confirmationText} Tap ${cmd.label} again to confirm.`);
			return;
		}

		setPendingConfirm(null);
		setTooltip(null);

		if (hasToggleDefault && toggleStateKey) {
			const current = Boolean(boardState[toggleStateKey]);
			onRunCommand(cmd.name, current ? "false" : "true");
			return;
		}

		onRunCommand(cmd.name);
	};

	const runDirectCommand = (name: CommandName, args?: string) => {
		const gate = getCommandGate(name, boardState);
		if (!gate.available) {
			setTooltip(gate.reason);
			return;
		}

		setPendingConfirm(null);
		setTooltip(null);
		onRunCommand(name, args);
	};

	const commandGroups = GROUPS.filter((group) => group.kind === "command");
	const queryGroups = GROUPS.filter((group) => group.kind === "query");

	return (
		<ScrollView
			className="flex-1 bg-background"
			contentContainerStyle={{ padding: 12, gap: 12, paddingBottom: 48 }}
		>
			<BusStatusBar
				boardState={boardState}
				preset={ACTIVE_PRESET}
				onOpenPalette={() => {
					setPaletteOpen(true);
					setPaletteQuery("");
				}}
			/>

			<View className="flex-row flex-wrap" style={{ gap: 12 }}>
				<View className="bg-card rounded-xl border border-border p-3 flex-1 min-w-[280px] flex-shrink gap-2">
					<View className="flex-row items-center justify-between">
						<Text className="text-sm font-semibold text-card-foreground">FSD</Text>
						<Text className="text-xs font-bold text-green-500">
							{boardState.fsd ? "ON" : "OFF"}
						</Text>
					</View>
					<FsdPanel boardState={boardState} onCommand={runDirectCommand} />
				</View>
			</View>

			<View className="flex-row flex-wrap" style={{ gap: 12 }}>
				<View className="flex-1 min-w-[280px] flex-shrink" style={{ gap: 12 }}>
					<SpeedTuningPanel boardState={boardState} onCommand={runDirectCommand} />
					<DasPanel boardState={boardState} onCommand={runDirectCommand} />
				</View>
				<View className="flex-1 min-w-[280px] flex-shrink" style={{ gap: 12 }}>
					<GamepadPanel boardState={boardState} onCommand={runDirectCommand} />
					<BmsPanel boardState={boardState} onCommand={runDirectCommand} />
				</View>
			</View>

			<VehicleCommandsPanel boardState={boardState} onCommand={runDirectCommand} />

			{tooltip ? (
				<View className="flex-row items-center justify-between bg-muted rounded-lg border border-border p-3">
					<Text className="text-sm text-foreground flex-1">{tooltip}</Text>
					<Text
						className="text-xs font-bold text-muted-foreground ml-2"
						onPress={() => {
							setTooltip(null);
							setPendingConfirm(null);
						}}
					>
						✕
					</Text>
				</View>
			) : null}

			{commandGroups.length > 0 ? (
				<View className="gap-2">
					<Text className="text-sm font-semibold text-muted-foreground uppercase">
						Feature Commands
					</Text>
					{commandGroups.map((group) => (
						<CommandGroupCard
							key={group.title}
							group={group}
							boardState={boardState}
							preset={ACTIVE_PRESET}
							onRun={runWithGuards}
						/>
					))}
				</View>
			) : null}

			{queryGroups.length > 0 ? (
				<View className="gap-2">
					<Text className="text-sm font-semibold text-muted-foreground uppercase">
						Telemetry & Queries
					</Text>
					{queryGroups.map((group) => (
						<CommandGroupCard
							key={group.title}
							group={group}
							boardState={boardState}
							preset={ACTIVE_PRESET}
							onRun={runWithGuards}
						/>
					))}
				</View>
			) : null}

			<CommandPaletteModal
				visible={paletteOpen}
				query={paletteQuery}
				preset={ACTIVE_PRESET}
				entries={paletteItems}
				onChangeQuery={setPaletteQuery}
				onRun={runWithGuards}
				onTogglePin={togglePin}
				onRequestClose={() => setPaletteOpen(false)}
			/>
		</ScrollView>
	);
}
