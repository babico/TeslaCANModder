import { Pressable, StyleSheet, Text, View } from "react-native";

import { colors, font, radius, spacing } from "../design/tokens";
import type { AppTabRoute } from "../state/appRoute";

interface MenuHeaderProps {
  tabs: Array<{ id: AppTabRoute; label: string }>;
  activeTab: AppTabRoute;
  onSelectTab: (tab: AppTabRoute) => void;
}

export function MenuHeader({ tabs, activeTab, onSelectTab }: MenuHeaderProps) {
  return (
    <View style={styles.wrap}>
      <View style={styles.bar}>
        {tabs.map((tab) => {
          const active = activeTab === tab.id;
          return (
            <Pressable
              key={tab.id}
              onPress={() => onSelectTab(tab.id)}
              style={({ pressed }) => [
                styles.item,
                active ? styles.itemActive : undefined,
                pressed ? styles.itemPressed : undefined,
              ]}
            >
              <Text style={[styles.label, active ? styles.labelActive : undefined]}>{tab.label}</Text>
            </Pressable>
          );
        })}
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  wrap: {
    borderTopWidth: 1,
    borderTopColor: colors.dashCardBorder,
    backgroundColor: colors.dashBackground,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
  },
  bar: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.sm,
    backgroundColor: colors.dashCard,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.lg,
    padding: spacing.xs,
  },
  item: {
    flex: 1,
    borderRadius: radius.md,
    minHeight: 38,
    alignItems: "center",
    justifyContent: "center",
    paddingHorizontal: spacing.md,
  },
  itemActive: {
    backgroundColor: colors.backgroundDarkSubtle,
    borderWidth: 1,
    borderColor: colors.primary,
  },
  itemPressed: {
    opacity: 0.85,
  },
  label: {
    color: colors.dashLabel,
    fontSize: font.size.md2,
    fontWeight: font.weight.semibold,
  },
  labelActive: {
    color: colors.dashPrimary,
    fontWeight: font.weight.bold,
  },
});

export default MenuHeader;
