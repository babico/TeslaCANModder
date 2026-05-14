import { useState } from "react";
import { ScrollView, View, useWindowDimensions } from "react-native";

import {
	MonitorBottomBar,
	MonitorSidebarNavigation,
	type MonitorSectionTab,
} from "../components/monitor/MonitorNavigation";
import { ConnectionSection } from "../components/monitor/sections/ConnectionSection";
import { DiagnosticsSection } from "../components/monitor/sections/DiagnosticsSection";
import { MonitorSection } from "../components/monitor/sections/MonitorSection";
import type { MonitorScreenProps } from "../components/monitor/sections/types";

export function ConsoleScreen(props: MonitorScreenProps) {
	const { width } = useWindowDimensions();
	const isWide = width >= 768;
	const [activeSection, setActiveSection] = useState<MonitorSectionTab>("console");

	const navItems = [
		{ tab: "console" as const, label: "CAN Monitor", icon: "\u25C9" },
		{ tab: "connection" as const, label: "Connection", icon: "\u25CC" },
		{ tab: "diagnostics" as const, label: "Events", icon: "\u25C8" },
	];

	if (isWide) {
		return (
			<View className="flex-1 flex-row bg-background">
				<MonitorSidebarNavigation
					items={navItems}
					activeSection={activeSection}
					onChangeSection={setActiveSection}
					frameCount={props.frameCount}
					canConnected={Object.keys(props.boardState.canHealth ?? {}).length > 0}
				/>

				<ScrollView
					className="flex-1 bg-background"
					contentContainerStyle={{ padding: 20, paddingBottom: 48, gap: 16 }}
				>
					{activeSection === "console" && <MonitorSection {...props} />}
					{activeSection === "connection" && <ConnectionSection {...props} />}
					{activeSection === "diagnostics" && <DiagnosticsSection {...props} />}
				</ScrollView>
			</View>
		);
	}

	return (
		<View className="flex-1 flex-col bg-background">
			<ScrollView
				className="flex-1 bg-background"
				contentContainerStyle={{ padding: 20, paddingBottom: 48, gap: 16 }}
			>
				{activeSection === "console" && <MonitorSection {...props} />}
				{activeSection === "connection" && <ConnectionSection {...props} />}
				{activeSection === "diagnostics" && <DiagnosticsSection {...props} />}
			</ScrollView>
			<MonitorBottomBar
				items={navItems}
				activeSection={activeSection}
				onChangeSection={setActiveSection}
			/>
		</View>
	);
}

export default ConsoleScreen;
