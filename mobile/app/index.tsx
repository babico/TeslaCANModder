/** Dashboard — main controls screen with connection bar and FSD/Nag/Profile controls. */

import { useEffect } from 'react';
import { View, ScrollView, StyleSheet } from 'react-native';
import ConnectionBar from '../components/ConnectionBar';
import ControlPanel from '../components/ControlPanel';
import { useTransport } from '../hooks/useTransport';
import { useBoardState } from '../hooks/useBoardState';
import { colors, spacing } from '../styles/theme';

export default function DashboardScreen() {
  const transport = useTransport();
  const board = useBoardState();

  useEffect(() => {
    transport.setOnMessage(board.handleMessage);
  }, [transport.setOnMessage, board.handleMessage]);

  useEffect(() => {
    if (!transport.connected) board.reset();
  }, [transport.connected, board.reset]);

  return (
    <View style={styles.container}>
      <ConnectionBar
        connected={transport.connected}
        transportType={transport.transportType}
        deviceName={transport.deviceName}
        state={board.state}
        canUseBle={transport.canUseBle}
        canUseSerial={transport.canUseSerial}
        onConnect={transport.connect}
        onDisconnect={transport.disconnect}
        onCommand={transport.send}
      />
      <ScrollView style={styles.body} contentContainerStyle={styles.bodyContent}>
        <ControlPanel
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
  body: { flex: 1 },
  bodyContent: { padding: spacing.md },
});
