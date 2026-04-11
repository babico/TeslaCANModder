import { useState, useCallback } from 'react';
import { View, Text, TouchableOpacity, StyleSheet, Modal, FlatList, ActivityIndicator, Platform } from 'react-native';
import { commands } from '@teslacanmodder/protocol';
import type { ScannedDevice, BoardState } from '@teslacanmodder/protocol';
import { colors, spacing, radius, shadows } from '../styles/theme';
import { StatusDot, Badge, Button } from './ui';

interface Props {
  connected: boolean;
  transportType: 'ble' | 'serial' | null;
  deviceName: string | null;
  state: BoardState;
  canUseSerial: boolean;
  canUseBle: boolean;
  onConnect: (type: 'ble' | 'serial', deviceId?: string) => void;
  onDisconnect: () => void;
  onCommand: (cmd: string) => void;
}

const BUS_DEFS = [
  { key: 'busFsd' as const, active: 'bus1' as const, label: 'FSD' },
  { key: 'busVehicle' as const, active: 'bus2' as const, label: 'Veh' },
  { key: 'busBody' as const, active: 'bus3' as const, label: 'Body' },
];

export default function ConnectionBar({
  connected, transportType, deviceName, state,
  onConnect, onDisconnect, onCommand, canUseSerial, canUseBle,
}: Props) {
  const { variant, rate, streaming, canOnline, standby, board } = state;
  const boardLabel = board === 'esp32' ? 'ESP32' : board === 'arduino' ? 'UNO' : '';
  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Waiting';
  const busStatus: 'connected' | 'warning' | 'disconnected' = standby ? 'warning' : canOnline ? 'connected' : 'disconnected';

  const activeBuses = BUS_DEFS
    .filter(b => state[b.key])
    .map(b => ({ label: b.label, ok: Boolean(state[b.active]) }));

  const [scanning, setScanning] = useState(false);
  const [devices, setDevices] = useState<ScannedDevice[]>([]);
  const [bleError, setBleError] = useState<string | null>(null);

  const startScan = useCallback(() => {
    setDevices([]);
    setBleError(null);
    setScanning(true);
    if (Platform.OS === 'web') return;
    import('../lib/transport/ble').then(({ scanForDevices }) => {
      try {
        const stop = scanForDevices((device) => {
          setDevices(prev => prev.find(d => d.id === device.id) ? prev : [...prev, device]);
        }, 8000);
        setTimeout(() => { stop(); setScanning(false); }, 8000);
      } catch (err: unknown) {
        setScanning(false);
        const msg = err instanceof Error ? err.message : 'BLE scan failed';
        setBleError(msg.includes('NativeEventEmitter')
          ? 'Bluetooth not available in Expo Go. A development build is required for BLE.'
          : msg);
      }
    }).catch(() => {
      setScanning(false);
      setBleError('Bluetooth not available in Expo Go. A development build is required for BLE.');
    });
  }, []);

  const selectDevice = useCallback((device: ScannedDevice) => {
    setScanning(false);
    setDevices([]);
    onConnect('ble', device.id);
  }, [onConnect]);

  const dismissModal = useCallback(() => {
    setScanning(false);
    setDevices([]);
    setBleError(null);
  }, []);

  return (
    <View style={[styles.bar, connected && styles.barConnected]}>
      {/* Status row */}
      <View style={styles.statusRow}>
        <StatusDot status={connected ? 'connected' : 'disconnected'} />
        <View style={styles.statusInfo}>
          <Text style={styles.label}>
            {connected ? (deviceName || (transportType === 'serial' ? 'USB Serial' : 'Bluetooth')) : 'Not Connected'}
          </Text>
          {connected && (
            <View style={styles.detailRow}>
              {boardLabel ? <Badge label={boardLabel} variant="blue" /> : null}
              <Badge label={variant.toUpperCase()} variant="accent" />
              <Badge label={`${rate} msg/s`} />
              <Badge label={busLabel} variant={busStatus === 'connected' ? 'success' : busStatus === 'warning' ? 'warning' : 'default'} />
            </View>
          )}
        </View>
      </View>

      {/* Bus status chips */}
      {connected && activeBuses.length > 0 && (
        <View style={styles.busRow}>
          {activeBuses.map(b => (
            <Badge key={b.label} label={`${b.label} ${b.ok ? '✓' : '✗'}`} variant={b.ok ? 'success' : 'default'} />
          ))}
        </View>
      )}

      {/* Actions */}
      <View style={styles.actions}>
        {connected ? (
          <>
            {(['hw4', 'hw3', 'legacy'] as const).map(v => (
              <Button
                key={v}
                label={v === 'legacy' ? 'Legacy' : v.toUpperCase()}
                active={variant === v}
                compact
                onPress={() => onCommand(commands.variant(v))}
              />
            ))}
            <Button
              label={streaming ? 'Stop' : 'Stream'}
              active={streaming}
              compact
              onPress={() => onCommand(commands.stream(!streaming))}
            />
            <Button label="Disconnect" variant="danger" compact onPress={onDisconnect} />
          </>
        ) : (
          <>
            {canUseSerial && (
              <Button label="USB" variant="primary" compact onPress={() => onConnect('serial')} />
            )}
            {canUseBle && (
              <Button label="Bluetooth" variant="primary" compact onPress={startScan} />
            )}
            {!canUseSerial && !canUseBle && (
              <Text style={styles.muted}>No transport available</Text>
            )}
          </>
        )}
      </View>

      {/* BLE scan modal */}
      <Modal visible={scanning || devices.length > 0 || bleError !== null} transparent animationType="slide">
        <View style={styles.modalOverlay}>
          <View style={styles.modalContent}>
            {bleError ? (
              <>
                <Text style={styles.modalTitle}>Bluetooth Error</Text>
                <Text style={styles.modalError}>{bleError}</Text>
              </>
            ) : (
              <>
                <Text style={styles.modalTitle}>
                  {scanning ? 'Scanning for devices...' : 'Select a device'}
                </Text>
                {scanning && <ActivityIndicator color={colors.accent} style={{ marginVertical: 12 }} />}
                <FlatList
                  data={devices}
                  keyExtractor={item => item.id}
                  renderItem={({ item }) => (
                    <TouchableOpacity style={styles.deviceRow} onPress={() => selectDevice(item)}>
                      <Text style={styles.deviceName}>{item.name || 'Unknown Device'}</Text>
                      <View style={styles.deviceMeta}>
                        <Text style={styles.deviceId}>{item.id}</Text>
                        {item.rssi != null && <Badge label={`${item.rssi} dBm`} variant="blue" />}
                      </View>
                    </TouchableOpacity>
                  )}
                  ListEmptyComponent={
                    !scanning ? <Text style={styles.muted}>No devices found</Text> : null
                  }
                />
              </>
            )}
            <Button label="Cancel" variant="danger" onPress={dismissModal} style={{ marginTop: 12, alignSelf: 'center' }} />
          </View>
        </View>
      </Modal>
    </View>
  );
}

