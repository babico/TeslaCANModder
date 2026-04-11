import { View, Text, StyleSheet, type ViewStyle } from 'react-native';
import { colors, spacing } from '../../styles/theme';

interface Props {
  title: string;
  right?: React.ReactNode;
  children: React.ReactNode;
  style?: ViewStyle;
}

export default function Section({ title, right, children, style }: Props) {
  return (
    <View style={[styles.section, style]}>
      <View style={styles.header}>
        <Text style={styles.title}>{title}</Text>
        {right && typeof right === 'string' ? <Text style={styles.right}>{right}</Text> : right}
      </View>
      {children}
    </View>
  );
}

const styles = StyleSheet.create({
  section: {
    marginBottom: spacing.md,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: spacing.sm,
  },
  title: {
    color: colors.text,
    fontWeight: '600',
    fontSize: 15,
  },
  right: {
    color: colors.textMuted,
    fontSize: 12,
  },
});
