import { useMemo, useState } from "react";
import {
  Pressable,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  View,
} from "react-native";

import { colors, font, radius, spacing } from "../design/tokens";
import {
  MONITOR_TRANSPORT_OPTIONS,
  type MonitorTransportType,
} from "../hardware/transportPresentation";
import {
  CONNECTION_PRESETS,
  useBoardConnection,
} from "../state/BoardConnectionContext";
import { Sheet } from "../ui/Sheet";

const DISPLAY_TRANSPORT_OPTIONS = MONITOR_TRANSPORT_OPTIONS.filter(
  (option) => option.id !== "bluetooth-serial",
);

function normalizeTransportType(type: MonitorTransportType): MonitorTransportType {
  return type === "bluetooth-serial" ? "serial" : type;
}

function formatTransportLabel(type: MonitorTransportType): string {
  return DISPLAY_TRANSPORT_OPTIONS.find((option) => option.id === normalizeTransportType(type))?.label
    ?? type.toUpperCase();
}

function buildTransportHint(type: MonitorTransportType, baseUrl: string): string {
  switch (normalizeTransportType(type)) {
    case "http":
      return baseUrl;
    case "ble":
      return "Web Bluetooth NUS picker";
    case "serial":
      return "USB serial or Bluetooth COM picker";
    default:
      return type;
  }
}

export function ConnectionHeader() {
  const conn = useBoardConnection();
  const [sheetOpen, setSheetOpen] = useState(false);

  const visualTransportType = normalizeTransportType(conn.selectedTransportType);
  const isReady = conn.isSelectedTransportReady;
  const statusColor = conn.connectionBusy
    ? colors.statusConnecting
    : isReady
      ? colors.statusConnected
      : colors.statusDisconnected;
  const statusLabel = conn.connectionBusy
    ? "Connecting"
    : isReady
      ? "Connected"
      : "Disconnected";

  const transportLabel = useMemo(
    () => formatTransportLabel(conn.selectedTransportType),
    [conn.selectedTransportType],
  );
  const transportHint = useMemo(
    () => buildTransportHint(conn.selectedTransportType, conn.baseUrl),
    [conn.selectedTransportType, conn.baseUrl],
  );

  async function handleApplyConnection(): Promise<void> {
    await conn.applyConnection();
    setSheetOpen(false);
  }

  return (
    <>
      <View style={styles.bar}>
        <View style={styles.barLeft}>
          <Text style={styles.appName}>Tesla CAN Modder</Text>
          <Text style={styles.appSubline}>Shared board connection</Text>
        </View>

        <View style={styles.barCenter}>
          <Text style={styles.transportLabel}>{transportLabel}</Text>
          <Text style={styles.urlLabel} numberOfLines={1}>{transportHint}</Text>
        </View>

        <Pressable style={styles.statusPill} onPress={() => setSheetOpen(true)}>
          <View style={[styles.dot, { backgroundColor: statusColor }]} />
          <Text style={[styles.statusText, { color: statusColor }]}>{statusLabel}</Text>
        </Pressable>
      </View>

      <Sheet
        visible={sheetOpen}
        onClose={() => setSheetOpen(false)}
        title="Board Connection"
        estimatedHeight={560}
      >
        <ScrollView contentContainerStyle={styles.sheetContent}>
          <View style={styles.callout}>
            <Text style={styles.calloutTitle}>{transportLabel}</Text>
            <Text style={styles.calloutBody}>{transportHint}</Text>
          </View>

          <Text style={styles.fieldLabel}>Transport</Text>
          <View style={styles.chipRow}>
            {DISPLAY_TRANSPORT_OPTIONS.map((option) => {
              const active = option.id === visualTransportType;
              return (
                <Pressable
                  key={option.id}
                  onPress={() => conn.setSelectedTransportType(option.id)}
                  style={[styles.chip, active ? styles.chipActive : undefined]}
                >
                  <Text style={[styles.chipText, active ? styles.chipTextActive : undefined]}>
                    {option.label}
                  </Text>
                  <Text style={styles.chipDetail}>{option.detail}</Text>
                </Pressable>
              );
            })}
          </View>

          {visualTransportType === "http" ? (
            <>
              <Text style={styles.fieldLabel}>Base URL</Text>
              <TextInput
                style={styles.input}
                value={conn.baseUrl}
                onChangeText={conn.setBaseUrl}
                placeholder="http://192.168.4.1"
                placeholderTextColor={colors.foregroundDarkSubtle}
                autoCapitalize="none"
                autoCorrect={false}
              />

              <Text style={styles.fieldLabel}>Command Path</Text>
              <TextInput
                style={styles.input}
                value={conn.commandPath}
                onChangeText={conn.setCommandPath}
                placeholder="/api/command"
                placeholderTextColor={colors.foregroundDarkSubtle}
                autoCapitalize="none"
                autoCorrect={false}
              />

              <Text style={styles.fieldLabel}>Status Path</Text>
              <TextInput
                style={styles.input}
                value={conn.statusPath}
                onChangeText={conn.setStatusPath}
                placeholder="/api/status"
                placeholderTextColor={colors.foregroundDarkSubtle}
                autoCapitalize="none"
                autoCorrect={false}
              />

              <Text style={styles.fieldLabel}>Quick Presets</Text>
              <View style={styles.presetRow}>
                {CONNECTION_PRESETS.map((preset) => (
                  <Pressable
                    key={preset.name}
                    onPress={() => {
                      void conn.applyPreset(preset);
                      setSheetOpen(false);
                    }}
                    style={styles.presetChip}
                  >
                    <Text style={styles.presetText}>{preset.name}</Text>
                  </Pressable>
                ))}
              </View>
            </>
          ) : null}

          {visualTransportType === "serial" ? (
            <View style={styles.helperCard}>
              <Text style={styles.helperTitle}>Serial / COM</Text>
              <Text style={styles.helperBody}>
                Uses the browser serial picker. This covers direct USB serial and Bluetooth COM ports exposed by the OS.
              </Text>
            </View>
          ) : null}

          {visualTransportType === "ble" ? (
            <View style={styles.helperCard}>
              <Text style={styles.helperTitle}>BLE (NUS)</Text>
              <Text style={styles.helperBody}>
                Uses Web Bluetooth and the Nordic UART Service UUIDs documented in the BLE adapter reference.
              </Text>
            </View>
          ) : null}

          <View style={styles.actionRow}>
            <Pressable
              style={[styles.connectBtn, conn.connectionBusy ? styles.buttonDisabled : undefined]}
              onPress={() => void handleApplyConnection()}
              disabled={conn.connectionBusy}
            >
              <Text style={styles.connectBtnText}>
                {conn.connectionBusy ? "Connecting..." : visualTransportType === "http" ? "Apply Connection" : "Open Picker and Connect"}
              </Text>
            </Pressable>
            <Pressable style={styles.closeBtn} onPress={() => setSheetOpen(false)}>
              <Text style={styles.closeBtnText}>Close</Text>
            </Pressable>
          </View>

          <Text style={styles.lastResult} numberOfLines={3}>{conn.lastResult}</Text>
        </ScrollView>
      </Sheet>
    </>
  );
}

