import { useState } from "react";
import { Pressable, ScrollView, StyleSheet, Text, TextInput, View, useWindowDimensions } from "react-native";
import { colors } from "../../../design/tokens";
import type { CommandName } from "../../../hardware/controller";
import type { MonitorScreenProps } from "./types";

export function ConnectionSection(props: MonitorScreenProps) {
  const { width } = useWindowDimensions();
  const isWide = width >= 980;

  const [runnerCommand, setRunnerCommand] = useState<string>(props.availableCommands[0] ?? "status");
  const [runnerArgs, setRunnerArgs] = useState("");
  const [runnerBusy, setRunnerBusy] = useState(false);
  const [runnerFeedback, setRunnerFeedback] = useState<string | null>(null);

  const runCommandFromMonitor = async () => {
    const commandName = runnerCommand.trim();
    if (!commandName) {
      setRunnerFeedback("Enter a command name.");
      return;
    }

    if (!props.availableCommands.includes(commandName as CommandName)) {
      setRunnerFeedback(`Unknown command: ${commandName}`);
      return;
    }

    setRunnerBusy(true);
    setRunnerFeedback(null);
    try {
      await props.onRunCommand(commandName as CommandName, runnerArgs);
      setRunnerFeedback(`Executed ${commandName}${runnerArgs.trim() ? ` with args ${runnerArgs.trim()}` : ""}.`);
    } catch (error) {
      setRunnerFeedback(error instanceof Error ? error.message : "Command execution failed.");
    } finally {
      setRunnerBusy(false);
    }
  };

  return (
    <View style={styles.section}>
      <Text style={styles.title}>Connection</Text>
      <Text style={styles.subtitle}>{props.selectedTransportOption.label}</Text>

      <View style={styles.metaRow}>
        <StatusPill label="Transport" value={props.transportStatus.title} ready={props.transportStatus.tone === "ready"} />
        <StatusPill label="History" value={`${props.history.length} entries`} />
        <StatusPill label="Events" value={`${props.diagnosticsEvents.length}`} />
      </View>

      <View style={[styles.grid, isWide ? styles.gridWide : undefined]}>
        <View style={styles.col}>
          <View style={styles.card}>
            <Text style={styles.label}>Selected Transport</Text>
            <View style={[styles.statusCard, props.transportStatus.tone === "ready" ? styles.statusReady : styles.statusPending]}>
              <Text style={styles.statusTitle}>{props.transportStatus.title}</Text>
              <Text style={styles.statusDetail}>{props.transportStatus.detail}</Text>
            </View>
            <ActionButton title="Fetch Status" onPress={() => void props.onFetchStatus()} disabled={runnerBusy} />
          </View>

          <View style={styles.card}>
            <Text style={styles.label}>ESP32 BLE Device Name</Text>
            <TextInput
              style={styles.input}
              value={props.bleDeviceName}
              onChangeText={props.onBleDeviceNameInputChange}
              placeholder="TeslaCANModder"
              placeholderTextColor={colors.dashMuted}
              autoCapitalize="none"
              autoCorrect={false}
              maxLength={32}
              editable={!props.bleConfigBusy}
            />
            <View style={styles.buttonRow}>
              <ActionButton title={props.bleConfigBusy ? "Reading" : "Read BLE Status"} variant="secondary" onPress={() => void props.onRefreshBleStatus()} disabled={props.bleConfigBusy} />
              <ActionButton title={props.bleConfigBusy ? "Applying" : "Apply BLE Name"} onPress={() => void props.onApplyBleDeviceName()} disabled={props.bleConfigBusy || !props.bleDeviceName.trim()} />
            </View>
          </View>
        </View>

        <View style={styles.col}>
          <View style={styles.card}>
            <Text style={styles.label}>Command Runner</Text>
            <TextInput
              style={styles.input}
              value={runnerCommand}
              onChangeText={setRunnerCommand}
              placeholder="status"
              placeholderTextColor={colors.dashMuted}
              autoCapitalize="none"
              autoCorrect={false}
            />
            <TextInput
              style={styles.input}
              value={runnerArgs}
              onChangeText={setRunnerArgs}
              placeholder="args (optional)"
              placeholderTextColor={colors.dashMuted}
              autoCapitalize="none"
              autoCorrect={false}
            />
            <View style={styles.buttonRow}>
              <ActionButton title={runnerBusy ? "Executing" : "Execute"} onPress={() => void runCommandFromMonitor()} disabled={runnerBusy} />
              <ActionButton title="Clear" variant="secondary" onPress={() => { setRunnerArgs(""); setRunnerFeedback(null); }} disabled={runnerBusy} />
            </View>
            {runnerFeedback ? <Text style={styles.info}>{runnerFeedback}</Text> : null}
          </View>

          <View style={styles.card}>
            <Text style={styles.label}>Status Output</Text>
            <ScrollView style={styles.statusOutput}>
              <Text style={styles.statusText}>{props.statusText}</Text>
            </ScrollView>
            <Text style={styles.label}>Last Result</Text>
            <Text style={styles.info}>{props.lastResult}</Text>
          </View>
        </View>
      </View>

      <ConnectionsSettingsPanel {...props} />
    </View>
  );
}

