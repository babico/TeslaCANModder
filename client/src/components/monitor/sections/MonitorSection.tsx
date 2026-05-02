import { useState } from "react";
import {
	Pressable,
	ScrollView,
	StyleSheet,
	Text,
	TextInput,
	View,
	useWindowDimensions,
} from "react-native";
import { colors } from "../../../design/tokens";
import type { MonitorScreenProps } from "./types";

const BUS_FILTER_OPTIONS: Array<{ value: string; label: string }> = [
	{ value: "all", label: "All" },
	{ value: "0", label: "Chassis" },
	{ value: "1", label: "Vehicle" },
	{ value: "2", label: "Body" },
];

const WINDOW_PRESETS = [25, 50, 100, 200];
const SAMPLE_PRESETS = [1, 2, 5, 10];

function tryFormatJsonText(raw: string): string | null {
	const trimmed = raw.trim();
	if (!(trimmed.startsWith("{") || trimmed.startsWith("["))) {
		return null;
	}

	try {
		const parsed = JSON.parse(trimmed);
		return JSON.stringify(parsed, null, 2);
	} catch {
		return null;
	}
}

function formatBoardMessageText(text: string): string {
	const normalized = text.replace(/\r\n/g, "\n");
	const lines = normalized.split("\n");

	const formatted = lines.map((line) => {
		const trimmed = line.trim();
		if (!trimmed) {
			return line;
		}

		const prefixedMatch = trimmed.match(/^([<>])\s+(.*)$/);
		if (prefixedMatch) {
			const [, prefix, payload] = prefixedMatch;
			const pretty = tryFormatJsonText(payload);
			if (!pretty) {
				return line;
			}

			return pretty
				.split("\n")
				.map((prettyLine, index) =>
					index === 0 ? `${prefix} ${prettyLine}` : `  ${prettyLine}`,
				)
				.join("\n");
		}

		const pretty = tryFormatJsonText(trimmed);
		return pretty ?? line;
	});

	return formatted.join("\n");
}

