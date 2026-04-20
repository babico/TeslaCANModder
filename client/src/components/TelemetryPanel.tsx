/**
 * TelemetryPanel — E-05
 *
 * Extended diagnostics panel for the Monitor tab.
 * Sections:
 *  1. BMS Extended   — temp range, HV state, contactor state, max regen/discharge
 *  2. TPMS           — 4-wheel pressure + temp (gated by hasTpms)
 *  3. Powertrain     — gated by hasPowertrain
 *  4. Firmware       — fwCompat level, steering mode
 *  5. CAN Health     — per-bus on/detected flags
 */

import { StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { colors, font, radius, spacing } from "../design/tokens";

// ── Helpers ───────────────────────────────────────────────────────────────────

function pressureBar(raw: number): string {
  return (raw / 100).toFixed(2);
}

const HV_STATE_LABELS: Record<number, string> = {
  0: "Idle",
  1: "Pre-charge",
  2: "Active",
  3: "Discharge",
  4: "Fault",
  5: "Sleep",
};
function hvStateLabel(val: number): string {
  return HV_STATE_LABELS[val] ?? `State ${val}`;
}

const CONTACTOR_LABELS: Record<number, string> = {
  0: "Open",
  1: "Closed",
  2: "Pre-charge",
  3: "Fault",
};
function contactorLabel(val: number): string {
  return CONTACTOR_LABELS[val] ?? `State ${val}`;
}

const STEERING_MODE_LABELS: Record<number, string> = {
  0: "Normal",
  1: "Sport",
  2: "Comfort",
};
function steeringModeLabel(val: number): string {
  return STEERING_MODE_LABELS[val] ?? `Mode ${val}`;
}

// ── SubSection ────────────────────────────────────────────────────────────────

function Section({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <View style={styles.section}>
      <Text style={styles.sectionTitle}>{title}</Text>
      <View style={styles.grid}>{children}</View>
    </View>
  );
}

function Row({ label, value, dimmed }: { label: string; value: string; dimmed?: boolean }) {
  return (
    <View style={styles.row}>
      <Text style={styles.rowLabel}>{label}</Text>
      <Text style={[styles.rowValue, dimmed ? styles.rowValueDimmed : undefined]}>{value}</Text>
    </View>
  );
}

// ── TelemetryPanel ────────────────────────────────────────────────────────────

export interface TelemetryPanelProps {
  state: BoardState;
}

export function TelemetryPanel({ state }: TelemetryPanelProps) {
  const canEntries = Object.entries(state.canHealth);

  return (
    <View style={styles.container}>
      {/* Section 1 — BMS Extended */}
      {state.hasBms ? (
        <Section title="BMS Extended">
          <Row label="Temp Range" value={`${state.bmsTempMin.toFixed(0)}–${state.bmsTempMax.toFixed(0)} °C`} />
          <Row label="HV State" value={hvStateLabel(state.bmsHvState)} />
          <Row label="Contactor" value={contactorLabel(state.bmsContactorState)} />
          <Row label="Max Regen" value={`${state.bmsMaxRegenPower.toFixed(0)} kW`} />
          <Row label="Max Discharge" value={`${state.bmsMaxDischargePower.toFixed(0)} kW`} />
          <Row label="Bus Voltage" value={`${state.bmsMinBusVoltage.toFixed(0)}–${state.bmsMaxBusVoltage.toFixed(0)} V`} />
        </Section>
      ) : null}

      {/* Section 2 — TPMS */}
      {state.hasTpms ? (
        <Section title="TPMS">
          {(["FL", "FR", "RL", "RR"] as const).map((pos) => {
            const pressure = pressureBar((state as unknown as Record<string, number>)[`tpmsPressure${pos}`] as number);
            const temp = ((state as unknown as Record<string, number>)[`tpmsTemp${pos}`] as number).toFixed(0);
            const low = parseFloat(pressure) < 1.8;
            return (
              <View key={pos} style={styles.tpmsCell}>
                <Text style={styles.tpmsPos}>{pos}</Text>
                <Text style={[styles.tpmsPressure, low ? styles.tpmsLow : undefined]}>{pressure} bar</Text>
                <Text style={styles.tpmsTemp}>{temp} °C</Text>
              </View>
            );
          })}
        </Section>
      ) : null}

      {/* Section 3 — Powertrain */}
      {state.hasPowertrain ? (
        <Section title="Powertrain">
          <Row label="Drive Mode" value={String(state.driveMode)} />
          <Row label="Power" value={`${state.bmsPower.toFixed(1)} kW`} />
        </Section>
      ) : null}

      {/* Section 4 — Firmware */}
      <Section title="Firmware">
        <Row label="FW Compat" value={String(state.fwCompat)} />
        {state.hasSteeringMode ? (
          <Row label="Steering Mode" value={steeringModeLabel(state.steeringMode)} />
        ) : null}
        <Row label="Uptime" value={`${state.uptime} s`} />
        <Row label="Rate" value={`${state.rate} msg/s`} />
      </Section>

      {/* Section 5 — CAN Health */}
      {canEntries.length > 0 ? (
        <Section title="CAN Health">
          {canEntries.map(([bus, health]) => (
            <Row
              key={bus}
              label={bus.toUpperCase()}
              value={health.on ? (health.det ? "On · Detected" : "On · Not detected") : "Offline"}
              dimmed={!health.on}
            />
          ))}
        </Section>
      ) : null}
    </View>
  );
}

// ── Styles ────────────────────────────────────────────────────────────────────

const styles = StyleSheet.create({
  container: {
    gap: spacing.md,
  },
  section: {
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    padding: spacing.md,
    gap: spacing.sm,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
  },
  sectionTitle: {
    color: colors.dashSecondary,
    fontSize: font.size.xs,
    fontWeight: font.weight.bold,
    textTransform: "uppercase",
    letterSpacing: 0.7,
    marginBottom: spacing.xs2,
  },
  grid: {
    gap: spacing.xs,
  },
  row: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  rowLabel: {
    color: colors.dashSecondary,
    fontSize: font.size.sm,
  },
  rowValue: {
    color: colors.dashValue,
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  rowValueDimmed: {
    color: colors.dashMuted,
  },
  tpmsCell: {
    alignItems: "center",
    flex: 1,
    padding: spacing.xs,
    backgroundColor: colors.backgroundDarkSubtle,
    borderRadius: radius.md,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    gap: spacing.xs2,
    flexDirection: "row",
    justifyContent: "space-between",
  },
  tpmsPos: {
    color: colors.dashSecondary,
    fontSize: font.size.xs,
    fontWeight: font.weight.bold,
    width: 24,
  },
  tpmsPressure: {
    color: colors.dashValue,
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  tpmsLow: {
    color: colors.alarmWarning,
  },
  tpmsTemp: {
    color: colors.dashSecondary,
    fontSize: font.size.xs,
  },
});