function ConnectionsSettingsPanel(props: MonitorScreenProps) {
  const [busyCommand, setBusyCommand] = useState<string | null>(null);
  const [feedback, setFeedback] = useState<string | null>(null);
  const [mqttBroker, setMqttBroker] = useState("broker.hivemq.com");
  const [mqttPort, setMqttPort] = useState("1883");
  const [mqttInterval, setMqttInterval] = useState("1000");
  const [haInterval, setHaInterval] = useState("1500");
  const [espNowChannel, setEspNowChannel] = useState("1");
  const [gvretPort, setGvretPort] = useState("23");

  const run = async (command: string) => {
    setBusyCommand(command);
    setFeedback(null);
    try {
      const response = await props.onRunRawCommand(command);
      await props.onFetchStatus();
      setFeedback(`${command} -> ${response}`);
    } catch (error) {
      setFeedback(error instanceof Error ? error.message : `Failed: ${command}`);
    } finally {
      setBusyCommand(null);
    }
  };

  const isBusy = Boolean(busyCommand);

  return (
    <View style={styles.section}>
      <Text style={styles.title}>Integration Settings</Text>
      <Text style={styles.subtitle}>MQTT, Tesla BLE, Home Assistant, ESP-NOW, GVRET, ScanMyTesla, ELM327</Text>

      <View style={styles.metaRow}>
        <StatusPill label="MQTT" value={readStatusField(props, "mqttConnected")} />
        <StatusPill label="Tesla BLE" value={readStatusField(props, "teslaBleConnected")} />
        <StatusPill label="HA" value={readStatusField(props, "haConnected")} />
        <StatusPill label="ESP-NOW" value={readStatusField(props, "espNowPeers")} />
        <StatusPill label="GVRET" value={readStatusField(props, "gvretClients")} />
        <StatusPill label="SMT" value={readStatusField(props, "scanMyTesla")} />
        <StatusPill label="ELM327" value={readStatusField(props, "elm327")} />
      </View>

      <View style={styles.settingsGrid}>
        <View style={styles.card}>
          <Text style={styles.label}>MQTT</Text>
          <View style={styles.buttonRow}>
            <ActionButton title="On" onPress={() => void run("mqtt:on")} disabled={isBusy} compact />
            <ActionButton title="Off" variant="secondary" onPress={() => void run("mqtt:off")} disabled={isBusy} compact />
          </View>
          <TextInput style={styles.input} value={mqttBroker} onChangeText={setMqttBroker} placeholder="Broker host" placeholderTextColor={colors.dashMuted} autoCapitalize="none" autoCorrect={false} />
          <View style={styles.buttonRowWrap}>
            <ActionButton title="Set Broker" variant="secondary" onPress={() => void run(`mqtt:broker:${mqttBroker.trim()}`)} disabled={isBusy || !mqttBroker.trim()} compact />
            <TextInput style={styles.inputInline} value={mqttPort} onChangeText={setMqttPort} placeholder="Port" placeholderTextColor={colors.dashMuted} keyboardType="numeric" />
            <ActionButton title="Set Port" variant="secondary" onPress={() => void run(`mqtt:port:${mqttPort.trim()}`)} disabled={isBusy || !mqttPort.trim()} compact />
          </View>
          <View style={styles.buttonRowWrap}>
            <TextInput style={styles.inputInline} value={mqttInterval} onChangeText={setMqttInterval} placeholder="Interval ms" placeholderTextColor={colors.dashMuted} keyboardType="numeric" />
            <ActionButton title="Set Interval" variant="secondary" onPress={() => void run(`mqtt:interval:${mqttInterval.trim()}`)} disabled={isBusy || !mqttInterval.trim()} compact />
          </View>
        </View>

        <View style={styles.card}>
          <Text style={styles.label}>Tesla BLE (VCSEC)</Text>
          <View style={styles.buttonRowWrap}>
            <ActionButton title="On" onPress={() => void run("teslable:on")} disabled={isBusy} compact />
            <ActionButton title="Off" variant="secondary" onPress={() => void run("teslable:off")} disabled={isBusy} compact />
            <ActionButton title="Auth" variant="secondary" onPress={() => void run("teslable:auth")} disabled={isBusy} compact />
            <ActionButton title="Forget" variant="secondary" onPress={() => void run("teslable:forget")} disabled={isBusy} compact />
          </View>
        </View>

        <View style={styles.card}>
          <Text style={styles.label}>Home Assistant</Text>
          <View style={styles.buttonRowWrap}>
            <ActionButton title="On" onPress={() => void run("ha:on")} disabled={isBusy} compact />
            <ActionButton title="Off" variant="secondary" onPress={() => void run("ha:off")} disabled={isBusy} compact />
            <ActionButton title="Discovery" variant="secondary" onPress={() => void run("ha:discovery")} disabled={isBusy} compact />
            <TextInput style={styles.inputInline} value={haInterval} onChangeText={setHaInterval} placeholder="Interval ms" placeholderTextColor={colors.dashMuted} keyboardType="numeric" />
            <ActionButton title="Set Interval" variant="secondary" onPress={() => void run(`ha:interval:${haInterval.trim()}`)} disabled={isBusy || !haInterval.trim()} compact />
          </View>
        </View>

        <View style={styles.card}>
          <Text style={styles.label}>ESP-NOW</Text>
          <View style={styles.buttonRowWrap}>
            <ActionButton title="On" onPress={() => void run("espnow:on")} disabled={isBusy} compact />
            <ActionButton title="Off" variant="secondary" onPress={() => void run("espnow:off")} disabled={isBusy} compact />
            <TextInput style={styles.inputInline} value={espNowChannel} onChangeText={setEspNowChannel} placeholder="Channel" placeholderTextColor={colors.dashMuted} keyboardType="numeric" />
            <ActionButton title="Set Channel" variant="secondary" onPress={() => void run(`espnow:channel:${espNowChannel.trim()}`)} disabled={isBusy || !espNowChannel.trim()} compact />
          </View>
        </View>

        <View style={styles.card}>
          <Text style={styles.label}>GVRET</Text>
          <View style={styles.buttonRowWrap}>
            <ActionButton title="On" onPress={() => void run("gvret:on")} disabled={isBusy} compact />
            <ActionButton title="Off" variant="secondary" onPress={() => void run("gvret:off")} disabled={isBusy} compact />
            <TextInput style={styles.inputInline} value={gvretPort} onChangeText={setGvretPort} placeholder="Port" placeholderTextColor={colors.dashMuted} keyboardType="numeric" />
            <ActionButton title="Set Port" variant="secondary" onPress={() => void run(`gvret:port:${gvretPort.trim()}`)} disabled={isBusy || !gvretPort.trim()} compact />
          </View>
        </View>

        <View style={styles.card}>
          <Text style={styles.label}>ScanMyTesla & ELM327</Text>
          <View style={styles.buttonRowWrap}>
            <ActionButton title="SMT On" onPress={() => void run("smt:on")} disabled={isBusy} compact />
            <ActionButton title="SMT Off" variant="secondary" onPress={() => void run("smt:off")} disabled={isBusy} compact />
            <ActionButton title="ELM On" onPress={() => void run("elm327:on")} disabled={isBusy} compact />
            <ActionButton title="ELM Off" variant="secondary" onPress={() => void run("elm327:off")} disabled={isBusy} compact />
          </View>
        </View>
      </View>

      {feedback ? <Text style={styles.info}>{feedback}</Text> : null}
    </View>
  );
}