export function MonitorSection(props: MonitorScreenProps) {
	const { width } = useWindowDimensions();
	const isWide = width >= 1024;
	const [rawCommand, setRawCommand] = useState("status");
	const [rawBusy, setRawBusy] = useState(false);
	const boardMessages = Array.isArray(props.boardState.messages)
		? props.boardState.messages.slice(0, 80)
		: [];

	const runRawFromConsole = async () => {
		const cmd = rawCommand.trim();
		if (!cmd) {
			return;
		}

		setRawBusy(true);
		try {
			await props.onRunRawCommand(cmd);
		} catch {
			// Errors are surfaced in the board message stream and last-result panels.
		} finally {
			setRawBusy(false);
		}
	};

	return (
		<View style={styles.section}>
			<Text style={styles.title}>CAN Monitor</Text>
			<Text style={styles.subtitle}>Frames + console stream</Text>

			<View style={[styles.card, isWide ? styles.cardWide : undefined]}>
				<View style={styles.col}>
					<Text style={styles.label}>Decoder</Text>
					<View style={styles.chipRow}>
						{props.decoderDatasets.map((entry) => (
							<Pressable
								key={entry.id}
								onPress={() => props.onDatasetChange(entry.id)}
								style={[
									styles.chip,
									props.selectedDecoderDataset.id === entry.id
										? styles.chipActive
										: undefined,
								]}
							>
								<Text style={styles.chipText}>{entry.label}</Text>
							</Pressable>
						))}
					</View>

					<Text style={styles.label}>Bus Filter</Text>
					<View style={styles.chipRow}>
						{BUS_FILTER_OPTIONS.map((opt) => (
							<Pressable
								key={opt.value}
								onPress={() => props.onBusFilterChange(opt.value)}
								style={[
									styles.chip,
									props.busFilter === opt.value ? styles.chipActive : undefined,
								]}
							>
								<Text style={styles.chipText}>{opt.label}</Text>
							</Pressable>
						))}
					</View>

					<Text style={styles.label}>Search</Text>
					<TextInput
						style={styles.input}
						value={props.frameFilter}
						onChangeText={props.onFrameFilterChange}
						placeholder="Search id, key, decoded name"
						placeholderTextColor={colors.dashMuted}
					/>
				</View>

				<View style={styles.colNarrow}>
					<Text style={styles.label}>Window</Text>
					<View style={styles.chipRow}>
						{WINDOW_PRESETS.map((preset) => (
							<Pressable
								key={`window-${preset}`}
								onPress={() => props.onFrameWindowSizeChange(preset)}
								style={[
									styles.chip,
									props.frameWindowSize === preset
										? styles.chipActive
										: undefined,
								]}
							>
								<Text style={styles.chipText}>{preset}</Text>
							</Pressable>
						))}
					</View>

					<Text style={styles.label}>Sample</Text>
					<View style={styles.chipRow}>
						{SAMPLE_PRESETS.map((preset) => (
							<Pressable
								key={`sample-${preset}`}
								onPress={() => props.onFrameSampleStepChange(preset)}
								style={[
									styles.chip,
									props.frameSampleStep === preset
										? styles.chipActive
										: undefined,
								]}
							>
								<Text style={styles.chipText}>{preset}x</Text>
							</Pressable>
						))}
					</View>

					<View style={styles.actionRow}>
						<ActionButton
							title={props.frameFeedPaused ? "Paused" : "Live"}
							onPress={() => props.onFrameFeedPausedChange(!props.frameFeedPaused)}
						/>
						<ActionButton
							title="Snapshot"
							onPress={props.onSaveSnapshot}
							variant="secondary"
						/>
						<ActionButton
							title="Clear"
							onPress={props.onClearFeed}
							variant="secondary"
						/>
					</View>
				</View>
			</View>

			<View style={styles.card}>
				<Text style={styles.label}>
					Live Feed ({props.visibleFrames.length}/{props.boardState.frameCount})
				</Text>
				<View style={styles.feedList}>
					{props.visibleFrames.slice(0, 32).map((frame) => (
						<View key={frame.key} style={styles.feedRow}>
							<Text style={styles.feedKey}>
								{frame.busName} 0x{frame.id.toString(16).toUpperCase()}
							</Text>
							<Text style={styles.feedData} numberOfLines={1}>
								{props.frameDecodedNameByKey[frame.key] || frame.data}
							</Text>
						</View>
					))}
					{props.visibleFrames.length === 0 ? (
						<Text style={styles.empty}>No visible frames.</Text>
					) : null}
				</View>
			</View>

			<View style={styles.card}>
				<View style={styles.boardInfoHeaderRow}>
					<Text style={styles.label}>Board Info Stream ({boardMessages.length})</Text>
					<ActionButton
						title={props.boardInfoFeedPaused ? "Paused" : "Live"}
						onPress={() =>
							props.onBoardInfoFeedPausedChange(!props.boardInfoFeedPaused)
						}
						variant="secondary"
					/>
				</View>
				<Text style={styles.subtitle}>Tracks all outputs reported by the board.</Text>

				<View style={styles.consoleRow}>
					<Text style={styles.prompt}>cmd&gt;</Text>
					<TextInput
						style={styles.consoleInput}
						value={rawCommand}
						onChangeText={setRawCommand}
						placeholder="status"
						placeholderTextColor={colors.dashMuted}
						autoCapitalize="none"
						autoCorrect={false}
						editable={!rawBusy}
						onSubmitEditing={() => void runRawFromConsole()}
					/>
					<ActionButton
						title={rawBusy ? "Sending" : "Send"}
						onPress={() => void runRawFromConsole()}
						disabled={rawBusy}
					/>
				</View>

				<ScrollView
					style={styles.messageList}
					contentContainerStyle={styles.messageListContent}
				>
					{boardMessages.length === 0 ? (
						<Text style={styles.empty}>No board messages yet.</Text>
					) : (
						boardMessages.map((message, index) => (
							<View
								key={`${message.id}-${message.ts}-${index}`}
								style={styles.messageRow}
							>
								<Text style={styles.messageMeta}>
									{message.ts} · {message.type.toUpperCase()}
								</Text>
								<ScrollView
									style={styles.messageTextScroll}
									contentContainerStyle={styles.messageTextScrollContent}
									nestedScrollEnabled
									showsVerticalScrollIndicator
								>
									<Text style={styles.messageText}>
										{formatBoardMessageText(message.text)}
									</Text>
								</ScrollView>
							</View>
						))
					)}
				</ScrollView>
			</View>
		</View>
	);
}

