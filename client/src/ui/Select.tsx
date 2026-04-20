/**
 * Select — a labeled selector that opens a Sheet to pick from options.
 *
 * Designed for sparse option lists (≤20 items). For large datasets,
 * use a search-enabled Sheet directly.
 */
import React, { useState } from "react";
import { Pressable, StyleSheet, Text, View } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";
import { Sheet } from "./Sheet";

export interface SelectOption<T extends string = string> {
  label: string;
  value: T;
  description?: string;
}

export interface SelectProps<T extends string = string> {
  label?: string;
  placeholder?: string;
  value?: T;
  options: SelectOption<T>[];
  onChange?: (value: T) => void;
  error?: string;
  hint?: string;
  variant?: "default" | "dark";
  disabled?: boolean;
}

export function Select<T extends string = string>({
  label,
  placeholder = "Select…",
  value,
  options,
  onChange,
  error,
  hint,
  variant = "default",
  disabled = false,
}: SelectProps<T>) {
  const [open, setOpen] = useState(false);
  const isDark = variant === "dark";
  const hasError = Boolean(error);

  const selected = options.find((o) => o.value === value);

  return (
    <View style={styles.wrapper}>
      {label ? (
        <Text style={[styles.label, isDark ? styles.labelDark : styles.labelLight]}>
          {label}
        </Text>
      ) : null}

      <Pressable
        style={[
          styles.trigger,
          isDark ? styles.triggerDark : styles.triggerLight,
          hasError ? styles.triggerError : undefined,
          disabled ? styles.triggerDisabled : undefined,
        ]}
        onPress={() => !disabled && setOpen(true)}
        accessibilityRole="button"
        accessibilityState={{ disabled }}
      >
        <Text
          style={[
            styles.triggerText,
            selected
              ? isDark ? styles.valueTextDark : styles.valueTextLight
              : styles.placeholderText,
          ]}
          numberOfLines={1}
        >
          {selected ? selected.label : placeholder}
        </Text>
        <Text style={[styles.chevron, isDark ? styles.chevronDark : styles.chevronLight]}>
          ▾
        </Text>
      </Pressable>

      {hasError ? (
        <Text style={styles.errorText}>{error}</Text>
      ) : hint ? (
        <Text style={[styles.hintText, isDark ? styles.hintDark : styles.hintLight]}>{hint}</Text>
      ) : null}

      <Sheet
        visible={open}
        onClose={() => setOpen(false)}
        title={label ?? placeholder}
        estimatedHeight={Math.min(72 + options.length * 52, 480)}
      >
        {options.map((option) => {
          const isActive = option.value === value;
          return (
            <Pressable
              key={option.value}
              style={[styles.option, isActive ? styles.optionActive : undefined]}
              onPress={() => {
                onChange?.(option.value);
                setOpen(false);
              }}
            >
              <View style={styles.optionInner}>
                <Text style={[styles.optionLabel, isActive ? styles.optionLabelActive : undefined]}>
                  {option.label}
                </Text>
                {option.description ? (
                  <Text style={styles.optionDescription}>{option.description}</Text>
                ) : null}
              </View>
              {isActive ? <Text style={styles.check}>✓</Text> : null}
            </Pressable>
          );
        })}
      </Sheet>
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
  trigger: {
    flexDirection: "row",
    alignItems: "center",
    borderRadius: radius.md,
    borderWidth: 1,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm2,
    gap: spacing.sm,
  },
  triggerLight: {
    backgroundColor: colors.background,
    borderColor: colors.border,
  },
  triggerDark: {
    backgroundColor: colors.dashCard,
    borderColor: colors.dashCardBorder,
  },
  triggerError: {
    borderColor: colors.alarmCritical,
  },
  triggerDisabled: {
    opacity: 0.45,
  },
  triggerText: {
    flex: 1,
    fontSize: font.size.md,
  },
  placeholderText: {
    color: colors.foregroundMuted,
  },
  valueTextLight: {
    color: colors.foreground,
  },
  valueTextDark: {
    color: colors.dashValue,
  },
  chevron: {
    fontSize: font.size.md,
  },
  chevronLight: {
    color: colors.foregroundMuted,
  },
  chevronDark: {
    color: colors.dashSecondary,
  },
  errorText: {
    fontSize: font.size.sm,
    color: colors.alarmCritical,
  },
  hintText: {
    fontSize: font.size.sm,
  },
  hintLight: {
    color: colors.foregroundMuted,
  },
  hintDark: {
    color: colors.dashSecondary,
  },
  // Sheet option rows
  option: {
    flexDirection: "row",
    alignItems: "center",
    paddingVertical: spacing.md2,
    paddingHorizontal: spacing.sm,
    borderRadius: radius.md,
    gap: spacing.sm,
    marginBottom: spacing.xs2,
  },
  optionActive: {
    backgroundColor: colors.dashCardBorder,
  },
  optionInner: {
    flex: 1,
  },
  optionLabel: {
    fontSize: font.size.md,
    color: colors.dashLabel,
  },
  optionLabelActive: {
    color: colors.dashValue,
    fontWeight: font.weight.semibold,
  },
  optionDescription: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
    marginTop: 2,
  },
  check: {
    color: colors.primary,
    fontSize: font.size.lg,
    fontWeight: font.weight.bold,
  },
});
