import { View, Text, ScrollView, StyleSheet } from 'react-native';
import Slider from '@react-native-community/slider';
import { commands } from '@teslacanmodder/protocol';
import type { BoardState } from '@teslacanmodder/protocol';
import { colors, spacing } from '../styles/theme';
import { Button, Card } from './ui';

interface Props {
  state: BoardState;
  connected: boolean;
  onCommand: (cmd: string) => void;
}

interface CmdDef {
  label: string;
  cmd: () => string;
  danger?: boolean;
}

interface GroupDef {
  title: string;
  warning?: string;
  cmds?: CmdDef[];
  render?: (d: boolean, onCmd: (c: string) => void) => React.ReactNode;
}

const GROUPS: GroupDef[] = [
  {
    title: 'Mirrors',
    cmds: [
      { label: 'Fold', cmd: commands.mirrorFold },
      { label: 'Unfold', cmd: commands.mirrorUnfold },
      { label: 'Heat', cmd: commands.mirrorHeat },
      { label: 'Auto-fold', cmd: commands.mirrorAutofold },
      { label: 'Dip on Reverse', cmd: commands.mirrorDip },
    ],
  },
  {
    title: 'Locks & Horn',
    cmds: [
      { label: 'Lock', cmd: commands.lock },
      { label: 'Unlock', cmd: commands.unlock },
      { label: 'Child Lock', cmd: commands.lockChild },
      { label: 'Horn', cmd: commands.horn },
    ],
  },
  {
    title: 'Trunk & Frunk',
    cmds: [
      { label: 'Frunk Open', cmd: commands.frunkOpen },
      { label: 'Frunk Close', cmd: commands.frunkClose },
      { label: 'Trunk Open', cmd: commands.trunkOpen },
      { label: 'Trunk Close', cmd: commands.trunkClose },
      { label: 'Glovebox', cmd: commands.glovebox },
    ],
  },
  {
    title: 'Lights',
    cmds: [
      { label: 'Front Fog', cmd: commands.lightFogFront },
      { label: 'Rear Fog', cmd: commands.lightFogRear },
      { label: 'Auto High', cmd: commands.lightHighbeamAuto },
      { label: 'Ambient', cmd: commands.lightAmbient },
      { label: 'Home', cmd: commands.lightHome },
      { label: 'Dome Off', cmd: commands.lightDomeOff },
      { label: 'Dome On', cmd: commands.lightDomeOn },
      { label: 'Dome Auto', cmd: commands.lightDomeAuto },
    ],
  },
  {
    title: 'Wipers',
    cmds: [
      { label: 'Off', cmd: commands.wiperOff },
      { label: 'Speed 1', cmd: commands.wiper1 },
      { label: 'Speed 2', cmd: commands.wiper2 },
      { label: 'Speed 3', cmd: commands.wiper3 },
    ],
  },
  {
    title: 'Window & Sentry',
    cmds: [
      { label: 'Vent Open', cmd: commands.ventOpen },
      { label: 'Vent Close', cmd: commands.ventClose },
      { label: 'Sentry On', cmd: commands.sentryOn },
      { label: 'Sentry Off', cmd: commands.sentryOff },
    ],
  },
  {
    title: 'Climate',
    cmds: [
      { label: 'Keep On', cmd: commands.climateKeep },
      { label: 'Off', cmd: commands.climateOff },
    ],
  },
  {
    title: 'Charging',
    cmds: [
      { label: 'Start', cmd: commands.chargeStart },
      { label: 'Stop', cmd: commands.chargeStop },
      { label: 'Open Port', cmd: commands.chargePort },
    ],
  },
  {
    title: 'Power',
    cmds: [
      { label: 'Acc On', cmd: commands.powerAccOn },
      { label: 'Acc Off', cmd: commands.powerAccOff },
      { label: 'Drive Ready', cmd: commands.powerReady },
      { label: 'Power Off', cmd: commands.powerOff, danger: true },
    ],
  },
];

