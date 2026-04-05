import { View, Text, TouchableOpacity, StyleSheet } from 'react-native';
import { commands } from '../lib/protocol/commands';
import { colors, spacing, radius } from '../styles/theme';

interface Props {
  connected: boolean;
  transport: 'usb' | 'bluetooth' | null;
  variant: string;
  rate: number;
  streaming: boolean;
  canOnline: boolean;
  standby: boolean;
  bus2: boolean;
  onConnect: (type: 'usb' | 'bluetooth') => void;
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
              <TouchableOpacity style={[styles.btn, styles.btnPrimary]} onPress={() => onConnect('usb')}>
                <Text style={styles.btnPrimaryText}>USB</Text>
              </TouchableOpacity>
            )}
            {canUseBle && (
              <TouchableOpacity style={[styles.btn, styles.btnPrimary]} onPress={() => onConnect('bluetooth')}>
                <Text style={styles.btnPrimaryText}>Bluetooth</Text>
              </TouchableOpacity>
            )}
            {!canUseSerial && !canUseBle && (
              <Text style={styles.muted}>No transport available</Text>
            )}
          </>
        )}
      </View>
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
});
