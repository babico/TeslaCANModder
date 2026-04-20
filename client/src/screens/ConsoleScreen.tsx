import { useState } from "react";
import { ScrollView, StyleSheet, View, useWindowDimensions } from "react-native";

import {
  MonitorBottomBar,
  MonitorSidebarNavigation,
  type MonitorSectionTab,
} from "../components/monitor/MonitorNavigation";
import { ConnectionSection } from "../components/monitor/sections/ConnectionSection";
import { DiagnosticsSection } from "../components/monitor/sections/DiagnosticsSection";
import { MonitorSection } from "../components/monitor/sections/MonitorSection";
import { colors } from "../design/tokens";
import type { MonitorScreenProps } from "../components/monitor/sections/types";

export function ConsoleScreen(props: MonitorScreenProps) {
  const { width } = useWindowDimensions();
  const isWide = width >= 768;
  const [activeSection, setActiveSection] = useState<MonitorSectionTab>("console");

  const navItems = [
    { tab: "console" as const, label: "CAN Monitor", icon: "◉" },
    { tab: "connection" as const, label: "Connection", icon: "◌" },
    { tab: "diagnostics" as const, label: "Events", icon: "◈" },
  ];

  if (isWide) {
    return (
      <View style={styles.containerSplit}>
        <MonitorSidebarNavigation
          items={navItems}
          activeSection={activeSection}
          onChangeSection={setActiveSection}
          frameCount={props.frameCount}
          canConnected={Object.keys(props.boardState.canHealth ?? {}).length > 0}
        />

        <ScrollView style={styles.mainPane} contentContainerStyle={styles.mainPaneInner}>
          {activeSection === "console" && <MonitorSection {...props} />}
          {activeSection === "connection" && <ConnectionSection {...props} />}
          {activeSection === "diagnostics" && <DiagnosticsSection {...props} />}
        </ScrollView>
      </View>
    );
  }

  return (
    <View style={styles.containerMobile}>
      <ScrollView style={styles.mainPane} contentContainerStyle={styles.mainPaneInner}>
        {activeSection === "console" && <MonitorSection {...props} />}
        {activeSection === "connection" && <ConnectionSection {...props} />}
        {activeSection === "diagnostics" && <DiagnosticsSection {...props} />}
      </ScrollView>
      <MonitorBottomBar items={navItems} activeSection={activeSection} onChangeSection={setActiveSection} />
    </View>
  );
}

const styles = StyleSheet.create({
  containerSplit: {
    flex: 1,
    flexDirection: "row",
    backgroundColor: colors.dashBackground,
  },
  mainPane: {
    flex: 1,
    backgroundColor: colors.dashBackground,
  },
  mainPaneInner: {
    padding: 20,
    paddingBottom: 48,
    gap: 16,
  },
  containerMobile: {
    flex: 1,
    flexDirection: "column",
    backgroundColor: colors.dashBackground,
  },
});

export default ConsoleScreen;
