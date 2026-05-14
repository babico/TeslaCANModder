import { useMemo, useState } from "react";
import { Linking, Platform, Pressable, ScrollView, Text, View } from "react-native";

import { flashMergedEspReleaseImage, supportsBrowserEspFlash } from "../hardware/webEspFlasher";
import { useBoardConnection } from "../state/BoardConnectionContext";
import { Card, CardHeader, CardTitle, CardContent } from "../ui/shadcn/card";
import { Button } from "../ui/shadcn/button";

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
	{ key: "chassis", label: "Chassis", desc: "X179 pins 13-14 \u00B7 DAS injection" },
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

function resolveEnvironment(
	connection: Record<string, number>,
	buses: Record<BusKey, number>,
	clock: 8 | 16,
): string {
	const parts: string[] = ["esp32"];
	if (connection.wifi) parts.push("wifi");
	if (connection.ble) parts.push("ble");
	if (buses.chassis) parts.push("chassis");
	if (buses.vehicle) parts.push("vehicle");
	if (buses.body) parts.push("body");
	parts.push(`${clock}mhz`);
	return parts.join("_");
}

function resolveAssetName(env: string): string {
	return `${env}.bin`;
}

async function resolveReleaseAsset(env: string): Promise<ResolvedReleaseAsset> {
	const assetName = resolveAssetName(env);
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

async function downloadReleaseBinary(env: string): Promise<string> {
	const asset = await resolveReleaseAsset(env);

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
	const [clock, setClock] = useState<8 | 16>(8);
	const [status, setStatus] = useState<StatusType>("idle");
	const [message, setMessage] = useState("");
	const [flashLog, setFlashLog] = useState<string[]>([]);

	const environment = useMemo(
		() => resolveEnvironment(connection, buses, clock),
		[connection, buses, clock],
	);
	const assetName = useMemo(() => resolveAssetName(environment), [environment]);
	const busSummary = useMemo(() => {
		const enabledLabels = BUS_OPTIONS.filter((bus) => buses[bus.key]).map((bus) => bus.label);
		return enabledLabels.length > 0 ? enabledLabels.join(" + ") : "No CAN lanes enabled";
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
			const filename = await downloadReleaseBinary(environment);
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
			const releaseAsset = await resolveReleaseAsset(environment);
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
		<ScrollView
			className="flex-1 bg-background"
			contentContainerStyle={{ padding: 16, gap: 14, paddingBottom: 28 }}
		>
			<Card>
				<CardHeader>
					<Text className="text-xs font-bold uppercase text-primary tracking-wide">
						FIRMWARE TOOLCHAIN
					</Text>
					<CardTitle>Flasher Workspace</CardTitle>
				</CardHeader>
				<CardContent>
					<Text className="text-sm text-muted-foreground leading-5">
						Select connectivity and CAN bus profile, then download the matching GitHub
						release image or flash it directly over Web Serial.
					</Text>
				</CardContent>
			</Card>

			<Card>
				<CardHeader>
					<CardTitle>Target Board</CardTitle>
				</CardHeader>
				<CardContent className="gap-3">
					<View className="flex-row flex-wrap gap-2">
						<View className="min-w-[220px] flex-1 border border-primary bg-muted rounded-lg p-3 gap-0.5">
							<Text className="text-sm font-bold text-primary">ESP32-S DevKit</Text>
							<Text className="text-xs text-muted-foreground">
								MCP2515 x1-4 / WiFi / BLE / NVS
							</Text>
						</View>
					</View>
					<Text className="text-xs text-muted-foreground">
						Latest release asset: {assetName}
					</Text>
				</CardContent>
			</Card>

			<Card>
				<CardHeader>
					<CardTitle>Connectivity</CardTitle>
				</CardHeader>
				<CardContent className="gap-3">
					<View className="flex-row flex-wrap gap-2">
						{CONNECTION_ESP32.map((option) => {
							const active = Boolean(connection[option.key]);
							return (
								<Pressable
									key={option.key}
									onPress={() => toggleConnection(option.key)}
									className={`min-w-[150px] flex-1 border rounded-lg p-3 gap-0.5 ${
										active
											? "border-primary bg-muted"
											: "border-border bg-background"
									} ${option.locked ? "opacity-90" : ""}`}
								>
									<Text
										className={`text-xs font-bold ${active ? "text-primary" : "text-foreground"}`}
									>
										{option.label}
									</Text>
									<Text className="text-xs text-muted-foreground">
										{option.desc}
									</Text>
									<Text
										className={`text-xs font-bold ${active ? "text-primary" : "text-muted"}`}
									>
										{active ? "ON" : "OFF"}
									</Text>
								</Pressable>
							);
						})}
					</View>
					<Text className="text-xs text-muted-foreground">
						Resolved firmware environment: {environment}
					</Text>
				</CardContent>
			</Card>

			<Card>
				<CardHeader>
					<CardTitle>CAN Buses</CardTitle>
				</CardHeader>
				<CardContent className="gap-3">
					<View className="flex-row flex-wrap gap-2">
						{BUS_OPTIONS.map((bus) => {
							const active = Boolean(buses[bus.key]);
							return (
								<Pressable
									key={bus.key}
									onPress={() => toggleBus(bus.key)}
									className={`min-w-[150px] flex-1 border rounded-lg p-3 gap-0.5 ${
										active
											? "border-primary bg-muted"
											: "border-border bg-background"
									}`}
								>
									<Text
										className={`text-xs font-bold ${active ? "text-primary" : "text-foreground"}`}
									>
										{bus.label}
									</Text>
									<Text className="text-xs text-muted-foreground">
										{bus.desc}
									</Text>
									<Text
										className={`text-xs font-bold ${active ? "text-primary" : "text-muted"}`}
									>
										{active ? "ON" : "OFF"}
									</Text>
								</Pressable>
							);
						})}
					</View>
					{!buses.chassis ? (
						<Text className="text-xs text-warning">
							{"\u26A0"} Chassis bus off {"\u2014"} passive sniffer mode, DAS
							injection disabled
						</Text>
					) : null}
				</CardContent>
			</Card>

			<Card>
				<CardHeader>
					<CardTitle>MCP2515 Crystal Clock</CardTitle>
				</CardHeader>
				<CardContent>
					<View className="flex-row flex-wrap gap-2">
						{([8, 16] as const).map((mhz) => (
							<Pressable
								key={mhz}
								onPress={() => setClock(mhz)}
								className={`min-w-[220px] flex-1 border rounded-lg p-3 gap-0.5 ${
									clock === mhz
										? "border-primary bg-muted"
										: "border-border bg-background"
								}`}
							>
								<Text
									className={`text-sm font-bold ${clock === mhz ? "text-primary" : "text-foreground"}`}
								>
									{mhz} MHz
								</Text>
								<Text className="text-xs text-muted-foreground">
									{mhz === 8 ? "Most modules" : "Some modules"}
								</Text>
							</Pressable>
						))}
					</View>
				</CardContent>
			</Card>

			<Card>
				<CardHeader>
					<CardTitle>Build and Flash</CardTitle>
				</CardHeader>
				<CardContent className="gap-3">
					<Text className="text-sm text-muted-foreground">{busSummary}</Text>
					<Text className="text-xs text-muted-foreground">
						GitHub Release asset: {assetName}
					</Text>
					<View className="flex-row flex-wrap gap-2">
						<Button
							label={
								status === "building" ? "Resolving..." : "Download Release Binary"
							}
							onPress={() => void handleBuild()}
							disabled={status === "building"}
						/>
						<Button
							label={
								status === "connecting"
									? "Connecting..."
									: status === "flashing"
										? "Flashing..."
										: "Flash via Web Serial"
							}
							variant="outline"
							onPress={() => void handleFlash()}
							disabled={
								status === "building" ||
								status === "connecting" ||
								status === "flashing"
							}
						/>
					</View>
					{message ? (
						<Text
							className={`text-xs ${
								status === "error"
									? "text-destructive"
									: status === "done"
										? "text-green-500"
										: "text-muted-foreground"
							}`}
						>
							{message}
						</Text>
					) : null}
				</CardContent>
			</Card>

			{flashLog.length > 0 ? (
				<Card>
					<CardHeader>
						<CardTitle>Flash Progress</CardTitle>
					</CardHeader>
					<CardContent>
						<ScrollView
							className="rounded-lg border border-border bg-background max-h-[300px]"
							contentContainerStyle={{ padding: 10, gap: 4 }}
							showsVerticalScrollIndicator
							nestedScrollEnabled
						>
							{flashLog.map((line, index) => (
								<Text key={index} className="text-xs text-foreground leading-4">
									{line}
								</Text>
							))}
						</ScrollView>
					</CardContent>
				</Card>
			) : null}

			<Card>
				<CardHeader>
					<CardTitle>CLI Reference</CardTitle>
				</CardHeader>
				<CardContent>
					<View className="rounded-lg border border-border bg-background p-3 gap-1">
						<Text className="text-xs text-foreground">cd firmware</Text>
						<Text className="text-xs text-foreground">
							.\pio.ps1 run -e {environment}
						</Text>
						<Text className="text-xs text-foreground">
							node ../tools/debug.js flash --port COM5 --hex build/firmware/
							{environment}.bin
						</Text>
					</View>
				</CardContent>
			</Card>
		</ScrollView>
	);
}
