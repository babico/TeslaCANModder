import { useState, useEffect, useCallback } from 'react';
import { View, Text, TouchableOpacity, StyleSheet, Modal, FlatList, ActivityIndicator, Platform } from 'react-native';
import { commands } from '../lib/protocol/commands';
import { colors, spacing, radius } from '../styles/theme';
import type { ScannedDevice } from '../lib/transport/types';

interface Props {
  connected: boolean;
  transport: 'usb' | 'bluetooth' | null;
  variant: string;
  rate: number;
  streaming: boolean;
  canOnline: boolean;
  standby: boolean;
  bus2: boolean;
  onConnect: (type: 'ble' | 'serial', deviceId?: string) => void;
  onDisconnect: () => void;
  onCommand: (cmd: string) => void;
  canUseSerial: boolean;
  canUseBle: boolean;
}

export default function ConnectionBar({
  connected, transport, variant, rate, streaming, canOnline, standby, bus2,
  onConnect, onDisconnect, onCommand, canUseSerial, canUseBle,
}: Props) {
  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Waiting';
  const busColor = standby ? colors.warning : canOnline ? colors.success : colors.textMuted;

  const [scanning, setScanning] = useState(false);
  const [devices, setDevices] = useState<ScannedDevice[]>([]);
  const [bleError, setBleError] = useState<string | null>(null);

  const startScan = useCallback(() => {
    setDevices([]);
    setBleError(null);
    setScanning(true);

    if (Platform.OS === 'web') return;
    // Dynamic import to avoid bundling BLE on web
    import('../lib/transport/ble').then(({ scanForDevices }) => {
      try {
        const stop = scanForDevices((device) => {
          setDevices(prev => {
            if (prev.find(d => d.id === device.id)) return prev;
            return [...prev, device];
          });
        }, 8000);
        setTimeout(() => {
          stop();
          setScanning(false);
        }, 8000);
      } catch (err: any) {
        setScanning(false);
        setBleError(err?.message?.includes('NativeEventEmitter')
          ? 'Bluetooth not available in Expo Go. A development build is required for BLE.'
          : (err?.message || 'BLE scan failed'));
      }
    }).catch((err) => {
      setScanning(false);
      setBleError('Bluetooth not available in Expo Go. A development build is required for BLE.');
    });
  }, []);

  const selectDevice = useCallback((device: ScannedDevice) => {
    setScanning(false);
    setDevices([]);
    onConnect('ble', device.id);
  }, [onConnect]);

  return (
    <View style={[styles.bar, connected && styles.barConnected]}>
      <View style={styles.status}>
        <View style={[styles.dot, { backgroundColor: connected ? colors.success : colors.textDim }]} />
        <View>
          <Text style={styles.label}>{connected ? 'Connected' : 'Not Connected'}</Text>
          {connected && (
            <Text style={styles.detail}>
              {transport === 'usb' ? 'USB' : 'Bluetooth'} · {variant.toUpperCase()}
              {bus2 ? ' · Dual CAN' : ''} · {rate} msg/s ·{' '}
              <Text style={{ color: busColor }}>{busLabel}</Text>
            </Text>
          )}
        </View>
      </View>

      <View style={styles.actions}>
        {connected ? (
          <>
            {(['hw4', 'hw3', 'legacy'] as const).map(v => (
              <TouchableOpacity
                key={v}
                style={[styles.btn, variant === v && styles.btnActive]}
                onPress={() => onCommand(commands.variant(v))}
              >
                <Text style={[styles.btnText, variant === v && styles.btnTextActive]}>
                  {v === 'legacy' ? 'Legacy' : v.toUpperCase()}
                </Text>
              </TouchableOpacity>
            ))}
            <TouchableOpacity
              style={[styles.btn, streaming && styles.btnActive]}
              onPress={() => onCommand(commands.stream(!streaming))}
            >
              <Text style={[styles.btnText, streaming && styles.btnTextActive]}>
                {streaming ? 'Stop' : 'Stream'}
              </Text>
            </TouchableOpacity>
            <TouchableOpacity style={[styles.btn, styles.btnDanger]} onPress={onDisconnect}>
              <Text style={styles.btnDangerText}>Disconnect</Text>
            </TouchableOpacity>
          </>
        ) : (
          <>
            {canUseSerial && (
              <TouchableOpacity style={[styles.btn, styles.btnPrimary]} onPress={() => onConnect('serial')}>
                <Text style={styles.btnPrimaryText}>USB</Text>
              </TouchableOpacity>
            )}
            {canUseBle && (
              <TouchableOpacity style={[styles.btn, styles.btnPrimary]} onPress={startScan}>
                <Text style={styles.btnPrimaryText}>Bluetooth</Text>
              </TouchableOpacity>
            )}
            {!canUseSerial && !canUseBle && (
              <Text style={styles.muted}>No transport available</Text>
            )}
          </>
        )}
      </View>

      <Modal visible={scanning || devices.length > 0 || bleError !== null} transparent animationType="slide">
        <View style={styles.modalOverlay}>
          <View style={styles.modalContent}>
            {bleError ? (
              <>
                <Text style={styles.modalTitle}>Bluetooth Error</Text>
                <Text style={[styles.muted, { textAlign: 'center', marginVertical: 16 }]}>{bleError}</Text>
              </>
            ) : (
              <>
                <Text style={styles.modalTitle}>
                  {scanning ? 'Scanning for devices...' : 'Select a device'}
                </Text>
                {scanning && <ActivityIndicator color={colors.accent} style={{ marginVertical: 12 }} />}
            <FlatList
              data={devices}
              keyExtractor={(item) => item.id}
              renderItem={({ item }) => (
                <TouchableOpacity style={styles.deviceRow} onPress={() => selectDevice(item)}>
                  <Text style={styles.deviceName}>{item.name || 'Unknown Device'}</Text>
                  <Text style={styles.deviceId}>{item.id}</Text>
                  {item.rssi != null && <Text style={styles.deviceRssi}>{item.rssi} dBm</Text>}
                </TouchableOpacity>
              )}
              ListEmptyComponent={
                !scanning ? <Text style={styles.muted}>No devices found</Text> : null
              }
            />
              </>
            )}
            <TouchableOpacity
              style={[styles.btn, styles.btnDanger, { marginTop: 12, alignSelf: 'center' }]}
              onPress={() => { setScanning(false); setDevices([]); setBleError(null); }}
            >
              <Text style={styles.btnDangerText}>Cancel</Text>
            </TouchableOpacity>
          </View>
        </View>
      </Modal>
    </View>
  );
}

