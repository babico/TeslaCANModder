/** Flasher tab — web-only firmware flashing UI. */

import { useState } from 'react';
import { View, Text, ScrollView, TouchableOpacity, StyleSheet, Platform } from 'react-native';
import { colors, spacing, radius } from '../styles/theme';

const VARIANTS = [
  { id: 'uno', name: 'Serial Only', bt: false, desc: 'Lightest firmware. USB serial control only.', flags: 'BUS_FSD_ACTIVE=1' },
  { id: 'uno_bt', name: 'Serial + Bluetooth', bt: true, desc: 'Adds HC-05 Bluetooth support.', flags: 'BOARD_ENABLE_BT=1' },
];

export default function FlasherScreen() {
  const [selected, setSelected] = useState<string | null>(null);
  const [status, setStatus] = useState<'idle' | 'connecting' | 'flashing' | 'done' | 'error'>('idle');
  const [message, setMessage] = useState('');

  if (Platform.OS !== 'web') {
    return (
      <View style={styles.container}>
        <Text style={styles.unsupported}>Firmware flashing is only available on web (Chrome/Edge). Use PlatformIO CLI on desktop.</Text>
      </View>
    );
  }

  async function handleFlash() {
    if (!selected) return;
    const variant = VARIANTS.find(v => v.id === selected);
    if (!variant) return;

    setStatus('connecting');
    setMessage('Requesting serial port...');

    try {
      if (!(navigator as any).serial) throw new Error('Web Serial API not supported.');
      const port = await (navigator as any).serial.requestPort();
      await port.open({ baudRate: 115200 });

      setStatus('flashing');
      setMessage(`Flashing ${variant.name}...`);

      const encoder = new TextEncoder();
      const writer = port.writable.getWriter();
      await writer.write(encoder.encode(JSON.stringify({ cmd: 'flash', env: variant.id }) + '\n'));
      writer.releaseLock();

      const decoder = new TextDecoder();
      const reader = port.readable.getReader();
      let result = '';
      const timeout = setTimeout(() => reader.cancel(), 30000);

      try {
        while (true) {
          const { value, done } = await reader.read();
          if (done) break;
          result += decoder.decode(value, { stream: true });
          if (result.includes('OK') || result.includes('ERROR')) break;
        }
      } finally {
        clearTimeout(timeout);
        reader.releaseLock();
      }
      await port.close();

      if (result.includes('ERROR')) throw new Error(result);
      setStatus('done');
      setMessage(`${variant.name} flashed successfully.`);
    } catch (err: any) {
      setStatus('error');
      setMessage(err.message || 'Flash failed.');
    }
  }

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      <Text style={styles.title}>Firmware Flasher</Text>
      <Text style={styles.subtitle}>Select and flash firmware to your Arduino Uno</Text>

      {VARIANTS.map(v => (
        <TouchableOpacity
          key={v.id}
          style={[styles.card, selected === v.id && styles.cardSelected]}
          onPress={() => { setSelected(v.id); setStatus('idle'); setMessage(''); }}
        >
          <Text style={styles.cardName}>{v.name}</Text>
          <View style={styles.badges}>
            <Text style={[styles.badge, v.bt ? styles.badgeOn : styles.badgeOff]}>BT</Text>
            <Text style={[styles.badge, v.dual ? styles.badgeOn : styles.badgeOff]}>Dual</Text>
          </View>
          <Text style={styles.cardDesc}>{v.desc}</Text>
          <Text style={styles.cardFlags}>{v.flags}</Text>
        </TouchableOpacity>
      ))}

      {selected && (
        <View style={styles.flashSection}>
          <TouchableOpacity
            style={[styles.flashBtn, (status === 'connecting' || status === 'flashing') && styles.flashBtnDisabled]}
            onPress={handleFlash}
            disabled={status === 'connecting' || status === 'flashing'}
          >
            <Text style={styles.flashBtnText}>
              {status === 'connecting' ? 'Connecting...' : status === 'flashing' ? 'Flashing...' : 'Flash via USB'}
            </Text>
          </TouchableOpacity>
          {message ? (
            <Text style={[styles.flashMsg, status === 'error' && styles.flashError, status === 'done' && styles.flashSuccess]}>
              {message}
            </Text>
          ) : null}
        </View>
      )}

      <View style={styles.cliSection}>
        <Text style={styles.cliTitle}>PlatformIO CLI</Text>
        <Text style={styles.cliCode}>{`cd hardware\npio run -e <variant> -t upload`}</Text>
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  content: { padding: spacing.lg },
  unsupported: { color: colors.textMuted, textAlign: 'center', marginTop: 100, fontSize: 16, padding: spacing.lg },
  title: { color: colors.text, fontSize: 24, fontWeight: '700', marginBottom: spacing.xs },
  subtitle: { color: colors.textMuted, fontSize: 14, marginBottom: spacing.lg },
  card: { backgroundColor: colors.surface, padding: spacing.md, borderRadius: radius.md, borderWidth: 1, borderColor: colors.border, marginBottom: spacing.sm },
  cardSelected: { borderColor: colors.accent },
  cardName: { color: colors.text, fontSize: 16, fontWeight: '600', marginBottom: spacing.xs },
  badges: { flexDirection: 'row', gap: spacing.xs, marginBottom: spacing.xs },
  badge: { paddingHorizontal: 6, paddingVertical: 2, borderRadius: radius.sm, fontSize: 11, overflow: 'hidden' },
  badgeOn: { backgroundColor: colors.successSoft, color: colors.success },
  badgeOff: { backgroundColor: colors.bgTertiary, color: colors.textDim },
  cardDesc: { color: colors.textMuted, fontSize: 13, marginBottom: spacing.xs },
  cardFlags: { color: colors.textDim, fontSize: 11, fontFamily: 'monospace' },
  flashSection: { marginTop: spacing.lg },
  flashBtn: { backgroundColor: colors.accent, paddingVertical: spacing.md, borderRadius: radius.md, alignItems: 'center' },
  flashBtnDisabled: { opacity: 0.5 },
  flashBtnText: { color: '#fff', fontSize: 16, fontWeight: '600' },
  flashMsg: { marginTop: spacing.sm, color: colors.textMuted, fontSize: 13 },
  flashError: { color: colors.error },
  flashSuccess: { color: colors.success },
  cliSection: { marginTop: spacing.xl, backgroundColor: colors.surface, padding: spacing.md, borderRadius: radius.md },
  cliTitle: { color: colors.text, fontSize: 16, fontWeight: '600', marginBottom: spacing.sm },
  cliCode: { color: colors.textMuted, fontSize: 12, fontFamily: 'monospace' },
});
