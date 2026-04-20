import { StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { colors, font, radius, spacing } from "../design/tokens";

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

export interface UtilityPanelProps {
  state: BoardState;
}

export function UtilityPanel({ state }: UtilityPanelProps) {
  return (
    <View style={styles.container}>
      <Section title="Utility Feature Status">
        <Row label="Seatbelt Emulation" value={state.seatbeltEmulation ? "Supported" : "Unavailable"} dimmed={!state.seatbeltEmulation} />
        <Row label="Speed Alert BLE" value={state.speedAlert ? "Enabled" : "Disabled"} dimmed={!state.speedAlert} />
        <Row label="CAN Simulation" value={state.canSim ? "Enabled" : "Disabled"} dimmed={!state.canSim} />
        <Row label="Single-shot TX" value={state.singleShot ? "Enabled" : "Disabled"} dimmed={!state.singleShot} />
        <Row label="Wiper Persistence" value={state.wiperPersist ? "Enabled" : "Disabled"} dimmed={!state.wiperPersist} />
        <Row label="Mirror Auto-fold" value={state.mirrorAutoFold ? "Enabled" : "Disabled"} dimmed={!state.mirrorAutoFold} />
      </Section>

      <Section title="Button Remapping">
        <Row label="Support" value={state.hasBtnMap ? "Available" : "Unavailable"} dimmed={!state.hasBtnMap} />
        {state.hasBtnMap ? (
          <>
            <Row label="Lamp Short" value={state.btnMapLampShort} />
            <Row label="Lamp Long" value={state.btnMapLampLong} />
            <Row label="Lamp Double" value={state.btnMapLampDouble} />
            <Row label="Park Short" value={state.btnMapParkShort} />
            <Row label="Park Long" value={state.btnMapParkLong} />
            <Row label="Park Double" value={state.btnMapParkDouble} />
          </>
        ) : null}
      </Section>

      <Section title="Advanced Utility Control Notes">
        <Row label="Quick Actions" value="canSimStart/canSimStop, btnMapQuery, btnMapReset" />
        <Row label="Parameterized" value="singleShot, seatbelt, speedAlert, wiperPersist, mirrorAutoFold, btnMap" />
        <Row label="Safe Flow" value="Use Controls palette or Monitor command runner for parameterized commands" />
      </Section>
    </View>
  );
}

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
    flex: 1,
  },
  rowValue: {
    color: colors.dashValue,
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
    textAlign: "right",
    flex: 1,
  },
  rowValueDimmed: {
    color: colors.dashMuted,
  },
});