const styles = StyleSheet.create({
  bar: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', padding: spacing.md, backgroundColor: colors.surface, borderBottomWidth: 1, borderBottomColor: colors.border },
  barConnected: { borderBottomColor: colors.success },
  status: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm, flex: 1 },
  dot: { width: 10, height: 10, borderRadius: 5 },
  label: { color: colors.text, fontWeight: '600', fontSize: 14 },
  detail: { color: colors.textMuted, fontSize: 12, marginTop: 2 },
  actions: { flexDirection: 'row', gap: spacing.xs, flexWrap: 'wrap', justifyContent: 'flex-end' },
  btn: { paddingHorizontal: 10, paddingVertical: 6, borderRadius: radius.sm, borderWidth: 1, borderColor: colors.border },
  btnActive: { backgroundColor: colors.accent, borderColor: colors.accent },
  btnText: { color: colors.text, fontSize: 12 },
  btnTextActive: { color: '#fff' },
  btnPrimary: { backgroundColor: colors.accent, borderColor: colors.accent },
  btnPrimaryText: { color: '#fff', fontSize: 12, fontWeight: '600' },
  btnDanger: { borderColor: colors.error },
  btnDangerText: { color: colors.error, fontSize: 12 },
  muted: { color: colors.textMuted, fontSize: 12 },
  modalOverlay: { flex: 1, justifyContent: 'flex-end', backgroundColor: 'rgba(0,0,0,0.5)' },
  modalContent: { backgroundColor: colors.surface, borderTopLeftRadius: radius.lg, borderTopRightRadius: radius.lg, padding: spacing.lg, maxHeight: '60%' },
  modalTitle: { color: colors.text, fontSize: 16, fontWeight: '600', marginBottom: spacing.sm, textAlign: 'center' },
  deviceRow: { paddingVertical: spacing.sm, paddingHorizontal: spacing.md, borderBottomWidth: 1, borderBottomColor: colors.border },
  deviceName: { color: colors.text, fontSize: 14, fontWeight: '500' },
  deviceId: { color: colors.textMuted, fontSize: 11, marginTop: 2 },
  deviceRssi: { color: colors.textDim, fontSize: 11, marginTop: 2 },
});
