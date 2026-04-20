import { Pressable, StyleSheet, Text, TextInput, View } from "react-native";
import { colors } from "../../../design/tokens";
import type { MonitorScreenProps } from "./types";

export function DiagnosticsSection(props: MonitorScreenProps) {
  return (
    <View style={styles.section}>
      <Text style={styles.title}>Diagnostics & History</Text>
      <Text style={styles.subtitle}>{props.history.length} command(s) executed</Text>

      <View style={styles.card}>
        <Text style={styles.label}>Search</Text>
        <TextInput
          style={styles.input}
          value={props.diagnosticsQuery}
          onChangeText={props.onDiagnosticsQueryChange}
          placeholder="Filter by command"
          placeholderTextColor={colors.dashMuted}
        />
        <View style={styles.chipRow}>
          {["all", "command", "board", "snapshot", "system"].map((cat) => (
            <Pressable
              key={cat}
              onPress={() => props.onDiagnosticsCategoryChange(cat)}
              style={[styles.chip, props.diagnosticsCategory === cat ? styles.chipActive : undefined]}
            >
              <Text style={styles.chipText}>{cat}</Text>
            </Pressable>
          ))}
        </View>
      </View>

      <View style={styles.card}>
        <Text style={styles.label}>Activity Log</Text>
        <View style={styles.list}>
          {props.diagnosticsEvents.length === 0 ? (
            <Text style={styles.empty}>No diagnostics events yet</Text>
          ) : (
            props.diagnosticsEvents.slice(0, 120).map((entry) => (
              <View key={entry.id} style={styles.row}>
                <View style={styles.rowMain}>
                  <Text style={styles.rowTitle}>{entry.summary}</Text>
                  <Text style={styles.rowSub}>{`${entry.tsLabel} · ${entry.category}`}</Text>
                  <Text style={styles.rowSub}>{entry.detail}</Text>
                </View>
                <Text style={entry.ok ? styles.ok : styles.err}>{entry.ok ? "OK" : "ERR"}</Text>
              </View>
            ))
          )}
        </View>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  section: { gap: 12 },
  title: { color: colors.dashValue, fontSize: 16, fontWeight: "700" },
  subtitle: { color: colors.dashLabel, fontSize: 12 },
  card: {
    backgroundColor: colors.dashCard,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 10,
    padding: 12,
    gap: 10,
  },
  label: { color: colors.dashMuted, fontSize: 11, textTransform: "uppercase", fontWeight: "700" },
  input: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    backgroundColor: colors.backgroundDarkSubtle,
    color: colors.dashValue,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  chipRow: { flexDirection: "row", gap: 6, flexWrap: "wrap" },
  chip: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 999,
    backgroundColor: colors.backgroundDarkSubtle,
    paddingHorizontal: 10,
    paddingVertical: 5,
  },
  chipActive: { borderColor: colors.dashPrimary },
  chipText: { color: colors.dashLabel, fontSize: 12, fontWeight: "600" },
  list: { gap: 8 },
  empty: { color: colors.dashMuted, fontSize: 12 },
  row: {
    flexDirection: "row",
    alignItems: "flex-start",
    justifyContent: "space-between",
    gap: 8,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    padding: 10,
    backgroundColor: colors.backgroundDarkSubtle,
  },
  rowMain: { flex: 1, gap: 2 },
  rowTitle: { color: colors.dashValue, fontSize: 13, fontWeight: "700" },
  rowSub: { color: colors.dashLabel, fontSize: 12 },
  ok: {
    color: colors.statusConnected,
    backgroundColor: colors.successSubtle,
    borderRadius: 999,
    paddingHorizontal: 8,
    paddingVertical: 3,
    fontSize: 11,
    fontWeight: "700",
  },
  err: {
    color: colors.alarmCritical,
    backgroundColor: colors.destructiveSubtle,
    borderRadius: 999,
    paddingHorizontal: 8,
    paddingVertical: 3,
    fontSize: 11,
    fontWeight: "700",
  },
});