const styles = StyleSheet.create({
  bar: {
    padding: spacing.md,
    backgroundColor: colors.surface,
    borderBottomWidth: 1,
    borderBottomColor: colors.border,
    gap: spacing.sm,
    ...shadows.sm,
  },
  barConnected: { borderBottomColor: colors.success },
  statusRow: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm },
  statusInfo: { flex: 1, gap: 4 },
  label: { color: colors.text, fontWeight: '600', fontSize: 14 },
  detailRow: { flexDirection: 'row', gap: spacing.xs, flexWrap: 'wrap' },
  busRow: { flexDirection: 'row', gap: spacing.xs },
  actions: { flexDirection: 'row', gap: spacing.xs, flexWrap: 'wrap' },
  muted: { color: colors.textMuted, fontSize: 12 },
  modalOverlay: { flex: 1, justifyContent: 'flex-end', backgroundColor: 'rgba(0,0,0,0.6)' },
  modalContent: {
    backgroundColor: colors.surface,
    borderTopLeftRadius: radius.xl,
    borderTopRightRadius: radius.xl,
    padding: spacing.lg,
    maxHeight: '60%',
    ...shadows.lg,
  },
  modalTitle: { color: colors.text, fontSize: 16, fontWeight: '600', marginBottom: spacing.sm, textAlign: 'center' },
  modalError: { color: colors.textMuted, textAlign: 'center', marginVertical: spacing.md, fontSize: 13 },
  deviceRow: {
    paddingVertical: spacing.sm,
    paddingHorizontal: spacing.md,
    borderBottomWidth: 1,
    borderBottomColor: colors.border,
    gap: 4,
  },
  deviceName: { color: colors.text, fontSize: 14, fontWeight: '500' },
  deviceMeta: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm },
  deviceId: { color: colors.textMuted, fontSize: 11 },
});