const styles = StyleSheet.create({
  bar: {
    flexDirection: "row",
    alignItems: "center",
    backgroundColor: colors.dashCard,
    borderBottomWidth: 1,
    borderBottomColor: colors.dashCardBorder,
    paddingHorizontal: spacing.lg,
    paddingVertical: spacing.md,
    gap: spacing.md,
  },
  barLeft: {
    minWidth: 118,
    gap: spacing.xs2,
  },
  appName: {
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
    color: colors.dashValue,
    letterSpacing: 0.3,
  },
  appSubline: {
    color: colors.dashLabel,
    fontSize: font.size.sm2,
  },
  barCenter: {
    flex: 1,
    gap: spacing.xs2,
  },
  transportLabel: {
    fontSize: font.size.sm2,
    fontWeight: font.weight.bold,
    color: colors.dashLabel,
    textTransform: "uppercase",
    letterSpacing: 0.7,
  },
  urlLabel: {
    fontSize: font.size.md2,
    color: colors.dashValue,
    fontFamily: "Courier",
  },
  statusPill: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.sm,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    backgroundColor: colors.dashBackground,
    borderRadius: radius.full,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
  },
  dot: {
    width: 8,
    height: 8,
    borderRadius: radius.full,
  },
  statusText: {
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  sheetContent: {
    gap: spacing.md,
    paddingBottom: spacing.md,
  },
  callout: {
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    backgroundColor: colors.backgroundDarkSubtle,
    padding: spacing.md,
    gap: spacing.xs,
  },
  calloutTitle: {
    color: colors.dashValue,
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
  },
  calloutBody: {
    color: colors.dashLabel,
    fontSize: font.size.md2,
    lineHeight: 18,
  },
  fieldLabel: {
    fontSize: font.size.sm2,
    fontWeight: font.weight.bold,
    color: colors.dashLabel,
    textTransform: "uppercase",
    letterSpacing: 0.7,
  },
  input: {
    backgroundColor: colors.dashBackground,
    borderRadius: radius.sm,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.md2,
    fontSize: font.size.md,
    color: colors.dashValue,
  },
  chipRow: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: spacing.sm,
  },
  chip: {
    minWidth: 156,
    flexGrow: 1,
    gap: spacing.xs,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.md,
    backgroundColor: colors.dashBackground,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
  },
  chipActive: {
    backgroundColor: colors.backgroundDarkSubtle,
    borderColor: colors.dashPrimary,
  },
  chipText: {
    fontSize: font.size.md2,
    fontWeight: font.weight.semibold,
    color: colors.dashValue,
  },
  chipTextActive: {
    color: colors.dashPrimary,
  },
  chipDetail: {
    fontSize: font.size.sm,
    color: colors.dashLabel,
    lineHeight: 16,
  },
  presetRow: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: spacing.sm,
  },
  presetChip: {
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    backgroundColor: colors.dashBackground,
    borderRadius: radius.full,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
  },
  presetText: {
    color: colors.dashValue,
    fontSize: font.size.sm,
    fontWeight: font.weight.medium,
  },
  helperCard: {
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    backgroundColor: colors.dashBackground,
    padding: spacing.md,
    gap: spacing.xs,
  },
  helperTitle: {
    color: colors.dashValue,
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
  },
  helperBody: {
    color: colors.dashLabel,
    fontSize: font.size.md2,
    lineHeight: 18,
  },
  actionRow: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: spacing.sm,
  },
  connectBtn: {
    flexGrow: 1,
    minWidth: 200,
    alignItems: "center",
    borderRadius: radius.md,
    backgroundColor: colors.dashPrimary,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.md,
  },
  buttonDisabled: {
    opacity: 0.6,
  },
  connectBtnText: {
    color: colors.backgroundDark,
    fontSize: font.size.md2,
    fontWeight: font.weight.bold,
  },
  closeBtn: {
    alignItems: "center",
    justifyContent: "center",
    minWidth: 120,
    borderRadius: radius.md,
    backgroundColor: colors.dashBackground,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.md,
  },
  closeBtnText: {
    color: colors.dashValue,
    fontSize: font.size.md2,
    fontWeight: font.weight.semibold,
  },
  lastResult: {
    color: colors.dashLabel,
    fontSize: font.size.sm,
    lineHeight: 18,
  },
});
