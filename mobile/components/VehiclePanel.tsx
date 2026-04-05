import { View, Text, TouchableOpacity, ScrollView, StyleSheet } from 'react-native';
import Slider from '@react-native-community/slider';
import { commands } from '../lib/protocol/commands';
import type { BoardState } from '../hooks/useBoardState';
import { colors, spacing, radius } from '../styles/theme';

interface Props {
  state: BoardState;
  connected: boolean;
  onCommand: (cmd: string) => void;
}

function Btn({ label, disabled, danger, onPress }: { label: string; disabled?: boolean; danger?: boolean; onPress: () => void }) {
  return (
    <TouchableOpacity style={[s.btn, danger && s.btnDanger]} disabled={disabled} onPress={onPress}>
      <Text style={[s.btnText, danger && s.btnDangerText, disabled && s.btnDisabled]}>{label}</Text>
    </TouchableOpacity>
  );
}

function Card({ title, warning, children }: { title: string; warning?: string; children: React.ReactNode }) {
  return (
    <View style={s.card}>
      <View style={s.cardHeader}>
        <Text style={s.cardTitle}>{title}</Text>
        {warning ? <Text style={s.cardWarning}>{warning}</Text> : null}
      </View>
      <View style={s.cardBody}>{children}</View>
    </View>
  );
}

function BtnGrid({ children }: { children: React.ReactNode }) {
  return <View style={s.grid}>{children}</View>;
}

export default function VehiclePanel({ state, connected, onCommand }: Props) {
  const d = !connected;
  const isLegacy = state.variant === 'legacy';

  if (isLegacy) {
    return (
      <View style={s.container}>
        <Card title="Vehicle Controls" warning="Limited on Legacy variant">
          <Text style={s.info}>Vehicle commands unavailable on Legacy variant. Switch to HW3 or HW4.</Text>
        </Card>
      </View>
    );
  }

  return (
    <ScrollView style={s.container} contentContainerStyle={s.content}>
      <Card title="Mirrors">
        <BtnGrid>
          <Btn label="Fold" disabled={d} onPress={() => onCommand(commands.mirrorFold())} />
          <Btn label="Unfold" disabled={d} onPress={() => onCommand(commands.mirrorUnfold())} />
          <Btn label="Heat" disabled={d} onPress={() => onCommand(commands.mirrorHeat())} />
          <Btn label="Auto-fold" disabled={d} onPress={() => onCommand(commands.mirrorAutofold())} />
          <Btn label="Dip on Reverse" disabled={d} onPress={() => onCommand(commands.mirrorDip())} />
        </BtnGrid>
      </Card>

      <Card title="Locks & Horn">
        <BtnGrid>
          <Btn label="Lock" disabled={d} onPress={() => onCommand(commands.lock())} />
          <Btn label="Unlock" disabled={d} onPress={() => onCommand(commands.unlock())} />
          <Btn label="Child Lock" disabled={d} onPress={() => onCommand(commands.lockChild())} />
          <Btn label="Horn" disabled={d} onPress={() => onCommand(commands.horn())} />
        </BtnGrid>
      </Card>

      <Card title="Trunk & Frunk">
        <BtnGrid>
          <Btn label="Frunk Open" disabled={d} onPress={() => onCommand(commands.frunkOpen())} />
          <Btn label="Frunk Close" disabled={d} onPress={() => onCommand(commands.frunkClose())} />
          <Btn label="Trunk Open" disabled={d} onPress={() => onCommand(commands.trunkOpen())} />
          <Btn label="Trunk Close" disabled={d} onPress={() => onCommand(commands.trunkClose())} />
          <Btn label="Glovebox" disabled={d} onPress={() => onCommand(commands.glovebox())} />
        </BtnGrid>
      </Card>

      <Card title="Lights">
        <BtnGrid>
          <Btn label="Front Fog" disabled={d} onPress={() => onCommand(commands.lightFogFront())} />
          <Btn label="Rear Fog" disabled={d} onPress={() => onCommand(commands.lightFogRear())} />
          <Btn label="Auto High" disabled={d} onPress={() => onCommand(commands.lightHighbeamAuto())} />
          <Btn label="Ambient" disabled={d} onPress={() => onCommand(commands.lightAmbient())} />
          <Btn label="Home" disabled={d} onPress={() => onCommand(commands.lightHome())} />
          <Btn label="Dome Off" disabled={d} onPress={() => onCommand(commands.lightDomeOff())} />
          <Btn label="Dome On" disabled={d} onPress={() => onCommand(commands.lightDomeOn())} />
          <Btn label="Dome Auto" disabled={d} onPress={() => onCommand(commands.lightDomeAuto())} />
        </BtnGrid>
      </Card>

      <Card title="Wipers">
        <BtnGrid>
          <Btn label="Off" disabled={d} onPress={() => onCommand(commands.wiperOff())} />
          <Btn label="Speed 1" disabled={d} onPress={() => onCommand(commands.wiper1())} />
          <Btn label="Speed 2" disabled={d} onPress={() => onCommand(commands.wiper2())} />
          <Btn label="Speed 3" disabled={d} onPress={() => onCommand(commands.wiper3())} />
        </BtnGrid>
      </Card>

      <Card title="Seat Heating">
        {[
          { label: 'FL', fn: commands.seatFL },
          { label: 'FR', fn: commands.seatFR },
          { label: 'RL', fn: commands.seatRL },
          { label: 'RR', fn: commands.seatRR },
          { label: 'RC', fn: commands.seatRC },
        ].map(seat => (
          <View key={seat.label} style={s.seatRow}>
            <Text style={s.seatLabel}>{seat.label}</Text>
            {[0, 1, 2, 3].map(l => (
              <Btn key={l} label={l === 0 ? 'Off' : String(l)} disabled={d} onPress={() => onCommand(seat.fn(l))} />
            ))}
          </View>
        ))}
      </Card>

      <Card title="Window & Sentry">
        <BtnGrid>
          <Btn label="Vent Open" disabled={d} onPress={() => onCommand(commands.ventOpen())} />
          <Btn label="Vent Close" disabled={d} onPress={() => onCommand(commands.ventClose())} />
          <Btn label="Sentry On" disabled={d} onPress={() => onCommand(commands.sentryOn())} />
          <Btn label="Sentry Off" disabled={d} onPress={() => onCommand(commands.sentryOff())} />
        </BtnGrid>
      </Card>

      <Card title="Climate">
        <BtnGrid>
          <Btn label="Keep On" disabled={d} onPress={() => onCommand(commands.climateKeep())} />
          <Btn label="Off" disabled={d} onPress={() => onCommand(commands.climateOff())} />
        </BtnGrid>
      </Card>

      <Card title="Charging">
        <BtnGrid>
          <Btn label="Start" disabled={d} onPress={() => onCommand(commands.chargeStart())} />
          <Btn label="Stop" disabled={d} onPress={() => onCommand(commands.chargeStop())} />
          <Btn label="Open Port" disabled={d} onPress={() => onCommand(commands.chargePort())} />
        </BtnGrid>
      </Card>

      <Card title="Drive Configuration">
        <Text style={s.driveLabel}>Pedal</Text>
        <BtnGrid>
          <Btn label="Standard" disabled={d} onPress={() => onCommand(commands.pedalStandard())} />
          <Btn label="Chill" disabled={d} onPress={() => onCommand(commands.pedalChill())} />
          <Btn label="Sport" disabled={d} onPress={() => onCommand(commands.pedalSport())} />
        </BtnGrid>
        <Text style={s.driveLabel}>Regen</Text>
        <BtnGrid>
          <Btn label="Off" disabled={d} onPress={() => onCommand(commands.regenOff())} />
          <Btn label="Low" disabled={d} onPress={() => onCommand(commands.regenLow())} />
          <Btn label="Standard" disabled={d} onPress={() => onCommand(commands.regenStd())} />
          <Btn label="Max" disabled={d} onPress={() => onCommand(commands.regenMax())} />
        </BtnGrid>
        <Text style={s.driveLabel}>Stop Mode</Text>
        <BtnGrid>
          <Btn label="Creep" disabled={d} onPress={() => onCommand(commands.stopCreep())} />
          <Btn label="Roll" disabled={d} onPress={() => onCommand(commands.stopRoll())} />
          <Btn label="Hold" disabled={d} onPress={() => onCommand(commands.stopHold())} />
        </BtnGrid>
      </Card>

      <Card title="Display">
        <Text style={s.driveLabel}>Brightness</Text>
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

      <Card title="Power">
        <BtnGrid>
          <Btn label="Acc On" disabled={d} onPress={() => onCommand(commands.powerAccOn())} />
          <Btn label="Acc Off" disabled={d} onPress={() => onCommand(commands.powerAccOff())} />
          <Btn label="Drive Ready" disabled={d} onPress={() => onCommand(commands.powerReady())} />
          <Btn label="Power Off" disabled={d} danger onPress={() => onCommand(commands.powerOff())} />
        </BtnGrid>
      </Card>
    </ScrollView>
  );
}

