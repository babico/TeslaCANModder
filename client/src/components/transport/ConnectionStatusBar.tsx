import { Pressable, Text, View } from "react-native";
import { ThemeSwitch } from "../ThemeSwitch";

export interface ConnectionStatusBarProps {
	isReady: boolean;
	isBusy: boolean;
	transportLabel: string;
	transportHint: string;
	onOpenSheet: () => void;
}

const CONNECTED_COLOR = "#22c55e";
const CONNECTING_COLOR = "#f59e0b";
const DISCONNECTED_COLOR = "#94a3b8";

export function ConnectionStatusBar({
	isReady,
	isBusy,
	transportLabel,
	transportHint,
	onOpenSheet,
}: ConnectionStatusBarProps) {
	const statusColor = isBusy ? CONNECTING_COLOR : isReady ? CONNECTED_COLOR : DISCONNECTED_COLOR;
	const statusLabel = isBusy ? "Connecting" : isReady ? "Connected" : "Disconnected";

	return (
		<View className="flex-row items-center bg-card border-b border-border px-4 py-3 gap-3">
			<View className="min-w-[118px] gap-0.5">
				<Text className="text-sm font-bold text-card-foreground tracking-wide">
					Tesla CAN Modder
				</Text>
				<Text className="text-xs text-muted-foreground">Shared board connection</Text>
			</View>

			<View className="flex-1 gap-0.5">
				<Text className="text-xs font-bold text-muted-foreground uppercase">
					{transportLabel}
				</Text>
				<Text className="text-sm text-card-foreground" numberOfLines={1}>
					{transportHint}
				</Text>
			</View>

			<Pressable
				onPress={onOpenSheet}
				className="flex-row items-center gap-2 px-3 py-2 bg-background rounded-full border border-border"
			>
				<View className="w-2 h-2 rounded-full" style={{ backgroundColor: statusColor }} />
				<Text className="text-sm font-semibold" style={{ color: statusColor }}>
					{statusLabel}
				</Text>
			</Pressable>

			<ThemeSwitch />
		</View>
	);
}
