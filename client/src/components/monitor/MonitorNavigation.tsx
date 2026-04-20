import { Pressable, StyleSheet, Text, View } from "react-native";
import { colors } from "../../design/tokens";

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
  return (
    <View style={styles.sidebar}>
      <View style={styles.sidebarHeader}>
        <Text style={styles.sidebarBrand}>Console</Text>
        <Text style={styles.sidebarSub}>CAN Workspace</Text>
      </View>
      <View style={styles.sidebarNav}>
        {items.map((item) => (
          <Pressable
            key={item.tab}
            onPress={() => onChangeSection(item.tab)}
            style={[styles.sidebarItem, activeSection === item.tab ? styles.sidebarItemActive : undefined]}
          >
            <Text style={[styles.sidebarIcon, activeSection === item.tab ? styles.sidebarIconActive : undefined]}>{item.icon}</Text>
            <Text style={[styles.sidebarLabel, activeSection === item.tab ? styles.sidebarLabelActive : undefined]}>{item.label}</Text>
          </Pressable>
        ))}
      </View>
      <View style={styles.sidebarFooter}>
        <Text style={styles.sidebarFooterText}>{frameCount} frames · {canConnected ? "CAN OK" : "No CAN"}</Text>
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
  return (
    <View style={styles.bottomBar}>
      {items.map((item) => (
        <Pressable
          key={item.tab}
          onPress={() => onChangeSection(item.tab)}
          style={[styles.bottomBarItem, activeSection === item.tab ? styles.bottomBarItemActive : undefined]}
        >
          <Text style={[styles.bottomBarIcon, activeSection === item.tab ? styles.bottomBarIconActive : undefined]}>{item.icon}</Text>
          <Text style={[styles.bottomBarLabel, activeSection === item.tab ? styles.bottomBarLabelActive : undefined]}>{item.label}</Text>
        </Pressable>
      ))}
    </View>
  );
}

const styles = StyleSheet.create({
  sidebar: {
    width: 200,
    backgroundColor: colors.dashCard,
    borderRightWidth: 1,
    borderRightColor: colors.dashCardBorder,
    flexDirection: "column",
  },
  sidebarHeader: {
    paddingHorizontal: 20,
    paddingTop: 24,
    paddingBottom: 16,
    borderBottomWidth: 1,
    borderBottomColor: colors.dashCardBorder,
    gap: 2,
  },
  sidebarBrand: {
    fontSize: 16,
    fontWeight: "700",
    color: colors.dashValue,
    letterSpacing: 0.5,
  },
  sidebarSub: {
    fontSize: 11,
    color: colors.dashLabel,
    letterSpacing: 0.3,
  },
  sidebarNav: {
    flex: 1,
    paddingVertical: 8,
    gap: 2,
  },
  sidebarItem: {
    flexDirection: "row",
    alignItems: "center",
    gap: 10,
    paddingHorizontal: 16,
    paddingVertical: 10,
    marginHorizontal: 8,
    borderRadius: 6,
  },
  sidebarItemActive: {
    backgroundColor: colors.backgroundDarkSubtle,
  },
  sidebarIcon: {
    fontSize: 14,
    color: colors.dashMuted,
  },
  sidebarIconActive: {
    color: colors.dashPrimary,
  },
  sidebarLabel: {
    fontSize: 13,
    fontWeight: "500",
    color: colors.dashLabel,
  },
  sidebarLabelActive: {
    color: colors.dashPrimary,
    fontWeight: "700",
  },
  sidebarFooter: {
    paddingHorizontal: 20,
    paddingVertical: 16,
    borderTopWidth: 1,
    borderTopColor: colors.dashCardBorder,
  },
  sidebarFooterText: {
    fontSize: 11,
    color: colors.dashMuted,
  },
  bottomBar: {
    flexDirection: "row",
    backgroundColor: colors.dashCard,
    borderTopWidth: 1,
    borderTopColor: colors.dashCardBorder,
  },
  bottomBarItem: {
    flex: 1,
    paddingVertical: 10,
    alignItems: "center",
    gap: 2,
  },
  bottomBarItemActive: {
    borderTopWidth: 2,
    borderTopColor: colors.dashPrimary,
  },
  bottomBarIcon: {
    fontSize: 16,
    color: colors.dashMuted,
  },
  bottomBarIconActive: {
    color: colors.dashPrimary,
  },
  bottomBarLabel: {
    fontSize: 10,
    fontWeight: "500",
    color: colors.dashMuted,
  },
  bottomBarLabelActive: {
    color: colors.dashPrimary,
  },
});
