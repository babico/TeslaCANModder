import { StyleSheet, View, type ViewProps } from "react-native";
import { colors, radius, spacing } from "../design/tokens";

export type CardVariant = "default" | "dark" | "subtle";

export interface CardProps extends ViewProps {
  variant?: CardVariant;
  children: React.ReactNode;
}

export function Card({ variant = "default", style, children, ...rest }: CardProps) {
  return (
    <View
      {...rest}
      style={[styles.base, styles[`variant_${variant}`], style]}
    >
      {children}
    </View>
  );
}

const styles = StyleSheet.create({
  base: {
    borderRadius: radius.lg,
    padding: spacing.md,
    borderWidth: 1,
    gap: spacing.sm,
  },
  variant_default: {
    backgroundColor: colors.backgroundCard,
    borderColor: colors.border,
  },
  variant_dark: {
    backgroundColor: colors.dashCard,
    borderColor: colors.dashCardBorder,
  },
  variant_subtle: {
    backgroundColor: colors.backgroundSubtle,
    borderColor: colors.border,
  },
});
