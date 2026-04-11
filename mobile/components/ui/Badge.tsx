import { View, Text, StyleSheet, type ViewStyle } from 'react-native';
import { colors, radius, spacing } from '../../styles/theme';

type BadgeVariant = 'default' | 'success' | 'warning' | 'error' | 'accent' | 'blue';

interface Props {
  label: string;
  variant?: BadgeVariant;
  style?: ViewStyle;
}

const colorMap: Record<BadgeVariant, { bg: string; text: string }> = {
  default: { bg: colors.bgTertiary, text: colors.textDim },
  success: { bg: colors.successSoft, text: colors.success },
  warning: { bg: colors.warningSoft, text: colors.warning },
  error: { bg: colors.errorSoft, text: colors.error },
  accent: { bg: colors.accentSoft, text: colors.accent },
  blue: { bg: colors.blueSoft, text: colors.blue },
};

export default function Badge({ label, variant = 'default', style }: Props) {
  const c = colorMap[variant];
  return (
    <View style={[styles.badge, { backgroundColor: c.bg }, style]}>
      <Text style={[styles.text, { color: c.text }]}>{label}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  badge: {
    paddingHorizontal: spacing.sm,
    paddingVertical: 2,
    borderRadius: radius.full,
    alignSelf: 'flex-start',
  },
  text: {
    fontSize: 11,
    fontWeight: '600',
  },
});
