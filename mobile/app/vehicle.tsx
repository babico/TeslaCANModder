/** Vehicle tab — mirrors, locks, lights, climate, charge, drive controls. */

import { ScrollView, View, Text, StyleSheet } from 'react-native';
import VehiclePanel from '../components/VehiclePanel';
import { useTransport } from '../hooks/useTransport';
import { useBoardState } from '../hooks/useBoardState';
import { colors, spacing } from '../styles/theme';

export default function VehicleScreen() {
  const transport = useTransport();
  const board = useBoardState();

  return (
    <View style={styles.container}>
      <ScrollView contentContainerStyle={styles.content}>
        <VehiclePanel
          state={board.state}
          connected={transport.connected}
          onCommand={transport.send}
        />
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  content: { padding: spacing.md },
});
