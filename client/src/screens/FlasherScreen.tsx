import { useMemo, useState } from "react";
import { Linking, Platform, Pressable, ScrollView, StyleSheet, Text, View } from "react-native";

import { colors, font, radius, spacing } from "../design/tokens";
import { useBoardConnection } from "../state/BoardConnectionContext";

type StatusType = "idle" | "building" | "connecting" | "flashing" | "done" | "error";

type ConnectionOption = {
	key: "serial" | "wifi" | "ble";
	label: string;
	desc: string;
	locked?: boolean;
};

const CONNECTION_ESP32: ConnectionOption[] = [
	{ key: "serial", label: "USB Serial", desc: "Always on", locked: true },
	{ key: "wifi", label: "WiFi", desc: "REST API + OTA" },
	{ key: "ble", label: "BLE", desc: "NimBLE GATT" },
];

const BUS_OPTIONS = [
	{ key: "chassis", label: "Chassis", desc: "X179 pins 13-14", locked: true },
	{ key: "vehicle", label: "Vehicle", desc: "X179 pins 9-10" },
	{ key: "body", label: "Body", desc: "X179 pins 2-3" },
] as const;

type BusKey = (typeof BUS_OPTIONS)[number]["key"];

type ReleaseAsset = {
	name: string;
	browser_download_url: string;
};

type GitHubRelease = {
	tag_name?: string;
	assets?: ReleaseAsset[];
};

const LATEST_RELEASE_ENDPOINT =
	"https://api.github.com/repos/babico/TeslaCANModder/releases/latest";

function isSerialFamilyTransport(type: string): boolean {
	return type === "serial" || type === "bluetooth-serial";
}

function resolveEnvironment(connection: Record<string, number>): string {
	const wifiEnabled = connection.wifi ? 1 : 0;
	const bleEnabled = connection.ble ? 1 : 0;
	if (wifiEnabled && bleEnabled) {
		return "esp32_wifi_ble";
	}
	if (wifiEnabled) {
		return "esp32_wifi";
	}
	if (bleEnabled) {
		return "esp32_ble";
	}
	return "esp32";
}

function resolveAssetName(env: string, buses: Record<BusKey, number>): string {
	const filenameParts = [env];
	if (buses.vehicle) {
		filenameParts.push("vehicle");
	}
	if (buses.body) {
		filenameParts.push("body");
	}
	return `${filenameParts.join("_")}.bin`;
}

async function downloadReleaseBinary(env: string, buses: Record<BusKey, number>): Promise<string> {
	const assetName = resolveAssetName(env, buses);
	const response = await fetch(LATEST_RELEASE_ENDPOINT, {
		headers: { Accept: "application/vnd.github+json" },
	});

	if (!response.ok) {
		throw new Error(`Failed to query latest GitHub release (HTTP ${response.status}).`);
	}

	const release = (await response.json()) as GitHubRelease;
	const asset = release.assets?.find((entry) => entry.name === assetName);
	if (!asset?.browser_download_url) {
		const tagText = release.tag_name ? ` (${release.tag_name})` : "";
		throw new Error(`${assetName} is not attached to the latest GitHub release${tagText}.`);
	}

	if (Platform.OS === "web") {
		const anchor = document.createElement("a");
		anchor.href = asset.browser_download_url;
		anchor.download = asset.name;
		anchor.rel = "noopener noreferrer";
		document.body.appendChild(anchor);
		anchor.click();
		anchor.remove();
	} else {
		await Linking.openURL(asset.browser_download_url);
	}

	return asset.name;
}

