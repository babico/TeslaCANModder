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

export interface IntegrationPanelProps {
  state: BoardState;
}

export function IntegrationPanel({ state }: IntegrationPanelProps) {
  return (
    <View style={styles.container}>
      <Section title="Connectivity & Integrations">
        <Row label="MQTT" value={state.mqtt ? (state.mqttConnected ? "Enabled · Connected" : "Enabled · Disconnected") : "Disabled"} dimmed={!state.mqtt} />
        <Row label="Tesla BLE" value={state.teslaBle ? (state.teslaBleConnected ? (state.teslaBleAuth ? "Connected · Authenticated" : "Connected · No auth") : "Enabled · Disconnected") : "Disabled"} dimmed={!state.teslaBle} />
        <Row label="Home Assistant" value={state.homeAssistant ? (state.haConnected ? `Enabled · Connected (${state.haEntities} entities)` : `Enabled · Disconnected (${state.haEntities} entities)`) : "Disabled"} dimmed={!state.homeAssistant} />
        <Row label="ESP-NOW" value={state.espNow ? `Enabled · CH ${state.espNowChannel} · ${state.espNowPeers} peers` : "Disabled"} dimmed={!state.espNow} />
        <Row label="GVRET" value={state.gvret ? `Enabled · Port ${state.gvretPort} · ${state.gvretClients} clients` : "Disabled"} dimmed={!state.gvret} />
        <Row label="ScanMyTesla" value={state.scanMyTesla ? "Enabled" : "Disabled"} dimmed={!state.scanMyTesla} />
        <Row label="ELM327" value={state.elm327 ? "Enabled" : "Disabled"} dimmed={!state.elm327} />
      </Section>

      <Section title="Integration Notes">
        <Row label="MQTT / Home Assistant" value="Bridge state shown from firmware" />
        <Row label="Tesla BLE" value="Auth and link status surfaced separately" />
        <Row label="GVRET / ESP-NOW" value="Runtime peers/clients indicate active consumers" />
        <Row label="ScanMyTesla / ELM327" value="Compatibility modes are read from board state" />
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
