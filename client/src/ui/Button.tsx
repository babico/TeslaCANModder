import { Pressable, StyleSheet, Text, type PressableProps } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

export type ButtonVariant = "primary" | "secondary" | "destructive" | "ghost";
export type ButtonSize = "sm" | "md" | "lg";

export interface ButtonProps extends Omit<PressableProps, "style"> {
  children: string;
  variant?: ButtonVariant;
  size?: ButtonSize;
  disabled?: boolean;
  fullWidth?: boolean;
}

export function Button({
  children,
  variant = "primary",
  size = "md",
  disabled = false,
  fullWidth = false,
  onPress,
  ...rest
}: ButtonProps) {
  return (
    <Pressable
      {...rest}
      onPress={disabled ? undefined : onPress}
      style={({ pressed }) => [
        styles.base,
        styles[`variant_${variant}`],
        styles[`size_${size}`],
        fullWidth ? styles.fullWidth : undefined,
        disabled ? styles.disabled : undefined,
        pressed && !disabled ? styles.pressed : undefined,
      ]}
      accessibilityRole="button"
      accessibilityState={{ disabled }}
    >
      <Text
        style={[
          styles.label,
          styles[`label_${variant}`],
          styles[`labelSize_${size}`],
          disabled ? styles.labelDisabled : undefined,
        ]}
      >
        {children}
      </Text>
    </Pressable>
  );
}

const styles = StyleSheet.create({
  base: {
    borderRadius: radius.md,
    alignItems: "center",
    justifyContent: "center",
  },
  fullWidth: {
    width: "100%",
  },
  pressed: {
    opacity: 0.78,
  },
  disabled: {
    backgroundColor: colors.disabled,
    borderColor: colors.disabled,
  },

  // Variants
  variant_primary: {
    backgroundColor: colors.primary,
  },
  variant_secondary: {
    backgroundColor: colors.secondary,
    borderWidth: 1,
    borderColor: colors.border,
  },
  variant_destructive: {
    backgroundColor: colors.destructive,
  },
  variant_ghost: {
    backgroundColor: "transparent",
    borderWidth: 1,
    borderColor: colors.border,
  },

  // Sizes
  size_sm: {
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.xs,
  },
  size_md: {
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
  },
  size_lg: {
    paddingHorizontal: spacing.xl,
    paddingVertical: spacing.md2,
  },

  // Labels
  label: {
    fontWeight: font.weight.bold,
  },
  label_primary: {
    color: colors.primaryForeground,
  },
  label_secondary: {
    color: colors.secondaryForeground,
  },
  label_destructive: {
    color: colors.destructiveForeground,
  },
  label_ghost: {
    color: colors.foreground,
  },
  labelDisabled: {
    color: colors.disabledForeground,
  },
  labelSize_sm: {
    fontSize: font.size.sm,
  },
  labelSize_md: {
    fontSize: font.size.md,
  },
  labelSize_lg: {
    fontSize: font.size.lg,
  },
});
