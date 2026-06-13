import { Pressable, Text, View } from "react-native";
import type { AppTabRoute } from "../state/appRoute";

interface MenuHeaderProps {
	tabs: Array<{ id: AppTabRoute; label: string }>;
	activeTab: AppTabRoute;
	onSelectTab: (tab: AppTabRoute) => void;
}

export function MenuHeader({ tabs, activeTab, onSelectTab }: MenuHeaderProps) {
	return (
		<View className="border-t border-border bg-background px-4 py-2">
			<View className="flex-row items-center gap-2 bg-card border border-border rounded-xl p-1">
				{tabs.map((tab) => {
					const active = activeTab === tab.id;
					return (
						<Pressable
							key={tab.id}
							onPress={() => onSelectTab(tab.id)}
							className={`flex-1 items-center justify-center rounded-lg px-3 min-h-[38px] ${
								active ? "bg-primary/15 border border-primary" : "bg-transparent"
							}`}
						>
							<Text
								className={`text-[13px] font-semibold ${
									active ? "text-primary font-bold" : "text-muted-foreground"
								}`}
							>
								{tab.label}
							</Text>
						</Pressable>
					);
				})}
			</View>
		</View>
	);
}

export default MenuHeader;