function readStatusField(props: MonitorScreenProps, key: string): string {
  const value = (props.boardState as unknown as Record<string, unknown>)[key];
  if (typeof value === "boolean") {
    return value ? "true" : "false";
  }
  if (typeof value === "number" || typeof value === "string") {
    return String(value);
  }
  if (Array.isArray(value)) {
    return `${value.length}`;
  }
  return "n/a";
}

function StatusPill({ label, value, ready = false }: { label: string; value: string; ready?: boolean }) {
  return (
    <View style={[styles.pill, ready ? styles.pillReady : undefined]}>
      <Text style={styles.pillLabel}>{label}</Text>
      <Text style={styles.pillValue}>{value}</Text>
    </View>
  );
}

function ActionButton({
  title,
  onPress,
  variant = "primary",
  disabled = false,
  compact = false,
}: {
  title: string;
  onPress: () => void;
  variant?: "primary" | "secondary";
  disabled?: boolean;
  compact?: boolean;
}) {
  return (
    <Pressable
      onPress={disabled ? undefined : onPress}
      style={({ pressed }) => [
        styles.btn,
        compact ? styles.btnCompact : undefined,
        variant === "secondary" ? styles.btnSecondary : undefined,
        disabled ? styles.btnDisabled : undefined,
        pressed && !disabled ? styles.btnPressed : undefined,
      ]}
    >
      <Text style={[styles.btnText, variant === "secondary" ? styles.btnTextSecondary : undefined]}>{title}</Text>
    </Pressable>
  );
}