export function FlasherScreen() {
	const sharedConnection = useBoardConnection();
	const [connection, setConnection] = useState<Record<string, number>>({
		serial: 1,
		wifi: 0,
		ble: 0,
	});
	const [buses, setBuses] = useState<Record<BusKey, number>>({ chassis: 1, vehicle: 0, body: 0 });
	const [status, setStatus] = useState<StatusType>("idle");
	const [message, setMessage] = useState("");
	const [flashLog, setFlashLog] = useState<string[]>([]);

	const environment = useMemo(() => resolveEnvironment(connection), [connection]);
	const assetName = useMemo(() => resolveAssetName(environment, buses), [environment, buses]);
	const buildFlags = useMemo(() => {
		const flags: string[] = [];
		if (buses.chassis) flags.push("-DBUS_CHASSIS_ACTIVE=1");
		if (buses.vehicle) flags.push("-DBUS_VEHICLE_ACTIVE=1");
		if (buses.body) flags.push("-DBUS_BODY_ACTIVE=1");
		return flags.join(" ");
	}, [buses]);

	const toggleConnection = (key: string) => {
		if (key === "serial") {
			return;
		}
		setConnection((current) => ({ ...current, [key]: current[key] ? 0 : 1 }));
	};

	const toggleBus = (key: BusKey) => {
		if (key === "chassis") {
			return;
		}
		setBuses((current) => ({ ...current, [key]: current[key] ? 0 : 1 }));
	};

	const handleBuild = async () => {
		setStatus("building");
		setMessage("Resolving latest GitHub release asset...");

		try {
			const filename = await downloadReleaseBinary(environment, buses);
			setStatus("done");
			setMessage(`${filename} downloaded from the latest GitHub release.`);
		} catch (error) {
			setStatus("error");
			setMessage(error instanceof Error ? error.message : "Release download failed.");
		}
	};

	const handleFlash = async () => {
		setFlashLog([]);
		setStatus("connecting");
		setMessage("Preparing dedicated flasher session...");

		const addLog = (line: string) => {
			setFlashLog((current) => [...current, line].slice(-80));
		};

		const sendFlashCommand = async (): Promise<void> => {
			const flashCommand = JSON.stringify({ cmd: "flash", env: environment });
			const result = await sharedConnection.controller.runRawCommand(flashCommand);
			if (!result.ok) {
				throw new Error(result.error ?? "Flash command failed.");
			}

			if (result.responseText?.trim()) {
				addLog(`[board] ${result.responseText.trim()}`);
			}
		};

		try {
			const activeTransport = sharedConnection.controller.activeTransportType;
			if (!isSerialFamilyTransport(activeTransport)) {
				throw new Error(
					"Shared USB serial must already be connected. Open shared connection first, then start flash.",
				);
			}
			addLog("> Reusing active shared USB serial connection");
			addLog("> Pausing shared frame feed for dedicated flashing...");
			sharedConnection.pauseFrameFeed(true);

			setStatus("flashing");
			setMessage(`Flashing ${environment}...`);
			addLog(`> Flash started: ${environment}`);
			await sendFlashCommand();

			addLog("✓ Flash completed successfully");
			setStatus("done");
			setMessage(`${environment} flashed successfully.`);
		} catch (error) {
			const errorMsg = error instanceof Error ? error.message : "Flash failed.";
			addLog(`✗ Error: ${errorMsg}`);
			setStatus("error");
			setMessage(errorMsg);
		} finally {
			sharedConnection.pauseFrameFeed(false);
			addLog("> Flasher session released; shared feed resumed");
		}
	};

	return (
		<ScrollView style={styles.screen} contentContainerStyle={styles.content}>
			<View style={styles.hero}>
				<Text style={styles.kicker}>FIRMWARE TOOLCHAIN</Text>
				<Text style={styles.title}>Flasher Workspace</Text>
				<Text style={styles.subtitle}>
					Select connectivity and CAN bus profile, then download the matching GitHub
					Actions release binary or flash over USB.
				</Text>
			</View>

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>Target Board</Text>
				<View style={styles.segmentRow}>
					<View style={[styles.segment, styles.segmentActive]}>
						<Text style={[styles.segmentTitle, styles.segmentTitleActive]}>
							ESP32-S DevKit
						</Text>
						<Text style={styles.segmentDetail}>MCP2515 x1-3 · WiFi · BLE · NVS</Text>
					</View>
				</View>
				<Text style={styles.resolvedLine}>Latest release asset: {assetName}</Text>
			</View>

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>Connectivity</Text>
				<View style={styles.chipGrid}>
					{CONNECTION_ESP32.map((option) => {
						const active = Boolean(connection[option.key]);
						return (
							<Pressable
								key={option.key}
								onPress={() => toggleConnection(option.key)}
								style={[
									styles.chip,
									active ? styles.chipActive : undefined,
									option.locked ? styles.chipLocked : undefined,
								]}
							>
								<Text
									style={[
										styles.chipLabel,
										active ? styles.chipLabelActive : undefined,
									]}
								>
									{option.label}
								</Text>
								<Text style={styles.chipDetail}>{option.desc}</Text>
								<Text
									style={[
										styles.chipStatus,
										active ? styles.chipStatusActive : undefined,
									]}
								>
									{active ? "ON" : "OFF"}
								</Text>
							</Pressable>
						);
					})}
				</View>
				<Text style={styles.resolvedLine}>
					Resolved firmware environment: {environment}
				</Text>
			</View>

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>CAN Buses</Text>
				<View style={styles.chipGrid}>
					{BUS_OPTIONS.map((bus) => {
						const active = Boolean(buses[bus.key]);
						return (
							<Pressable
								key={bus.key}
								onPress={() => toggleBus(bus.key)}
								style={[
									styles.chip,
									active ? styles.chipActive : undefined,
									bus.key === "chassis" ? styles.chipLocked : undefined,
								]}
							>
								<Text
									style={[
										styles.chipLabel,
										active ? styles.chipLabelActive : undefined,
									]}
								>
									{bus.label}
								</Text>
								<Text style={styles.chipDetail}>{bus.desc}</Text>
								<Text
									style={[
										styles.chipStatus,
										active ? styles.chipStatusActive : undefined,
									]}
								>
									{active ? "ON" : "OFF"}
								</Text>
							</Pressable>
						);
					})}
				</View>
			</View>

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>Build and Flash</Text>
				<Text style={styles.summaryLine}>
					{["Chassis", buses.vehicle ? "Vehicle" : null, buses.body ? "Body" : null]
						.filter(Boolean)
						.join(" + ")}
				</Text>
				<Text style={styles.resolvedLine}>GitHub Release asset: {assetName}</Text>
				<View style={styles.actionsRow}>
					<Pressable
						onPress={() => void handleBuild()}
						disabled={status === "building"}
						style={[
							styles.action,
							styles.primaryAction,
							status === "building" ? styles.actionDisabled : undefined,
						]}
					>
						<Text style={styles.actionText}>
							{status === "building" ? "Resolving..." : "Download Release Binary"}
						</Text>
					</Pressable>
					<Pressable
						onPress={() => void handleFlash()}
						disabled={
							status === "building" ||
							status === "connecting" ||
							status === "flashing"
						}
						style={[
							styles.action,
							styles.secondaryAction,
							status === "building" ||
							status === "connecting" ||
							status === "flashing"
								? styles.actionDisabled
								: undefined,
						]}
					>
						<Text style={styles.secondaryActionText}>
							{status === "connecting"
								? "Connecting..."
								: status === "flashing"
									? "Flashing..."
									: "Flash via USB"}
						</Text>
					</Pressable>
				</View>
				{message ? (
					<Text
						style={[
							styles.message,
							status === "error"
								? styles.messageError
								: status === "done"
									? styles.messageDone
									: undefined,
						]}
					>
						{message}
					</Text>
				) : null}
			</View>

			{flashLog.length > 0 ? (
				<View style={styles.panel}>
					<Text style={styles.panelTitle}>Flash Progress</Text>
					<ScrollView
						style={styles.flashConsole}
						contentContainerStyle={styles.flashConsoleContent}
						showsVerticalScrollIndicator
						nestedScrollEnabled
					>
						{flashLog.map((line, idx) => (
							<Text key={idx} style={styles.flashConsoleText}>
								{line}
							</Text>
						))}
					</ScrollView>
				</View>
			) : null}

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>CLI Reference</Text>
				<View style={styles.codeBlock}>
					<Text style={styles.codeText}>cd firmware</Text>
					<Text style={styles.codeText}>
						PLATFORMIO_BUILD_FLAGS="{buildFlags}" pio run -e {environment}
					</Text>
					<Text style={styles.codeText}>pio run -e {environment} -t upload</Text>
				</View>
			</View>
		</ScrollView>
	);
}