const SEATS = [
  { label: 'FL', fn: commands.seatFL },
  { label: 'FR', fn: commands.seatFR },
  { label: 'RL', fn: commands.seatRL },
  { label: 'RR', fn: commands.seatRR },
  { label: 'RC', fn: commands.seatRC },
];

export default function VehiclePanel({ state, connected, onCommand }: Props) {
  const d = !connected;

  if (state.variant === 'legacy') {
    return (
      <View style={styles.container}>
        <Card title="Vehicle Controls" warning="Limited on Legacy variant">
          <Text style={styles.info}>Vehicle commands unavailable on Legacy variant. Switch to HW3 or HW4.</Text>
        </Card>
      </View>
    );
  }

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      {GROUPS.map(group => (
        <Card key={group.title} title={group.title} warning={group.warning}>
          <View style={styles.grid}>
            {group.cmds?.map(c => (
              <Button
                key={c.label}
                label={c.label}
                variant={c.danger ? 'danger' : 'secondary'}
                disabled={d}
                compact
                onPress={() => onCommand(c.cmd())}
              />
            ))}
          </View>
        </Card>
      ))}

      <Card title="Seat Heating">
        {SEATS.map(seat => (
          <View key={seat.label} style={styles.seatRow}>
            <Text style={styles.seatLabel}>{seat.label}</Text>
            {[0, 1, 2, 3].map(l => (
              <Button key={l} label={l === 0 ? 'Off' : String(l)} disabled={d} compact onPress={() => onCommand(seat.fn(l))} />
            ))}
          </View>
        ))}
      </Card>

      <Card title="Drive Configuration">
        <Text style={styles.driveLabel}>Pedal</Text>
        <View style={styles.grid}>
          <Button label="Standard" disabled={d} compact onPress={() => onCommand(commands.pedalStandard())} />
          <Button label="Chill" disabled={d} compact onPress={() => onCommand(commands.pedalChill())} />
          <Button label="Sport" disabled={d} compact onPress={() => onCommand(commands.pedalSport())} />
        </View>
        <Text style={styles.driveLabel}>Regen</Text>
        <View style={styles.grid}>
          <Button label="Off" disabled={d} compact onPress={() => onCommand(commands.regenOff())} />
          <Button label="Low" disabled={d} compact onPress={() => onCommand(commands.regenLow())} />
          <Button label="Standard" disabled={d} compact onPress={() => onCommand(commands.regenStd())} />
          <Button label="Max" disabled={d} compact onPress={() => onCommand(commands.regenMax())} />
        </View>
        <Text style={styles.driveLabel}>Stop Mode</Text>
        <View style={styles.grid}>
          <Button label="Creep" disabled={d} compact onPress={() => onCommand(commands.stopCreep())} />
          <Button label="Roll" disabled={d} compact onPress={() => onCommand(commands.stopRoll())} />
          <Button label="Hold" disabled={d} compact onPress={() => onCommand(commands.stopHold())} />
        </View>
      </Card>

      <Card title="Display">
        <Text style={styles.driveLabel}>Brightness</Text>
        <Slider
          style={{ width: '100%', height: 36 }}
          minimumValue={0}
          maximumValue={127}
          step={1}
          value={64}
          disabled={d}
          minimumTrackTintColor={colors.accent}
          maximumTrackTintColor={colors.border}
          onSlidingComplete={(v: number) => onCommand(commands.mainDisplay(Math.round(v)))}
        />
      </Card>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1 },
  content: { padding: spacing.md, gap: spacing.sm },
  grid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.xs },
  seatRow: { flexDirection: 'row', alignItems: 'center', gap: spacing.xs, marginBottom: spacing.xs },
  seatLabel: { color: colors.textMuted, fontWeight: '600', fontSize: 12, width: 28 },
  driveLabel: { color: colors.textMuted, fontWeight: '600', fontSize: 12, marginTop: spacing.sm, marginBottom: spacing.xs },
  info: { color: colors.textMuted, fontSize: 13 },
});
