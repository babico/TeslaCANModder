import { useState } from 'react';
import { View, Text, TouchableOpacity, TextInput, ScrollView, StyleSheet } from 'react-native';
import { commands } from '../lib/protocol/commands';
import type { BoardState } from '../hooks/useBoardState';
import { colors, spacing, radius } from '../styles/theme';

interface Props {
  state: BoardState;
  connected: boolean;
  onCommand: (cmd: string) => void;
}

function Btn({ label, active, disabled, danger, onPress }: { label: string; active?: boolean; disabled?: boolean; danger?: boolean; onPress: () => void }) {
  return (
    <TouchableOpacity
      style={[styles.btn, active && styles.btnActive, danger && styles.btnDanger]}
      disabled={disabled}
      onPress={onPress}
    >
      <Text style={[styles.btnText, active && styles.btnTextActive, danger && styles.btnDangerText, disabled && styles.btnTextDisabled]}>{label}</Text>
    </TouchableOpacity>
  );
}

function Section({ title, right, children }: { title: string; right?: string; children: React.ReactNode }) {
  return (
    <View style={styles.section}>
      <View style={styles.sectionHeader}>
        <Text style={styles.sectionTitle}>{title}</Text>
        {right ? <Text style={styles.sectionRight}>{right}</Text> : null}
      </View>
      {children}
    </View>
  );
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

export default function ControlPanel({ state, connected, onCommand }: Props) {
  const { hardware, driver, uptime, variant, fsd, nag, profile, profilePinned, offset, offsetPinned, isaChime, features, canOnline, standby, bus2, summonActive } = state;
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
  const busColor = standby ? colors.warning : canOnline ? colors.success : colors.textDim;

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      {/* Board + EEPROM info */}
      <View style={styles.row}>
        <View style={styles.half}>
          <Section title="Board" right={variant.toUpperCase()}>
            <View style={[styles.healthBanner, { borderLeftColor: busColor }]}>
              <View style={[styles.healthDot, { backgroundColor: busColor }]} />
              <Text style={styles.healthText}>{busLabel}</Text>
            </View>
            <View style={styles.statGrid}>
              <Stat label="Board" value={hardware} />
              <Stat label="Driver" value={driver} />
              <Stat label="Uptime" value={formatUptime(uptime)} />
              <Stat label="CAN" value={bus2 ? 'Dual' : 'Single'} />
            </View>
          </Section>
        </View>
        <View style={styles.half}>
          <Section title="EEPROM" right="Saved">
            <View style={styles.statGrid}>
              <Stat label="Variant" value={variant.toUpperCase()} />
              <Stat label="FSD" value={fsd ? 'ON' : 'OFF'} color={fsd ? colors.success : undefined} />
              <Stat label="Nag" value={nag ? 'ON' : 'OFF'} color={nag ? colors.success : undefined} />
              <Stat label="Profile" value={`${profile} ${profilePinned ? '(pin)' : '(auto)'}`} />
              {features.speedOffset && <Stat label="Offset" value={`${offset}% ${offsetPinned ? '(pin)' : '(auto)'}`} />}
              {features.isaSpeedChime && <Stat label="ISA" value={isaChime ? 'Suppress' : 'Original'} />}
            </View>
          </Section>
        </View>
      </View>

      {/* Feature toggles */}
      <View style={styles.featureGrid}>
        <FeatureCard title="FSD" status={fsd ? 'ON' : 'OFF'} on={fsd}>
          <Btn label="Enable" active={fsd} disabled={d} onPress={() => onCommand(commands.fsd(true))} />
          <Btn label="Disable" active={!fsd} disabled={d} onPress={() => onCommand(commands.fsd(false))} />
        </FeatureCard>

        <FeatureCard title="Nag Suppress" status={nag ? 'ON' : 'OFF'} on={nag}>
          <Btn label="Enable" active={nag} disabled={d} onPress={() => onCommand(commands.nag(true))} />
          <Btn label="Disable" active={!nag} disabled={d} onPress={() => onCommand(commands.nag(false))} />
        </FeatureCard>

        {features.isaSpeedChime && (
          <FeatureCard title="ISA Chime" status={isaChime ? 'Suppress' : 'Original'} on={isaChime}>
            <Btn label="Suppress" active={isaChime} disabled={d} onPress={() => onCommand(commands.isaChime(true))} />
            <Btn label="Original" active={!isaChime} disabled={d} onPress={() => onCommand(commands.isaChime(false))} />
          </FeatureCard>
        )}

        {features.summon && (
          <FeatureCard title="Summon" status={summonActive ? 'Active' : 'Idle'} on={summonActive}>
            <Btn label="Fwd" disabled={d} onPress={() => onCommand(commands.summonForward())} />
            <Btn label="Rev" disabled={d} onPress={() => onCommand(commands.summonReverse())} />
            <Btn label="Stop" danger disabled={d} onPress={() => onCommand(commands.summonStop())} />
          </FeatureCard>
        )}
      </View>

      {/* Speed Profile */}
      <Section title="Speed Profile" right={`${profile} ${profilePinned ? '(PINNED)' : '(AUTO)'}`}>
        <View style={styles.btnRow}>
          {[{ id: 0, n: 'Chill' }, { id: 1, n: 'Normal' }, { id: 2, n: 'Hurry' }, { id: 3, n: 'Max' }, { id: 4, n: 'Sloth' }].map(p => (
            <Btn key={p.id} label={p.n} active={profilePinned && profile === p.id} disabled={d} onPress={() => onCommand(commands.profile(p.id))} />
          ))}
          <Btn label="Auto" active={!profilePinned} disabled={d} onPress={() => onCommand(commands.profileAuto())} />
        </View>
      </Section>

      {/* Speed Offset */}
      {features.speedOffset && (
        <Section title="Speed Offset" right={`${offset}% ${offsetPinned ? '(PINNED)' : '(AUTO)'}`}>
          <View style={styles.btnRow}>
            {[0, 20, 40, 60, 80, 100].map(o => (
              <Btn key={o} label={`${o}%`} active={offsetPinned && offset === o} disabled={d} onPress={() => onCommand(commands.offset(o))} />
            ))}
            <Btn label="Auto" active={!offsetPinned} disabled={d} onPress={() => onCommand(commands.offsetAuto())} />
          </View>
          <View style={styles.customRow}>
            <TextInput style={styles.customInput} keyboardType="numeric" placeholder="Custom %" placeholderTextColor={colors.textDim} value={customOffset} onChangeText={setCustomOffset} editable={connected} />
            <Btn label="Set" disabled={d || !customOffset} onPress={handleCustomOffset} />
          </View>
        </Section>
      )}
    </ScrollView>
  );
}

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
    <View style={styles.featureCard}>
      <View style={styles.featureTop}>
        <Text style={styles.featureName}>{title}</Text>
        <Text style={[styles.featureStatus, on ? styles.statusOn : styles.statusOff]}>{status}</Text>
      </View>
      <View style={styles.btnRow}>{children}</View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1 },
  content: { padding: spacing.md, gap: spacing.md },
  row: { flexDirection: 'row', gap: spacing.md },
  half: { flex: 1 },
  section: { backgroundColor: colors.surface, borderRadius: radius.md, padding: spacing.md, marginBottom: spacing.md },
  sectionHeader: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', marginBottom: spacing.sm },
  sectionTitle: { color: colors.text, fontWeight: '600', fontSize: 15 },
  sectionRight: { color: colors.textMuted, fontSize: 12 },
  healthBanner: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm, padding: spacing.sm, borderLeftWidth: 3, backgroundColor: colors.bgSecondary, borderRadius: radius.sm, marginBottom: spacing.sm },
  healthDot: { width: 8, height: 8, borderRadius: 4 },
  healthText: { color: colors.text, fontSize: 13, fontWeight: '500' },
  statGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.sm },
  stat: { width: '45%' },
  statLabel: { color: colors.textMuted, fontSize: 11 },
  statValue: { color: colors.text, fontWeight: '600', fontSize: 13 },
  featureGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.sm },
  featureCard: { backgroundColor: colors.surface, borderRadius: radius.md, padding: spacing.md, width: '48%' },
  featureTop: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', marginBottom: spacing.sm },
  featureName: { color: colors.text, fontWeight: '600', fontSize: 14 },
  featureStatus: { fontSize: 12, fontWeight: '600' },
  statusOn: { color: colors.success },
  statusOff: { color: colors.textDim },
  btnRow: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.xs },
  btn: { paddingHorizontal: 10, paddingVertical: 6, borderRadius: radius.sm, borderWidth: 1, borderColor: colors.border, backgroundColor: colors.bgSecondary },
  btnActive: { backgroundColor: colors.accent, borderColor: colors.accent },
  btnDanger: { borderColor: colors.error },
  btnText: { color: colors.text, fontSize: 12 },
  btnTextActive: { color: '#fff' },
  btnDangerText: { color: colors.error },
  btnTextDisabled: { opacity: 0.4 },
  customRow: { flexDirection: 'row', gap: spacing.sm, marginTop: spacing.sm, alignItems: 'center' },
  customInput: { borderWidth: 1, borderColor: colors.border, borderRadius: radius.sm, padding: spacing.sm, color: colors.text, fontSize: 13, width: 90 },
});
