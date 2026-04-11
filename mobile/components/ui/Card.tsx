import { View, Text, StyleSheet, type ViewStyle } from 'react-native';
import { colors, spacing, radius, shadows } from '../../styles/theme';

interface Props {
  title?: string;
  right?: React.ReactNode;
  warning?: string;
  children: React.ReactNode;
  style?: ViewStyle;
}

export default function Card({ title, right, warning, children, style }: Props) {
  return (
    <View style={[styles.card, style]}>
      {(title || right || warning) && (
        <View style={styles.header}>
          <Text style={styles.title}>{title}</Text>
          {warning ? <Text style={styles.warning}>{warning}</Text> : null}
          {right && typeof right === 'string' ? <Text style={styles.right}>{right}</Text> : right}
        </View>
      )}
      <View style={styles.body}>{children}</View>
    </View>
  );
}

const styles = StyleSheet.create({
  card: {
    backgroundColor: colors.surface,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.borderLight,
    overflow: 'hidden',
    ...shadows.sm,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    borderBottomWidth: 1,
    borderBottomColor: colors.borderLight,
  },
  title: {
    color: colors.text,
    fontWeight: '600',
    fontSize: 14,
  },
  right: {
    color: colors.textMuted,
    fontSize: 12,
  },
  warning: {
    color: colors.warning,
    fontSize: 11,
  },
  body: {
    padding: spacing.md,
  },
});