const s = StyleSheet.create({
  container: { flex: 1 },
  content: { padding: spacing.md, gap: spacing.md },
  card: { backgroundColor: colors.surface, borderRadius: radius.md, overflow: 'hidden', marginBottom: spacing.sm },
  cardHeader: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', padding: spacing.sm, borderBottomWidth: 1, borderBottomColor: colors.border },
  cardTitle: { color: colors.text, fontWeight: '600', fontSize: 14 },
  cardWarning: { color: colors.warning, fontSize: 11 },
  cardBody: { padding: spacing.sm },
  grid: { flexDirection: 'row', flexWrap: 'wrap', gap: spacing.xs },
  btn: { paddingHorizontal: 10, paddingVertical: 6, borderRadius: radius.sm, borderWidth: 1, borderColor: colors.border, backgroundColor: colors.bgSecondary },
  btnDanger: { borderColor: colors.error },
  btnText: { color: colors.text, fontSize: 12 },
  btnDangerText: { color: colors.error },
  btnDisabled: { opacity: 0.4 },
  seatRow: { flexDirection: 'row', alignItems: 'center', gap: spacing.xs, marginBottom: spacing.xs },
  seatLabel: { color: colors.textMuted, fontWeight: '600', fontSize: 12, width: 24 },
  driveLabel: { color: colors.textMuted, fontWeight: '600', fontSize: 12, marginTop: spacing.sm, marginBottom: spacing.xs },
  info: { color: colors.textMuted, fontSize: 13 },
});
