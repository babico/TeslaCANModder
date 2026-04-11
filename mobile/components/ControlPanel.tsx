import { useState } from 'react';
import { View, Text, TextInput, ScrollView, StyleSheet } from 'react-native';
import { commands } from '@teslacanmodder/protocol';
import type { BoardState } from '@teslacanmodder/protocol';
import { colors, spacing, radius, shadows } from '../styles/theme';
import { Button, Card, Badge, StatusDot } from './ui';

interface Props {
  state: BoardState;
  connected: boolean;
  onCommand: (cmd: string) => void;
}

const formatUptime = (ms?: number) => {
  if (!ms) return '—';
  const s = Math.floor(ms / 1000);
  const m = Math.floor(s / 60);
  const h = Math.floor(m / 60);
  if (h > 0) return `${h}h ${m % 60}m`;
  if (m > 0) return `${m}m ${s % 60}s`;
  return `${s}s`;
};

function Stat({ label, value, color }: { label: string; value: string; color?: string }) {
  return (
    <View style={styles.stat}>
      <Text style={styles.statLabel}>{label}</Text>
      <Text style={[styles.statValue, color ? { color } : undefined]}>{value}</Text>
    </View>
  );
}

function FeatureCard({ title, status, on, children }: { title: string; status: string; on: boolean; children: React.ReactNode }) {
  return (
    <View style={[styles.featureCard, on && styles.featureCardOn]}>
      <View style={styles.featureTop}>
        <Text style={styles.featureName}>{title}</Text>
        <Badge label={status} variant={on ? 'success' : 'default'} />
      </View>
      <View style={styles.btnRow}>{children}</View>
    </View>
  );
}

