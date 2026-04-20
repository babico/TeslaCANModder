import { StyleSheet, Text } from "react-native";
import { colors, font } from "../design/tokens";

export interface LabelProps {
  children: string;
  variant?: "default" | "dark" | "muted" | "caps";
}

export function Label({ children, variant = "default" }: LabelProps) {
  return (
    <Text style={[styles.base, styles[`variant_${variant}`]]}>
      {variant === "caps" ? children.toUpperCase() : children}
    </Text>
  );
}

const styles = StyleSheet.create({
  base: {
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  variant_default: {
    color: colors.foreground,
  },
  variant_dark: {
    color: colors.dashLabel,
  },
  variant_muted: {
    color: colors.foregroundMuted,
  },
  variant_caps: {
    color: colors.foregroundMuted,
    fontSize: font.size.xs,
    fontWeight: font.weight.bold,
    letterSpacing: 0.8,
  },
});
