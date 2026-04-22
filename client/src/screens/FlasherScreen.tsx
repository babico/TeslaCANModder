import { useMemo, useState } from "react";
import { Linking, Platform, Pressable, ScrollView, StyleSheet, Text, View } from "react-native";

import { colors, font, radius, spacing } from "../design/tokens";
import { flashMergedEspReleaseImage, supportsBrowserEspFlash } from "../hardware/webEspFlasher";
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
	{ key: "chassis", label: "Chassis", desc: "X179 pins 13-14" },
	{ key: "vehicle", label: "Vehicle", desc: "X179 pins 9-10" },
	{ key: "body", label: "Body", desc: "X179 pins 2-3" },
] as const;

type BusKey = (typeof BUS_OPTIONS)[number]["key"];

type ReleaseAsset = {
	name: string;
	browser_download_url: string;
};

type ResolvedReleaseAsset = ReleaseAsset & {
	tag_name?: string;
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
	const enabled = BUS_OPTIONS.filter((bus) => buses[bus.key]).map((bus) => bus.key);
	if (enabled.length === 0) {
		return `${env}_no_can.bin`;
	}
	if (buses.chassis) {
		const nonChassis = enabled.filter((key) => key !== "chassis");
		return nonChassis.length === 0 ? `${env}.bin` : `${env}_${nonChassis.join("_")}.bin`;
	}
	return `${env}_${enabled.join("_")}_only.bin`;
}

async function resolveReleaseAsset(
	env: string,
	buses: Record<BusKey, number>,
): Promise<ResolvedReleaseAsset> {
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

	return {
		...asset,
		tag_name: release.tag_name,
	};
}

async function downloadReleaseBinary(env: string, buses: Record<BusKey, number>): Promise<string> {
	const asset = await resolveReleaseAsset(env, buses);

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
	const [buses, setBuses] = useState<Record<BusKey, number>>({
		chassis: 1,
		vehicle: 0,
		body: 0,
	});
	const [status, setStatus] = useState<StatusType>("idle");
	const [message, setMessage] = useState("");
	const [flashLog, setFlashLog] = useState<string[]>([]);

	const environment = useMemo(() => resolveEnvironment(connection), [connection]);
	const assetName = useMemo(() => resolveAssetName(environment, buses), [environment, buses]);
	const busSummary = useMemo(() => {
		const enabledLabels = BUS_OPTIONS.filter((bus) => buses[bus.key]).map((bus) => bus.label);
		return enabledLabels.length > 0 ? enabledLabels.join(" + ") : "No CAN lanes enabled";
	}, [buses]);
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
		setMessage("Preparing browser flasher session...");

		const addLog = (line: string) => {
			setFlashLog((current) => [...current, line].slice(-80));
		};

		let lastLoggedProgress = -1;

		try {
			if (Platform.OS !== "web" || !supportsBrowserEspFlash()) {
				throw new Error(
					"USB flashing requires the web client in Chrome or Edge with Web Serial enabled.",
				);
			}

			const activeTransport = sharedConnection.controller.activeTransportType;
			if (isSerialFamilyTransport(activeTransport)) {
				addLog("> Closing shared serial session before flashing...");
				await sharedConnection.disconnectTransport();
			}

			addLog("> Resolving latest merged release image...");
			const releaseAsset = await resolveReleaseAsset(environment, buses);
			addLog(
				releaseAsset.tag_name
					? `> Release asset: ${releaseAsset.name} from ${releaseAsset.tag_name}`
					: `> Release asset: ${releaseAsset.name}`,
			);
			addLog("> Pausing shared frame feed while Web Serial flashing is active...");
			sharedConnection.pauseFrameFeed(true);

			setStatus("flashing");
			setMessage(`Flashing ${releaseAsset.name} over Web Serial...`);
			addLog(`> Flash started: ${releaseAsset.name}`);

			await flashMergedEspReleaseImage({
				assetName: releaseAsset.name,
				assetUrl: releaseAsset.browser_download_url,
				onLog(line) {
					addLog(`[flasher] ${line}`);
				},
				onProgress(written, total) {
					if (total <= 0) {
						return;
					}

					const percent = Math.floor((written / total) * 100);
					if (percent === lastLoggedProgress || (percent % 10 !== 0 && percent !== 100)) {
						return;
					}

					lastLoggedProgress = percent;
					addLog(`> Progress: ${percent}% (${written}/${total} bytes)`);
				},
			});

			addLog("> Flash completed successfully");
			addLog("> Reconnect from Monitor after reboot to verify status output");
			setStatus("done");
			setMessage(`${releaseAsset.name} flashed successfully over Web Serial.`);
		} catch (error) {
			const errorMessage = error instanceof Error ? error.message : "Flash failed.";
			addLog(`> Error: ${errorMessage}`);
			setStatus("error");
			setMessage(errorMessage);
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
					release image or flash it directly over Web Serial.
				</Text>
			</View>

			<View style={styles.panel}>
				<Text style={styles.panelTitle}>Target Board</Text>
				<View style={styles.segmentRow}>
					<View style={[styles.segment, styles.segmentActive]}>
						<Text style={[styles.segmentTitle, styles.segmentTitleActive]}>
							ESP32-S DevKit
						</Text>
						<Text style={styles.segmentDetail}>MCP2515 x1-3 / WiFi / BLE / NVS</Text>
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
								style={[styles.chip, active ? styles.chipActive : undefined]}
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
				<Text style={styles.summaryLine}>{busSummary}</Text>
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
									: "Flash via Web Serial"}
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
						{flashLog.map((line, index) => (
							<Text key={index} style={styles.flashConsoleText}>
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
					<Text style={styles.codeText}>
						node ../tools/debug.js flash --port COM5 --hex build/firmware/{environment}
						.bin
					</Text>
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