const styles = StyleSheet.create({
	screen: {
		flex: 1,
		backgroundColor: colors.dashBackground,
	},
	content: {
		padding: spacing.lg,
		gap: spacing.md3,
		paddingBottom: spacing.xl2,
	},
	hero: {
		borderRadius: radius.xl,
		padding: spacing.lg,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.dashCard,
		gap: spacing.sm2,
	},
	kicker: {
		color: colors.dashPrimary,
		letterSpacing: 1,
		fontSize: font.size.sm2,
		fontWeight: font.weight.bold,
	},
	title: {
		color: colors.dashValue,
		fontSize: font.size.xl3,
		fontWeight: font.weight.extrabold,
	},
	subtitle: {
		color: colors.dashLabel,
		fontSize: font.size.md,
		lineHeight: 20,
	},
	panel: {
		borderRadius: radius.lg2,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.backgroundDarkSubtle,
		padding: spacing.md,
		gap: spacing.md2,
	},
	panelTitle: {
		color: colors.dashValue,
		fontSize: font.size.lg,
		fontWeight: font.weight.bold,
	},
	segmentRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	segment: {
		minWidth: 220,
		flexGrow: 1,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: radius.md2,
		padding: spacing.md2,
		backgroundColor: colors.dashBackground,
		gap: 3,
	},
	segmentActive: {
		borderColor: colors.dashPrimary,
		backgroundColor: colors.backgroundDarkSubtle,
	},
	segmentTitle: {
		color: colors.dashValue,
		fontWeight: font.weight.bold,
		fontSize: font.size.md2,
	},
	segmentTitleActive: {
		color: colors.dashPrimary,
	},
	segmentDetail: {
		color: colors.dashLabel,
		fontSize: font.size.sm,
	},
	chipGrid: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	chip: {
		minWidth: 150,
		flexGrow: 1,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		borderRadius: radius.md2,
		padding: spacing.md2,
		backgroundColor: colors.dashBackground,
		gap: 3,
	},
	chipActive: {
		borderColor: colors.dashPrimary,
		backgroundColor: colors.backgroundDarkSubtle,
	},
	chipLocked: {
		opacity: 0.9,
	},
	chipLabel: {
		color: colors.dashValue,
		fontWeight: font.weight.bold,
		fontSize: font.size.sm,
	},
	chipLabelActive: {
		color: colors.dashPrimary,
	},
	chipDetail: {
		color: colors.dashLabel,
		fontSize: font.size.sm2,
	},
	chipStatus: {
		color: colors.dashMuted,
		fontSize: font.size.sm2,
		fontWeight: font.weight.bold,
	},
	chipStatusActive: {
		color: colors.dashPrimary,
	},
	resolvedLine: {
		color: colors.dashLabel,
		fontSize: font.size.sm,
	},
	summaryLine: {
		color: colors.dashLabel,
		fontSize: font.size.md2,
	},
	actionsRow: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: 8,
	},
	action: {
		minWidth: 180,
		flexGrow: 1,
		borderRadius: radius.md,
		paddingVertical: spacing.md2,
		paddingHorizontal: spacing.md,
		alignItems: "center",
	},
	primaryAction: {
		backgroundColor: colors.dashPrimary,
	},
	secondaryAction: {
		backgroundColor: colors.dashBackground,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
	},
	actionDisabled: {
		opacity: 0.65,
	},
	actionText: {
		color: colors.backgroundDark,
		fontWeight: font.weight.extrabold,
		fontSize: font.size.md2,
	},
	secondaryActionText: {
		color: colors.dashValue,
		fontWeight: font.weight.bold,
		fontSize: font.size.md2,
	},
	message: {
		fontSize: font.size.sm,
		color: colors.dashLabel,
	},
	messageDone: {
		color: colors.statusConnected,
	},
	messageError: {
		color: colors.destructive,
	},
	flashConsole: {
		borderRadius: radius.md2,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.dashBackground,
		maxHeight: 300,
	},
	flashConsoleContent: {
		padding: spacing.md2,
		gap: 4,
	},
	flashConsoleText: {
		color: colors.dashValue,
		fontFamily: "Courier",
		fontSize: font.size.sm2,
		lineHeight: 16,
	},
	codeBlock: {
		borderRadius: radius.md2,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		backgroundColor: colors.dashBackground,
		padding: spacing.md2,
		gap: 5,
	},
	codeText: {
		color: colors.dashValue,
		fontFamily: "Courier",
		fontSize: font.size.sm,
	},
});
