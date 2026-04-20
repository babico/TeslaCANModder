import { StyleSheet, Text, View } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

/**
 * StatChip — compact labeled data block used in telemetry grids.
 * Available in light (default) and dark (dashboard) themes.
 */
export interface StatChipProps {
  label: string;
  value: string;
  unit?: string;
  variant?: "default" | "dark";
  /** Highlight the value (e.g. active gear, warning state) */
  accent?: boolean;
}

export function StatChip({ label, value, unit, variant = "default", accent = false }: StatChipProps) {
  const isDark = variant === "dark";
  return (
    <View style={[styles.base, isDark ? styles.baseDark : styles.baseLight]}>
      <Text style={[styles.label, isDark ? styles.labelDark : styles.labelLight]}>
        {label}
      </Text>
      <View style={styles.row}>
        <Text style={[styles.value, isDark ? styles.valueDark : styles.valueLight, accent ? (isDark ? styles.valueAccentDark : styles.valueAccentLight) : undefined]}>
          {value}
        </Text>
        {unit ? (
          <Text style={[styles.unit, isDark ? styles.unitDark : styles.unitLight]}>{unit}</Text>
        ) : null}
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  base: {
    minWidth: 112,
    flexGrow: 1,
    borderRadius: radius.md2,
    borderWidth: 1,
    paddingHorizontal: spacing.md2,
    paddingVertical: spacing.sm,
    gap: spacing.xs2,
  },
  baseLight: {
    backgroundColor: colors.backgroundSubtle,
    borderColor: colors.border,
  },
  baseDark: {
    backgroundColor: colors.dashCard,
    borderColor: colors.dashCardBorder,
  },
  row: {
    flexDirection: "row",
    alignItems: "flex-end",
    gap: spacing.xs2,
  },
  label: {
    fontSize: font.size.xs,
    fontWeight: font.weight.semibold,
    textTransform: "uppercase",
    letterSpacing: 0.4,
  },
  labelLight: {
    color: colors.foregroundMuted,
  },
  labelDark: {
    color: colors.dashLabel,
  },
  value: {
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
  },
  valueLight: {
    color: colors.foreground,
  },
  valueDark: {
    color: colors.dashValue,
  },
  valueAccentLight: {
    color: colors.primary,
  },
  valueAccentDark: {
    color: colors.dashPrimary,
  },
  unit: {
    fontSize: font.size.xs,
    fontWeight: font.weight.medium,
    paddingBottom: 1,
  },
  unitLight: {
    color: colors.foregroundSubtle,
  },
  unitDark: {
    color: colors.dashUnit,
  },
});
