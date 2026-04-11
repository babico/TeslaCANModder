import { View, StyleSheet, type ViewStyle } from 'react-native';
import { colors } from '../../styles/theme';

type Status = 'connected' | 'disconnected' | 'warning' | 'active';

interface Props {
  status: Status;
  size?: number;
  style?: ViewStyle;
}

const statusColors: Record<Status, string> = {
  connected: colors.success,
  disconnected: colors.textDim,
  warning: colors.warning,
  active: colors.accent,
};

export default function StatusDot({ status, size = 10, style }: Props) {
  return (
    <View
      style={[
        styles.dot,
        {
          width: size,
          height: size,
          borderRadius: size / 2,
          backgroundColor: statusColors[status],
        },
        style,
      ]}
    />
  );
}

const styles = StyleSheet.create({
  dot: {},
});
