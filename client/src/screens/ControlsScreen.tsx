import React, { useMemo, useState } from "react";
import { Pressable, ScrollView, StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import {
	FEATURE_IDS,
	FEATURE_SETTINGS_BY_ID,
	getFeatureSettingsSpecById,
	type FeatureId,
	type FeatureSpecKind,
} from "../../../packages/protocol/src/featureSettings";
import { colors, font, radius, spacing } from "../design/tokens";
import { getCommandGate } from "../state/commandGating";
import { ALL_COMMANDS, type CommandName } from "../hardware/controller";
import {
	BusStatusBar,
	CommandGroupCard,
	CommandPaletteModal,
	SpeedTuningCard,
	TooltipBanner,
} from "../components/controls/ControlsBlocks";

interface ControlsScreenProps {
	boardState: BoardState;
	onRunCommand: (name: CommandName, args?: string) => void;
}

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

const FEATURE_BUS_HINTS: Partial<
	Record<FeatureId, { busLabel: string; busColor: string; busField?: BusField }>
> = {
	fsd: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	nagControl: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	speedProfile: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	speedOffset: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	isaChime: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	banShield: { busLabel: "Chassis", busColor: colors.apActive, busField: "chassisOnline" },
	windowVent: { busLabel: "Body", busColor: colors.alarmWarning, busField: "bodyOnline" },
	turnSignal: { busLabel: "Body", busColor: colors.alarmWarning, busField: "bodyOnline" },
	airRecirc: { busLabel: "Body", busColor: colors.alarmWarning, busField: "bodyOnline" },
	lockControl: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	mirrorControl: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	lights: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	wiper: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	seatHeating: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	powerState: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	driveMode: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	trackMode: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
	summon: { busLabel: "Vehicle", busColor: colors.primary, busField: "vehicleOnline" },
};

const BOOLEAN_TOGGLE_STATE_KEYS: Partial<Record<CommandName, keyof BoardState>> = {
	fsd: "fsd",
	fsdForce: "fsdForce",
	nag: "nag",
	isaChime: "isaChime",
	nagKiller: "nagKiller",
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
	nagKiller: "Nag Killer affects steering-input safety behavior.",
	powerAccOff: "ACC Off can disable accessory systems.",
	powerOff: "Power Off can interrupt active vehicle systems.",
};

const COMMAND_ARG_COUNTS = new Map<CommandName, number>(
	ALL_COMMANDS.map((entry) => [entry.name, entry.argCount]),
);

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

const GROUPS: CommandGroup[] = FEATURE_IDS.map((featureId) => {
	const spec = FEATURE_SETTINGS_BY_ID[featureId];
	const commands = buildFeatureCommandItems(featureId);
	const busHint = FEATURE_BUS_HINTS[featureId] ?? {
		busLabel: "Mixed",
		busColor: colors.dashSecondary,
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

export function ControlsScreen({ boardState, onRunCommand }: ControlsScreenProps) {
	const [tooltip, setTooltip] = useState<string | null>(null);
	const [pendingConfirm, setPendingConfirm] = useState<CommandName | null>(null);
	const [paletteOpen, setPaletteOpen] = useState(false);
	const [paletteQuery, setPaletteQuery] = useState("");
	const [pinned, setPinned] = useState<CommandName[]>(["lock", "unlock", "horn", "chargePort"]);

	const profileSetting = getFeatureSettingsSpecById("speedProfile")?.settings.find(
		(setting) => setting.id === "profile",
	);
	const profileLevelLabel = profileSetting?.label ?? "Profile Level";

	const commandSearch = paletteQuery.trim().toLowerCase();
	const profileValue = typeof boardState.profile === "number" ? boardState.profile : 0;
	const offsetValue = typeof boardState.offset === "number" ? boardState.offset : 0;
	const bmsVoltage = typeof boardState.bmsVoltage === "number" ? boardState.bmsVoltage : 0;
	const bmsCurrent = typeof boardState.bmsCurrent === "number" ? boardState.bmsCurrent : 0;
	const bmsPower = typeof boardState.bmsPower === "number" ? boardState.bmsPower : 0;
	const bmsSoc = typeof boardState.bmsSoc === "number" ? boardState.bmsSoc : 0;
	const bmsTempMin = typeof boardState.bmsTempMin === "number" ? boardState.bmsTempMin : 0;
	const bmsTempMax = typeof boardState.bmsTempMax === "number" ? boardState.bmsTempMax : 0;

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

	const importantFeatures = [
		{
			title: "FSD",
			active: boardState.fsd,
			activeLabel: boardState.fsd ? "ON" : "OFF",
			primary: { label: "Enable", action: () => runDirectCommand("fsd", "true") },
			secondary: { label: "Disable", action: () => runDirectCommand("fsd", "false") },
		},
		{
			title: "Nag Suppress",
			active: boardState.nag,
			activeLabel: boardState.nag ? "ON" : "OFF",
			primary: { label: "Enable", action: () => runDirectCommand("nag", "true") },
			secondary: { label: "Disable", action: () => runDirectCommand("nag", "false") },
		},
		{
			title: "Nag Killer",
			active: boardState.nagKiller,
			activeLabel: boardState.nagKiller ? "ON" : "OFF",
			primary: { label: "Enable", action: () => runDirectCommand("nagKiller", "true") },
			secondary: { label: "Disable", action: () => runDirectCommand("nagKiller", "false") },
		},
		{
			title: "Track Mode",
			active: boardState.trackMode,
			activeLabel: boardState.trackMode ? "ON" : "OFF",
			primary: { label: "Enable", action: () => runDirectCommand("trackMode", "true") },
			secondary: { label: "Disable", action: () => runDirectCommand("trackMode", "false") },
		},
		{
			title: "Precondition",
			active: boardState.precondition,
			activeLabel: boardState.precondition ? "ON" : "OFF",
			primary: { label: "Enable", action: () => runDirectCommand("precondition", "true") },
			secondary: {
				label: "Disable",
				action: () => runDirectCommand("precondition", "false"),
			},
		},
		{
			title: "Summon",
			active: boardState.summonActive,
			activeLabel: boardState.summonActive ? "Active" : "Idle",
			primary: { label: "Fwd", action: () => runDirectCommand("summonForward") },
			secondary: { label: "Rev", action: () => runDirectCommand("summonReverse") },
			tertiary: { label: "Stop", action: () => runDirectCommand("summonStop") },
		},
	];

	return (
		<ScrollView style={styles.root} contentContainerStyle={styles.content}>
			<BusStatusBar
				boardState={boardState}
				preset={ACTIVE_PRESET}
				onOpenPalette={() => {
					setPaletteOpen(true);
					setPaletteQuery("");
				}}
			/>

			<View style={styles.topGrid}>
				<View style={styles.panelCard}>
					<View style={styles.panelHeader}>
						<Text style={styles.panelTitle}>Board</Text>
						<View style={styles.badgeAccent}>
							<Text style={styles.badgeAccentText}>
								{String(boardState.variant || "unknown").toUpperCase()}
							</Text>
						</View>
					</View>
					<View style={styles.healthBanner}>
						<View
							style={[
								styles.healthDot,
								{
									backgroundColor:
										boardState.chassisOnline ||
										boardState.vehicleOnline ||
										boardState.bodyOnline
											? colors.powerPositive
											: colors.statusDisconnected,
								},
							]}
						/>
						<View style={styles.healthCopy}>
							<Text style={styles.healthTitle}>
								{boardState.otaInProgress ? "Standby" : "CAN Active"}
							</Text>
							<Text style={styles.healthSubtitle}>
								{boardState.otaInProgress
									? "TX paused during OTA/update state"
									: "Vehicle buses responding"}
							</Text>
						</View>
					</View>
					<View style={styles.statGrid}>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Board</Text>
							<Text style={styles.statValue}>{boardState.hardware || "-"}</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Driver</Text>
							<Text style={styles.statValue}>{boardState.driver || "-"}</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Uptime</Text>
							<Text style={styles.statValue}>
								{Math.max(0, Math.floor((boardState.uptime || 0) / 1000))}s
							</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Detected HW</Text>
							<Text style={styles.statValue}>
								{boardState.detectedHW > 0 ? `HW${boardState.detectedHW}` : "Auto"}
							</Text>
						</View>
					</View>
				</View>

				<View style={styles.panelCard}>
					<View style={styles.panelHeader}>
						<Text style={styles.panelTitle}>EEPROM</Text>
						<View style={styles.badgeMuted}>
							<Text style={styles.badgeMutedText}>Saved</Text>
						</View>
					</View>
					<View style={styles.settingsGrid}>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>Variant</Text>
							<Text style={styles.settingsValue}>
								{String(boardState.variant || "unknown").toUpperCase()}
							</Text>
						</View>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>FSD</Text>
							<Text
								style={[
									styles.settingsValue,
									boardState.fsd ? styles.valueOn : styles.valueOff,
								]}
							>
								{boardState.fsd ? "ON" : "OFF"}
							</Text>
						</View>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>Nag</Text>
							<Text
								style={[
									styles.settingsValue,
									boardState.nag ? styles.valueOn : styles.valueOff,
								]}
							>
								{boardState.nag ? "ON" : "OFF"}
							</Text>
						</View>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>{profileLevelLabel}</Text>
							<Text style={styles.settingsValue}>
								{profileValue} {boardState.profilePinned ? "(pinned)" : "(auto)"}
							</Text>
						</View>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>Offset</Text>
							<Text style={styles.settingsValue}>
								{offsetValue}% {boardState.offsetPinned ? "(pinned)" : "(auto)"}
							</Text>
						</View>
						<View style={styles.settingsItem}>
							<Text style={styles.settingsKey}>ISA</Text>
							<Text style={styles.settingsValue}>
								{boardState.isaChime ? "Suppress" : "Original"}
							</Text>
						</View>
					</View>
				</View>
			</View>

			<View style={styles.featureTileGrid}>
				{importantFeatures.map((feature) => (
					<View key={feature.title} style={styles.featureTile}>
						<View style={styles.featureTileHeader}>
							<Text style={styles.featureTileTitle}>{feature.title}</Text>
							<Text
								style={[
									styles.featureStatus,
									feature.active ? styles.statusOn : styles.statusOff,
								]}
							>
								{feature.activeLabel}
							</Text>
						</View>
						<View style={styles.featureTileActions}>
							<Pressable
								style={({ pressed }) => [
									styles.featureButton,
									feature.active ? styles.featureButtonActive : undefined,
									pressed ? styles.featureButtonPressed : undefined,
								]}
								onPress={feature.primary.action}
							>
								<Text
									style={[
										styles.featureButtonText,
										feature.active ? styles.featureButtonTextActive : undefined,
									]}
								>
									{feature.primary.label}
								</Text>
							</Pressable>
							<Pressable
								style={({ pressed }) => [
									styles.featureButton,
									!feature.active && !feature.tertiary
										? styles.featureButtonActive
										: undefined,
									pressed ? styles.featureButtonPressed : undefined,
								]}
								onPress={feature.secondary.action}
							>
								<Text
									style={[
										styles.featureButtonText,
										!feature.active && !feature.tertiary
											? styles.featureButtonTextActive
											: undefined,
									]}
								>
									{feature.secondary.label}
								</Text>
							</Pressable>
							{feature.tertiary ? (
								<Pressable
									style={({ pressed }) => [
										styles.featureButton,
										styles.featureButtonDanger,
										pressed ? styles.featureButtonPressed : undefined,
									]}
									onPress={feature.tertiary.action}
								>
									<Text style={styles.featureButtonTextActive}>
										{feature.tertiary.label}
									</Text>
								</Pressable>
							) : null}
						</View>
					</View>
				))}
			</View>

			<SpeedTuningCard
				boardState={boardState}
				preset={ACTIVE_PRESET}
				profileControlsTitle="Speed Profile Controls"
				profileLevelLabel={profileLevelLabel}
				onSetProfile={(profile) => runDirectCommand("profile", String(profile))}
				onSetProfileAuto={() => runDirectCommand("profileAuto")}
				onSetOffset={(offset) => runDirectCommand("offset", String(offset))}
				onSetOffsetAuto={() => runDirectCommand("offsetAuto")}
			/>

			{tooltip ? (
				<TooltipBanner
					message={tooltip}
					onClose={() => {
						setTooltip(null);
						setPendingConfirm(null);
					}}
				/>
			) : null}

			{boardState.otaInProgress || boardState.txPaused || boardState.detectedHW > 0 ? (
				<View style={styles.panelCard}>
					<View style={styles.panelHeader}>
						<Text style={styles.panelTitle}>Vehicle Status</Text>
					</View>
					<View style={styles.settingsGrid}>
						{boardState.otaInProgress ? (
							<View style={styles.settingsItem}>
								<Text style={styles.settingsKey}>OTA</Text>
								<Text style={[styles.settingsValue, styles.valueDanger]}>
									In Progress
								</Text>
							</View>
						) : null}
						{boardState.txPaused ? (
							<View style={styles.settingsItem}>
								<Text style={styles.settingsKey}>TX</Text>
								<Text style={[styles.settingsValue, styles.valueDanger]}>
									Paused
								</Text>
							</View>
						) : null}
						{boardState.detectedHW > 0 ? (
							<View style={styles.settingsItem}>
								<Text style={styles.settingsKey}>Detected HW</Text>
								<Text style={styles.settingsValue}>HW{boardState.detectedHW}</Text>
							</View>
						) : null}
					</View>
				</View>
			) : null}

			{boardState.hasBms ? (
				<View style={styles.panelCard}>
					<View style={styles.panelHeader}>
						<Text style={styles.panelTitle}>Battery (BMS)</Text>
						<View style={styles.badgeAccent}>
							<Text style={styles.badgeAccentText}>Live</Text>
						</View>
					</View>
					<View style={styles.statGrid}>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Voltage</Text>
							<Text style={styles.statValue}>{bmsVoltage.toFixed(1)}V</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Current</Text>
							<Text style={styles.statValue}>{bmsCurrent.toFixed(1)}A</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Power</Text>
							<Text style={styles.statValue}>{bmsPower.toFixed(1)}kW</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>SoC</Text>
							<Text style={styles.statValue}>{bmsSoc.toFixed(1)}%</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Temp Min</Text>
							<Text style={styles.statValue}>{bmsTempMin}°C</Text>
						</View>
						<View style={styles.statCell}>
							<Text style={styles.statLabel}>Temp Max</Text>
							<Text style={styles.statValue}>{bmsTempMax}°C</Text>
						</View>
					</View>
				</View>
			) : null}

			<View style={styles.sectionBlock}>
				<Text style={styles.sectionHeader}>Feature Commands</Text>
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

			{queryGroups.length > 0 ? (
				<View style={styles.sectionBlock}>
					<Text style={styles.sectionHeader}>Telemetry & Queries</Text>
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

const styles = StyleSheet.create({
	root: {
		flex: 1,
		backgroundColor: colors.dashBackground,
	},
	content: {
		padding: spacing.md,
		gap: spacing.md,
		paddingBottom: spacing.xl4,
	},
	topGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: spacing.md,
	},
	panelCard: {
		backgroundColor: colors.dashCard,
		borderRadius: radius.lg,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		padding: spacing.md,
		gap: spacing.sm,
		flexGrow: 1,
		flexBasis: 280,
	},
	panelHeader: {
		flexDirection: "row",
		justifyContent: "space-between",
		alignItems: "center",
	},
	panelTitle: {
		color: colors.dashValue,
		fontSize: font.size.md,
		fontWeight: font.weight.semibold,
	},
	badgeAccent: {
		borderRadius: radius.full,
		paddingHorizontal: spacing.sm,
		paddingVertical: 4,
		backgroundColor: colors.primary,
	},
	badgeAccentText: {
		color: colors.backgroundDark,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
	},
	badgeMuted: {
		borderRadius: radius.full,
		paddingHorizontal: spacing.sm,
		paddingVertical: 4,
		backgroundColor: colors.dashCardBorder,
	},
	badgeMutedText: {
		color: colors.dashSecondary,
		fontSize: font.size.xs,
		fontWeight: font.weight.semibold,
	},
	healthBanner: {
		flexDirection: "row",
		alignItems: "center",
		gap: spacing.sm,
		borderRadius: radius.md,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.backgroundDarkSubtle,
		padding: spacing.sm,
	},
	healthDot: {
		width: 10,
		height: 10,
		borderRadius: radius.full,
	},
	healthCopy: {
		flex: 1,
	},
	healthTitle: {
		color: colors.dashValue,
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
	},
	healthSubtitle: {
		color: colors.dashSecondary,
		fontSize: font.size.xs,
	},
	statGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: spacing.sm,
	},
	statCell: {
		minWidth: 120,
		flexGrow: 1,
		backgroundColor: colors.backgroundDarkSubtle,
		borderRadius: radius.md,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		padding: spacing.sm,
		gap: 2,
	},
	statLabel: {
		color: colors.dashSecondary,
		fontSize: font.size.xs,
	},
	statValue: {
		color: colors.dashValue,
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
	},
	settingsGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: spacing.sm,
	},
	settingsItem: {
		minWidth: 130,
		flexGrow: 1,
		gap: 2,
	},
	settingsKey: {
		color: colors.dashSecondary,
		fontSize: font.size.xs,
	},
	settingsValue: {
		color: colors.dashValue,
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
	},
	valueOn: {
		color: colors.powerPositive,
	},
	valueOff: {
		color: colors.dashSecondary,
	},
	valueDanger: {
		color: colors.alarmCritical,
	},
	featureTileGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: spacing.md,
	},
	featureTile: {
		flexBasis: 220,
		flexGrow: 1,
		backgroundColor: colors.dashCard,
		borderRadius: radius.lg,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		padding: spacing.md,
		gap: spacing.sm,
	},
	featureTileHeader: {
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "space-between",
		gap: spacing.sm,
	},
	featureTileTitle: {
		color: colors.dashValue,
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
	},
	featureStatus: {
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
	},
	statusOn: {
		color: colors.powerPositive,
	},
	statusOff: {
		color: colors.dashSecondary,
	},
	featureTileActions: {
		flexDirection: "row",
		gap: spacing.xs,
	},
	featureButton: {
		flex: 1,
		minHeight: 38,
		borderRadius: radius.md,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.backgroundDarkSubtle,
		alignItems: "center",
		justifyContent: "center",
		paddingHorizontal: spacing.sm,
	},
	featureButtonActive: {
		borderColor: colors.primary,
		backgroundColor: colors.primary,
	},
	featureButtonDanger: {
		borderColor: colors.alarmCritical,
		backgroundColor: colors.alarmCritical,
	},
	featureButtonPressed: {
		opacity: 0.82,
	},
	featureButtonText: {
		color: colors.dashValue,
		fontSize: font.size.xs,
		fontWeight: font.weight.semibold,
	},
	featureButtonTextActive: {
		color: colors.backgroundDark,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
	},
	sectionBlock: {
		gap: spacing.sm,
	},
	sectionHeader: {
		color: colors.dashSecondary,
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
		letterSpacing: 0.5,
	},
});
