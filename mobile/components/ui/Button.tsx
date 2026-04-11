import { TouchableOpacity, Text, StyleSheet, type ViewStyle, type TextStyle } from 'react-native';
import { colors, spacing, radius, touchMin } from '../../styles/theme';

type Variant = 'primary' | 'secondary' | 'ghost' | 'danger';

interface Props {
  label: string;
  onPress: () => void;
  variant?: Variant;
  active?: boolean;
  disabled?: boolean;
  compact?: boolean;
  style?: ViewStyle;
}

const variantStyles: Record<Variant, { base: ViewStyle; text: TextStyle }> = {
  primary: {
    base: { backgroundColor: colors.accent, borderColor: colors.accent },
    text: { color: '#fff', fontWeight: '600' },
  },
  secondary: {
    base: { backgroundColor: colors.bgSecondary, borderColor: colors.border },
    text: { color: colors.text },
  },
  ghost: {
    base: { backgroundColor: 'transparent', borderColor: 'transparent' },
    text: { color: colors.textMuted },
  },
  danger: {
    base: { backgroundColor: 'transparent', borderColor: colors.error },
    text: { color: colors.error },
  },
};

export default function Button({ label, onPress, variant = 'secondary', active, disabled, compact, style }: Props) {
  const v = variantStyles[variant];
  return (
    <TouchableOpacity
      style={[
        styles.base,
        v.base,
        active && styles.active,
        compact && styles.compact,
        disabled && styles.disabled,
        style,
      ]}
      disabled={disabled}
      onPress={onPress}
      activeOpacity={0.7}
    >
      <Text style={[styles.label, v.text, active && styles.activeText, disabled && styles.disabledText]}>
        {label}
      </Text>
    </TouchableOpacity>
  );
}

const styles = StyleSheet.create({
  base: {
    paddingHorizontal: 12,
    paddingVertical: 8,
    borderRadius: radius.md,
    borderWidth: 1,
    minHeight: touchMin,
    justifyContent: 'center',
    alignItems: 'center',
  },
  compact: {
    paddingHorizontal: 10,
    paddingVertical: 6,
    minHeight: 32,
  },
  label: {
    fontSize: 13,
    fontWeight: '500',
  },
  active: {
    backgroundColor: colors.accent,
    borderColor: colors.accent,
  },
  activeText: {
    color: '#fff',
    fontWeight: '600',
  },
  disabled: {
    opacity: 0.4,
  },
  disabledText: {
    opacity: 0.6,
  },
});