function ActionButton({
	title,
	onPress,
	variant = "primary",
	disabled = false,
}: {
	title: string;
	onPress: () => void;
	variant?: "primary" | "secondary";
	disabled?: boolean;
}) {
	return (
		<Pressable
			onPress={disabled ? undefined : onPress}
			style={({ pressed }) => [
				styles.btn,
				variant === "secondary" ? styles.btnSecondary : undefined,
				disabled ? styles.btnDisabled : undefined,
				pressed && !disabled ? styles.btnPressed : undefined,
			]}
		>
			<Text
				style={[
					styles.btnText,
					variant === "secondary" ? styles.btnTextSecondary : undefined,
				]}
			>
				{title}
			</Text>
		</Pressable>
	);
}

const styles = StyleSheet.create({
	section: { gap: 12 },
	title: { color: colors.dashValue, fontSize: 16, fontWeight: "700" },
	subtitle: { color: colors.dashLabel, fontSize: 12 },
	card: {
		backgroundColor: colors.dashCard,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 10,
		padding: 12,
		gap: 10,
	},
	cardWide: { flexDirection: "row" },
	col: { flex: 1, gap: 8 },
	colNarrow: { flex: 1, gap: 8 },
	label: { color: colors.dashMuted, fontSize: 11, textTransform: "uppercase", fontWeight: "700" },
	boardInfoHeaderRow: {
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "space-between",
		gap: 8,
	},
	chipRow: { flexDirection: "row", flexWrap: "wrap", gap: 6 },
	chip: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 999,
		backgroundColor: colors.backgroundDarkSubtle,
		paddingHorizontal: 10,
		paddingVertical: 5,
	},
	chipActive: { borderColor: colors.dashPrimary },
	chipText: { color: colors.dashLabel, fontSize: 12, fontWeight: "600" },
	input: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 8,
		backgroundColor: colors.backgroundDarkSubtle,
		color: colors.dashValue,
		paddingHorizontal: 10,
		paddingVertical: 8,
	},
	actionRow: { flexDirection: "row", flexWrap: "wrap", gap: 8 },
	feedList: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 8,
		backgroundColor: colors.dashBackground,
		maxHeight: 300,
		gap: 4,
		padding: 8,
	},
	feedRow: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 6,
		backgroundColor: colors.dashCard,
		paddingHorizontal: 8,
		paddingVertical: 6,
		gap: 2,
	},
	messageList: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 8,
		backgroundColor: colors.dashBackground,
		maxHeight: 320,
	},
	messageListContent: {
		padding: 8,
		gap: 6,
	},
	messageRow: {
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 6,
		backgroundColor: colors.backgroundDarkSubtle,
		paddingHorizontal: 8,
		paddingVertical: 7,
		gap: 2,
		maxHeight: 220,
	},
	messageTextScroll: {
		maxHeight: 170,
	},
	messageTextScrollContent: {
		paddingBottom: 2,
	},
	messageMeta: {
		color: colors.dashMuted,
		fontSize: 10,
		fontWeight: "700",
		textTransform: "uppercase",
	},
	messageText: {
		color: colors.dashValue,
		fontSize: 12,
		fontWeight: "500",
	},
	feedKey: { color: colors.dashMuted, fontSize: 11, fontWeight: "700" },
	feedData: { color: colors.dashValue, fontSize: 12, fontWeight: "600" },
	empty: { color: colors.dashMuted, fontSize: 12 },
	consoleRow: { flexDirection: "row", alignItems: "center", gap: 8 },
	prompt: { color: colors.dashLabel, fontWeight: "700" },
	consoleInput: {
		flex: 1,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: 8,
		backgroundColor: colors.backgroundDarkSubtle,
		color: colors.dashValue,
		paddingHorizontal: 10,
		paddingVertical: 8,
	},
	btn: {
		borderWidth: 1,
		borderColor: colors.dashPrimary,
		backgroundColor: colors.dashPrimary,
		borderRadius: 8,
		paddingHorizontal: 10,
		paddingVertical: 8,
	},
	btnSecondary: {
		backgroundColor: colors.backgroundDarkSubtle,
		borderColor: colors.dashCardBorder,
	},
	btnDisabled: { opacity: 0.6 },
	btnPressed: { opacity: 0.85 },
	btnText: { color: colors.backgroundDark, fontSize: 12, fontWeight: "700" },
	btnTextSecondary: { color: colors.dashValue },
});
