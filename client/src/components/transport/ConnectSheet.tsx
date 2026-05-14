import { Text, TextInput, View, Pressable } from "react-native";
import type { MonitorTransportType } from "../../hardware/transportPresentation";
import type { ConnectionPreset } from "../../state/TransportContext";

export interface ConnectSheetProps {
	transportType: MonitorTransportType;
	baseUrl: string;
	commandPath: string;
	statusPath: string;
	connectionBusy: boolean;
	lastResult: string;
	onChangeBaseUrl: (v: string) => void;
	onChangeCommandPath: (v: string) => void;
	onChangeStatusPath: (v: string) => void;
	onApplyPreset?: (preset: ConnectionPreset) => void;
	onConnect: () => void;
	presets?: ConnectionPreset[];
}

function normalize(type: MonitorTransportType): MonitorTransportType {
	return type === "bluetooth-serial" ? "serial" : type;
}

export function ConnectSheetContent({
	transportType,
	baseUrl,
	commandPath,
	statusPath,
	connectionBusy,
	lastResult,
	onChangeBaseUrl,
	onChangeCommandPath,
	onChangeStatusPath,
	onApplyPreset,
	onConnect,
	presets,
}: ConnectSheetProps) {
	const visual = normalize(transportType);

	return (
		<View className="gap-3">
			{visual === "http" ? (
				<>
					<View className="gap-1">
						<Text className="text-xs font-bold uppercase text-muted-foreground">
							Base URL
						</Text>
						<TextInput
							className="bg-background rounded-md border border-border px-3 py-2 text-sm text-foreground"
							value={baseUrl}
							onChangeText={onChangeBaseUrl}
							placeholder="http://192.168.4.1"
							placeholderTextColor="#5c7ea0"
							autoCapitalize="none"
							autoCorrect={false}
						/>
					</View>

					<View className="gap-1">
						<Text className="text-xs font-bold uppercase text-muted-foreground">
							Command Path
						</Text>
						<TextInput
							className="bg-background rounded-md border border-border px-3 py-2 text-sm text-foreground"
							value={commandPath}
							onChangeText={onChangeCommandPath}
							placeholder="/api/command"
							placeholderTextColor="#5c7ea0"
							autoCapitalize="none"
							autoCorrect={false}
						/>
					</View>

					<View className="gap-1">
						<Text className="text-xs font-bold uppercase text-muted-foreground">
							Status Path
						</Text>
						<TextInput
							className="bg-background rounded-md border border-border px-3 py-2 text-sm text-foreground"
							value={statusPath}
							onChangeText={onChangeStatusPath}
							placeholder="/api/status"
							placeholderTextColor="#5c7ea0"
							autoCapitalize="none"
							autoCorrect={false}
						/>
					</View>

					{presets && presets.length > 0 ? (
						<>
							<Text className="text-xs font-bold uppercase text-muted-foreground">
								Quick Presets
							</Text>
							<View className="flex-row flex-wrap gap-2">
								{presets.map((preset) => (
									<Pressable
										key={preset.name}
										onPress={() => onApplyPreset?.(preset)}
										className="px-3 py-2 rounded-full border border-border bg-background"
									>
										<Text className="text-sm font-medium text-foreground">
											{preset.name}
										</Text>
									</Pressable>
								))}
							</View>
						</>
					) : null}
				</>
			) : null}

			{visual === "serial" ? (
				<View className="rounded-xl border border-border bg-background p-3 gap-1">
					<Text className="text-sm font-bold text-foreground">Serial / COM</Text>
					<Text className="text-sm text-muted-foreground leading-4">
						Uses the browser serial picker. This covers direct USB serial and Bluetooth
						COM ports exposed by the OS.
					</Text>
				</View>
			) : null}

			{visual === "ble" ? (
				<View className="rounded-xl border border-border bg-background p-3 gap-1">
					<Text className="text-sm font-bold text-foreground">BLE (NUS)</Text>
					<Text className="text-sm text-muted-foreground leading-4">
						Uses Web Bluetooth and the Nordic UART Service UUIDs documented in the BLE
						adapter reference.
					</Text>
				</View>
			) : null}

			<View className="flex-row flex-wrap gap-2">
				<Pressable
					className={`flex-1 min-w-[200px] items-center rounded-md bg-primary px-3 py-3 ${
						connectionBusy ? "opacity-60" : ""
					}`}
					onPress={onConnect}
					disabled={connectionBusy}
				>
					<Text className="text-sm font-bold text-primary-foreground">
						{connectionBusy
							? "Connecting..."
							: visual === "http"
								? "Apply Connection"
								: "Open Picker and Connect"}
					</Text>
				</Pressable>
			</View>

			{lastResult ? (
				<Text className="text-xs text-muted-foreground leading-4" numberOfLines={3}>
					{lastResult}
				</Text>
			) : null}
		</View>
	);
}
