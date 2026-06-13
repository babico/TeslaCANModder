import { Pressable, Text, View } from "react-native";
import { selectDashColors } from "../../design/tokens";
import { useThemeState } from "../../state/ThemeContext";

export type MonitorSectionTab = "console" | "connection" | "diagnostics";

interface NavItem {
	tab: MonitorSectionTab;
	label: string;
	icon: string;
}

interface MonitorNavigationProps {
	items: NavItem[];
	activeSection: MonitorSectionTab;
	onChangeSection: (tab: MonitorSectionTab) => void;
	frameCount: number;
	canConnected: boolean;
}

export function MonitorSidebarNavigation({
	items,
	activeSection,
	onChangeSection,
	frameCount,
	canConnected,
}: MonitorNavigationProps) {
	const { isDark: _isDark } = useThemeState();
	const _colors = selectDashColors(_isDark);
	return (
		<View className="w-[200px] bg-card border-r border-border flex-col">
			<View className="px-5 pt-6 pb-4 border-b border-border gap-0.5">
				<Text className="text-lg font-bold text-card-foreground tracking-wide">
					Console
				</Text>
				<Text className="text-xs text-muted-foreground tracking-wide">CAN Workspace</Text>
			</View>
			<View className="flex-1 py-2 gap-0.5">
				{items.map((item) => (
					<Pressable
						key={item.tab}
						onPress={() => onChangeSection(item.tab)}
						className={`flex-row items-center gap-2.5 px-4 py-2.5 mx-2 rounded-md ${activeSection === item.tab ? "bg-muted" : ""}`}
					>
						<Text
							className={`text-sm ${activeSection === item.tab ? "text-primary" : "text-muted-foreground"}`}
						>
							{item.icon}
						</Text>
						<Text
							className={`text-[13px] font-medium ${activeSection === item.tab ? "text-primary font-bold" : "text-muted-foreground"}`}
						>
							{item.label}
						</Text>
					</Pressable>
				))}
			</View>
			<View className="px-5 py-4 border-t border-border">
				<Text className="text-xs text-muted-foreground">
					{frameCount} frames · {canConnected ? "CAN OK" : "No CAN"}
				</Text>
			</View>
		</View>
	);
}

interface MonitorBottomBarProps {
	items: NavItem[];
	activeSection: MonitorSectionTab;
	onChangeSection: (tab: MonitorSectionTab) => void;
}

export function MonitorBottomBar({ items, activeSection, onChangeSection }: MonitorBottomBarProps) {
	const { isDark: _isDark } = useThemeState();
	const _colors = selectDashColors(_isDark);
	return (
		<View className="flex-row bg-card border-t border-border">
			{items.map((item) => (
				<Pressable
					key={item.tab}
					onPress={() => onChangeSection(item.tab)}
					className={`flex-1 py-2.5 items-center gap-0.5 ${activeSection === item.tab ? "border-t-2 border-primary" : ""}`}
				>
					<Text
						className={`text-base ${activeSection === item.tab ? "text-primary" : "text-muted-foreground"}`}
					>
						{item.icon}
					</Text>
					<Text
						className={`text-[10px] font-medium ${activeSection === item.tab ? "text-primary" : "text-muted-foreground"}`}
					>
						{item.label}
					</Text>
				</Pressable>
			))}
		</View>
	);
}