const styles = StyleSheet.create({
  section: { gap: 12 },
  title: { color: colors.dashValue, fontSize: 16, fontWeight: "700" },
  subtitle: { color: colors.dashLabel, fontSize: 12 },
  metaRow: { flexDirection: "row", flexWrap: "wrap", gap: 8 },
  pill: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    backgroundColor: colors.dashCard,
    minWidth: 120,
    paddingHorizontal: 10,
    paddingVertical: 7,
    gap: 2,
  },
  pillReady: { borderColor: colors.statusConnected },
  pillLabel: { color: colors.dashMuted, fontSize: 10, textTransform: "uppercase", fontWeight: "700" },
  pillValue: { color: colors.dashValue, fontSize: 12, fontWeight: "700" },
  grid: { gap: 10 },
  gridWide: { flexDirection: "row", alignItems: "flex-start" },
  col: { flex: 1, gap: 10 },
  card: {
    backgroundColor: colors.dashCard,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 10,
    padding: 12,
    gap: 10,
  },
  label: { color: colors.dashMuted, fontSize: 11, textTransform: "uppercase", fontWeight: "700" },
  statusCard: {
    borderRadius: 8,
    borderWidth: 1,
    padding: 10,
    gap: 4,
    backgroundColor: colors.backgroundDarkSubtle,
  },
  statusReady: { borderColor: colors.statusConnected },
  statusPending: { borderColor: colors.alarmWarning },
  statusTitle: { color: colors.dashValue, fontSize: 13, fontWeight: "700" },
  statusDetail: { color: colors.dashLabel, fontSize: 12 },
  input: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    backgroundColor: colors.backgroundDarkSubtle,
    color: colors.dashValue,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  inputInline: {
    minWidth: 110,
    flexGrow: 1,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    backgroundColor: colors.backgroundDarkSubtle,
    color: colors.dashValue,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  buttonRow: { flexDirection: "row", flexWrap: "wrap", gap: 8 },
  buttonRowWrap: { flexDirection: "row", flexWrap: "wrap", gap: 8, alignItems: "center" },
  info: { color: colors.dashLabel, fontSize: 12 },
  statusOutput: {
    maxHeight: 240,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: 8,
    backgroundColor: colors.backgroundDarkSubtle,
    paddingHorizontal: 8,
    paddingVertical: 6,
  },
  statusText: { color: colors.dashValue, fontSize: 12, fontFamily: "Courier" },
  settingsGrid: { gap: 10 },
  btn: {
    borderWidth: 1,
    borderColor: colors.dashPrimary,
    backgroundColor: colors.dashPrimary,
    borderRadius: 8,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  btnCompact: { paddingVertical: 7 },
  btnSecondary: { backgroundColor: colors.backgroundDarkSubtle, borderColor: colors.dashCardBorder },
  btnDisabled: { opacity: 0.6 },
  btnPressed: { opacity: 0.85 },
  btnText: { color: colors.backgroundDark, fontSize: 12, fontWeight: "700" },
  btnTextSecondary: { color: colors.dashValue },
});
