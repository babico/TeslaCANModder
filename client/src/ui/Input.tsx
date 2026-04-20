import { StyleSheet, TextInput, Text, View, type TextInputProps } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

export interface InputProps extends Omit<TextInputProps, "style"> {
  /** Optional label rendered above the input */
  label?: string;
  /** Error message rendered below the input */
  error?: string;
  /** Hint rendered below the input (shown only when no error) */
  hint?: string;
  variant?: "default" | "dark";
}

export function Input({
  label,
  error,
  hint,
  variant = "default",
  ...rest
}: InputProps) {
  const isDark = variant === "dark";
  const hasError = Boolean(error);
  return (
    <View style={styles.wrapper}>
      {label ? (
        <Text style={[styles.label, isDark ? styles.labelDark : styles.labelLight]}>
          {label}
        </Text>
      ) : null}
      <TextInput
        placeholderTextColor={isDark ? colors.dashSecondary : colors.foregroundMuted}
        {...rest}
        style={[
          styles.input,
          isDark ? styles.inputDark : styles.inputLight,
          hasError ? styles.inputError : undefined,
        ]}
      />
      {hasError ? (
        <Text style={styles.errorText}>{error}</Text>
      ) : hint ? (
        <Text style={[styles.hintText, isDark ? styles.hintDark : styles.hintLight]}>{hint}</Text>
      ) : null}
    </View>
  );
}

const styles = StyleSheet.create({
  wrapper: {
    gap: spacing.xs2,
  },
  label: {
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  labelLight: {
    color: colors.foreground,
  },
  labelDark: {
    color: colors.dashLabel,
  },
  input: {
    borderRadius: radius.md,
    borderWidth: 1,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm2,
    fontSize: font.size.md,
  },
  inputLight: {
    backgroundColor: colors.background,
    borderColor: colors.border,
    color: colors.foreground,
  },
  inputDark: {
    backgroundColor: colors.dashCard,
    borderColor: colors.dashCardBorder,
    color: colors.dashValue,
  },
  inputError: {
    borderColor: colors.alarmCritical,
  },
  errorText: {
    fontSize: font.size.xs,
    color: colors.alarmCritical,
  },
  hintText: {
    fontSize: font.size.xs,
  },
  hintLight: {
    color: colors.foregroundMuted,
  },
  hintDark: {
    color: colors.dashSecondary,
  },
});