export default function ControlPanel({ state, connected, onCommand }: Props) {
  const { hardware, driver, uptime, variant, board, fsd, nag, profile, profilePinned, offset, offsetPinned, isaChime, summonInject, features, canOnline, standby, bus2, summonActive } = state;
  const [customOffset, setCustomOffset] = useState('');
  const d = !connected;

  const handleCustomOffset = () => {
    const val = parseInt(customOffset, 10);
    if (!isNaN(val) && val >= 0 && val <= 100) {
      onCommand(commands.offset(val));
      setCustomOffset('');
    }
  };

  const busLabel = standby ? 'Standby' : canOnline ? 'CAN Active' : 'Offline';
  const busStatus: 'connected' | 'warning' | 'disconnected' = standby ? 'warning' : canOnline ? 'connected' : 'disconnected';

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      {/* Board + EEPROM info */}
      <View style={styles.row}>
        <Card title="Board" right={<><Badge label={board === 'esp32' ? 'ESP32' : board === 'arduino' ? 'UNO' : '?'} variant="blue" /><Badge label={variant.toUpperCase()} variant="accent" /></>} style={styles.half}>
          <View style={styles.healthBanner}>
            <StatusDot status={busStatus} size={8} />
            <Text style={styles.healthText}>{busLabel}</Text>
          </View>
          <View style={styles.statGrid}>
            <Stat label="Board" value={hardware} />
            <Stat label="Driver" value={driver} />
            <Stat label="Uptime" value={formatUptime(uptime)} />
            <Stat label="CAN" value={bus2 ? 'Dual' : 'Single'} />
          </View>
        </Card>

        <Card title={board === 'esp32' ? 'NVS Flash' : 'EEPROM'} right={<Badge label="Saved" variant="success" />} style={styles.half}>
          <View style={styles.statGrid}>
            <Stat label="Variant" value={variant.toUpperCase()} />
            <Stat label="FSD" value={fsd ? 'ON' : 'OFF'} color={fsd ? colors.success : undefined} />
            <Stat label="Nag" value={nag ? 'ON' : 'OFF'} color={nag ? colors.success : undefined} />
            <Stat label="Profile" value={`${profile} ${profilePinned ? '(pin)' : '(auto)'}`} />
            {features.speedOffset && <Stat label="Offset" value={`${offset}% ${offsetPinned ? '(pin)' : '(auto)'}`} />}
            {features.isaSpeedChime && <Stat label="ISA" value={isaChime ? 'Suppress' : 'Original'} />}
            {features.summon && <Stat label="Summon Inj." value={summonInject ? 'ON' : 'OFF'} color={summonInject ? colors.success : undefined} />}
          </View>
        </Card>
      </View>

      {/* Feature toggles */}
      <View style={styles.featureGrid}>
        <FeatureCard title="FSD" status={fsd ? 'ON' : 'OFF'} on={fsd}>
          <Button label="Enable" active={fsd} disabled={d} compact onPress={() => onCommand(commands.fsd(true))} />
          <Button label="Disable" active={!fsd} disabled={d} compact onPress={() => onCommand(commands.fsd(false))} />
        </FeatureCard>

        <FeatureCard title="Nag Suppress" status={nag ? 'ON' : 'OFF'} on={nag}>
          <Button label="Enable" active={nag} disabled={d} compact onPress={() => onCommand(commands.nag(true))} />
          <Button label="Disable" active={!nag} disabled={d} compact onPress={() => onCommand(commands.nag(false))} />
        </FeatureCard>

        {features.isaSpeedChime && (
          <FeatureCard title="ISA Chime" status={isaChime ? 'Suppress' : 'Original'} on={isaChime}>
            <Button label="Suppress" active={isaChime} disabled={d} compact onPress={() => onCommand(commands.isaChime(true))} />
            <Button label="Original" active={!isaChime} disabled={d} compact onPress={() => onCommand(commands.isaChime(false))} />
          </FeatureCard>
        )}

        {features.summon && (
          <FeatureCard title="Summon Inject" status={summonInject ? 'ON' : 'OFF'} on={summonInject}>
            <Button label="Enable" active={summonInject} disabled={d} compact onPress={() => onCommand(commands.summonInject(true))} />
            <Button label="Disable" active={!summonInject} disabled={d} compact onPress={() => onCommand(commands.summonInject(false))} />
          </FeatureCard>
        )}

        {features.summon && (
          <FeatureCard title="Summon" status={summonActive ? 'Active' : 'Idle'} on={summonActive}>
            <Button label="Fwd" disabled={d} compact onPress={() => onCommand(commands.summonForward())} />
            <Button label="Rev" disabled={d} compact onPress={() => onCommand(commands.summonReverse())} />
            <Button label="Stop" variant="danger" disabled={d} compact onPress={() => onCommand(commands.summonStop())} />
          </FeatureCard>
        )}
      </View>

      {/* Speed Profile */}
      <Card title="Speed Profile" right={<Text style={styles.cardRight}>{profile} {profilePinned ? '(PINNED)' : '(AUTO)'}</Text>}>
        <View style={styles.btnRow}>
          {[{ id: 0, n: 'Chill' }, { id: 1, n: 'Normal' }, { id: 2, n: 'Hurry' }, { id: 3, n: 'Max' }, { id: 4, n: 'Sloth' }].map(p => (
            <Button key={p.id} label={p.n} active={profilePinned && profile === p.id} disabled={d} compact onPress={() => onCommand(commands.profile(p.id))} />
          ))}
          <Button label="Auto" active={!profilePinned} disabled={d} compact onPress={() => onCommand(commands.profileAuto())} />
        </View>
      </Card>

      {/* Speed Offset */}
      {features.speedOffset && (
        <Card title="Speed Offset" right={<Text style={styles.cardRight}>{offset}% {offsetPinned ? '(PINNED)' : '(AUTO)'}</Text>}>
          <View style={styles.btnRow}>
            {[0, 20, 40, 60, 80, 100].map(o => (
              <Button key={o} label={`${o}%`} active={offsetPinned && offset === o} disabled={d} compact onPress={() => onCommand(commands.offset(o))} />
            ))}
            <Button label="Auto" active={!offsetPinned} disabled={d} compact onPress={() => onCommand(commands.offsetAuto())} />
          </View>
          <View style={styles.customRow}>
            <TextInput style={styles.customInput} keyboardType="numeric" placeholder="Custom %" placeholderTextColor={colors.textDim} value={customOffset} onChangeText={setCustomOffset} editable={connected} />
            <Button label="Set" disabled={d || !customOffset} compact onPress={handleCustomOffset} />
          </View>
        </Card>
      )}
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1 },
  content: { gap: spacing.md },
  row: { flexDirection: 'row', gap: spacing.sm },
  half: { flex: 1 },
  healthBanner: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: spacing.sm,
    padding: spacing.sm,
    backgroundColor: colors.bgTertiary,
    borderRadius: radius.md,
    marginBottom: spacing.sm,
  },
  healthText: { color: colors.text, fontSize: 13, fontWeight: '500' },
  statGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.sm },
  stat: { width: '45%' },
  statLabel: { color: colors.textMuted, fontSize: 11 },
  statValue: { color: colors.text, fontWeight: '600', fontSize: 13 },
  featureGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.sm },
  featureCard: {
    backgroundColor: colors.surface,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.border,
    padding: spacing.md,
    width: '48%',
    ...shadows.sm,
  },
  featureCardOn: {
    borderColor: colors.success,
  },
  featureTop: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', marginBottom: spacing.sm },
  featureName: { color: colors.text, fontWeight: '600', fontSize: 14 },
  btnRow: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.xs },
  cardRight: { color: colors.textMuted, fontSize: 12 },
  customRow: { flexDirection: 'row', gap: spacing.sm, marginTop: spacing.sm, alignItems: 'center' },
  customInput: {
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: radius.md,
    padding: spacing.sm,
    color: colors.text,
    fontSize: 13,
    width: 90,
    backgroundColor: colors.bgTertiary,
  },
});
