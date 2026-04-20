import { Modal, Pressable, ScrollView, StyleSheet, Text, TextInput, View } from "react-native";
import Svg, { Circle, Path, Rect } from "react-native-svg";
import type { BoardState } from "@teslacanmodder/protocol";
import { colors, font, radius, spacing } from "../../design/tokens";
import { getCommandGate } from "../../state/commandGating";
import type { CommandName } from "../../hardware/controller";

export type ControlsPreset = "oem" | "performance" | "service";

interface PresetTheme {
  railBorder: string;
  railAccent: string;
  panelAccent: string;
  capsuleBg: string;
  capsuleBorder: string;
  actionAccent: string;
  warningAccent: string;
  heroTitle: string;
}

const PRESET_THEME: Record<ControlsPreset, PresetTheme> = {
  oem: {
    railBorder: colors.dashCardBorder,
    railAccent: colors.primary,
    panelAccent: colors.backgroundDarkSubtle,
    capsuleBg: colors.dashCardBorder,
    capsuleBorder: colors.dashCardBorder,
    actionAccent: colors.primary,
    warningAccent: colors.alarmWarning,
    heroTitle: colors.dashValue,
  },
  performance: {
    railBorder: colors.primary,
    railAccent: colors.primary,
    panelAccent: colors.backgroundDarkCard,
    capsuleBg: colors.backgroundDarkSubtle,
    capsuleBorder: colors.primary,
    actionAccent: colors.powerPositive,
    warningAccent: colors.alarmCritical,
    heroTitle: colors.primary,
  },
  service: {
    railBorder: colors.alarmWarning,
    railAccent: colors.alarmWarning,
    panelAccent: colors.backgroundDarkSubtle,
    capsuleBg: colors.dashBackground,
    capsuleBorder: colors.dashCardBorder,
    actionAccent: colors.alarmWarning,
    warningAccent: colors.alarmWarning,
    heroTitle: colors.alarmWarning,
  },
};

export interface CommandItemShape {
  name: CommandName;
  label: string;
  requiresArgs?: boolean;
}

export interface CommandGroupShape {
  title: string;
  busLabel: string;
  busColor: string;
  busField?: "chassisOnline" | "vehicleOnline" | "bodyOnline";
  commands: CommandItemShape[];
}

const PRESET_COLUMN_COUNT: Record<ControlsPreset, number> = {
  oem: 2,
  performance: 2,
  service: 2,
};

const PROFILE_LEVEL_NAMES = ["Chill", "Normal", "Hurry", "Max", "Sloth"] as const;

interface PaletteEntry {
  item: CommandItemShape;
  gateReason: string | null;
  pinned: boolean;
}

const GROUP_GLYPH: Record<string, "autopilot" | "drive" | "access" | "charge" | "climate" | "lights" | "wipers" | "advanced" | "system"> = {
  "Autopilot & Speed": "autopilot",
  "Drive Modes": "drive",
  "Vehicle Access": "access",
  Charging: "charge",
  Climate: "climate",
  Lights: "lights",
  Wipers: "wipers",
  "Advanced Utilities": "advanced",
  System: "system",
};

interface BusStatusBarProps {
  boardState: BoardState;
  preset: ControlsPreset;
  onOpenPalette: () => void;
}

export function BusStatusBar({ boardState, preset, onOpenPalette }: BusStatusBarProps) {
  const theme = PRESET_THEME[preset];
  return (
    <View style={[styles.busBar, { borderColor: theme.railBorder }]}>
      <View style={styles.heroStripe} />
      <View style={styles.heroHeader}>
        <View>
          <Text style={[styles.heroTitle, { color: theme.heroTitle }]}>Controls Cockpit</Text>
          <Text style={styles.heroSubtitle}>Tesla-style command deck</Text>
        </View>
        <Pressable style={[styles.paletteButton, { borderColor: theme.railAccent }]} onPress={onOpenPalette}>
          <Text style={[styles.paletteButtonText, { color: theme.railAccent }]}>Palette</Text>
        </Pressable>
      </View>
      <View style={styles.busDotsRow}>
        <BusDot label="Chassis" online={boardState.chassisOnline} />
        <BusDot label="Vehicle" online={boardState.vehicleOnline} />
        <BusDot label="Body" online={boardState.bodyOnline} />
      </View>
    </View>
  );
}

interface QuickActionsCardProps {
  boardState: BoardState;
  preset: ControlsPreset;
  commands: CommandItemShape[];
  onRunCommand: (command: CommandItemShape, gateReason: string | null) => void;
}

interface SpeedTuningCardProps {
  boardState: BoardState;
  preset: ControlsPreset;
  profileControlsTitle?: string;
  profileLevelLabel?: string;
  onSetProfile: (profile: number) => void;
  onSetProfileAuto: () => void;
  onSetOffset: (offset: number) => void;
  onSetOffsetAuto: () => void;
}

export function QuickActionsCard({ boardState, preset, commands, onRunCommand }: QuickActionsCardProps) {
  const theme = PRESET_THEME[preset];
  const columns = splitIntoColumns(commands, 2);
  return (
    <View style={[styles.quickActionsCard, { borderColor: theme.railBorder, backgroundColor: theme.panelAccent }]}>
      <View style={styles.quickHeader}>
        <View style={styles.quickTitleWrap}>
          <GroupGlyph type="system" accent={theme.actionAccent} />
          <Text style={styles.quickActionsTitle}>Quick Actions</Text>
        </View>
        <Text style={styles.quickActionsMeta}>{commands.length} pinned</Text>
      </View>
      <View style={styles.columnRack}>
        {columns.map((column, index) => (
          <View key={`quick-col-${index}`} style={styles.boxColumn}>
            {column.map((cmd) => {
              const gate = getCommandGate(cmd.name, boardState);
              return (
                <CommandGridCell
                  key={cmd.name}
                  commandName={cmd.name}
                  label={cmd.label}
                  preset={preset}
                  requiresArgs={Boolean(cmd.requiresArgs)}
                  available={gate.available}
                  fillMode="compact"
                  capsuleBackground={theme.capsuleBg}
                  capsuleBorder={theme.capsuleBorder}
                  onPress={() => onRunCommand(cmd, gate.available ? null : gate.reason)}
                />
              );
            })}
          </View>
        ))}
      </View>
    </View>
  );
}

export function SpeedTuningCard({
  boardState,
  preset,
  profileControlsTitle = "Speed Profile Controls",
  profileLevelLabel = "Profile Level",
  onSetProfile,
  onSetProfileAuto,
  onSetOffset,
  onSetOffsetAuto,
}: SpeedTuningCardProps) {
  const theme = PRESET_THEME[preset];
  const profileLevel = Math.max(0, Math.min(4, boardState.profile ?? 0));
  const profileName = PROFILE_LEVEL_NAMES[profileLevel] ?? `Level ${profileLevel}`;
  const offsetValue = Math.max(0, Math.min(100, boardState.offset ?? 0));
  const offsetFill = `${offsetValue}%` as const;

  return (
    <View style={[styles.tuningCard, { borderColor: theme.railBorder, backgroundColor: theme.panelAccent }]}>
      <Text style={styles.tuningTitle}>{profileControlsTitle}</Text>

      <View style={styles.tuningSection}>
        <View style={styles.tuningHeaderRow}>
          <Text style={styles.tuningLabel}>{profileLevelLabel}</Text>
          <Text style={styles.tuningValue}>{profileName} · {boardState.profilePinned ? "Pinned" : "Auto"}</Text>
        </View>
        <View style={styles.tuningButtonWrap}>
          {[0, 1, 2, 3, 4].map((level) => (
            <Pressable
              key={`profile-${level}`}
              onPress={() => onSetProfile(level)}
              style={({ pressed }) => [
                styles.tuningButton,
                level === profileLevel ? [styles.tuningButtonActive, { borderColor: theme.actionAccent }] : undefined,
                pressed ? styles.tuningButtonPressed : undefined,
              ]}
            >
              <Text style={[styles.tuningButtonText, level === profileLevel ? { color: theme.actionAccent } : undefined]}>
                {PROFILE_LEVEL_NAMES[level]}
              </Text>
            </Pressable>
          ))}
          <Pressable onPress={onSetProfileAuto} style={({ pressed }) => [styles.tuningButtonAuto, { borderColor: theme.actionAccent }, pressed ? styles.tuningButtonPressed : undefined]}>
            <Text style={[styles.tuningButtonText, { color: theme.actionAccent }]}>Auto</Text>
          </Pressable>
        </View>
      </View>

      <View style={styles.tuningSection}>
        <View style={styles.tuningHeaderRow}>
          <Text style={styles.tuningLabel}>Speed Offset</Text>
          <Text style={styles.tuningValue}>{offsetValue}% · {boardState.offsetPinned ? "Pinned" : "Auto"}</Text>
        </View>
        <View style={styles.tuningTrack}>
          <View style={[styles.tuningFill, { width: offsetFill, backgroundColor: theme.railAccent }]} />
        </View>
        <View style={styles.offsetStepRow}>
          <Pressable onPress={() => onSetOffset(Math.max(0, offsetValue - 5))} style={({ pressed }) => [styles.offsetStepButton, pressed ? styles.tuningButtonPressed : undefined]}>
            <Text style={styles.offsetStepText}>-5</Text>
          </Pressable>
          <Pressable onPress={() => onSetOffset(Math.min(100, offsetValue + 5))} style={({ pressed }) => [styles.offsetStepButton, pressed ? styles.tuningButtonPressed : undefined]}>
            <Text style={styles.offsetStepText}>+5</Text>
          </Pressable>
          <Pressable onPress={onSetOffsetAuto} style={({ pressed }) => [styles.offsetStepButton, { borderColor: theme.actionAccent }, pressed ? styles.tuningButtonPressed : undefined]}>
            <Text style={[styles.offsetStepText, { color: theme.actionAccent }]}>Auto</Text>
          </Pressable>
        </View>
      </View>
    </View>
  );
}

interface TooltipBannerProps {
  message: string;
  onClose: () => void;
}

export function TooltipBanner({ message, onClose }: TooltipBannerProps) {
  return (
    <View style={styles.tooltipBanner}>
      <Text style={styles.tooltipText}>⊘ {message}</Text>
      <Pressable onPress={onClose} style={styles.tooltipClose}>
        <Text style={styles.tooltipCloseText}>✕</Text>
      </Pressable>
    </View>
  );
}

interface CommandGroupCardProps {
  group: CommandGroupShape;
  boardState: BoardState;
  preset: ControlsPreset;
  onRun: (command: CommandItemShape, gateReason?: string | null) => void;
}

export function CommandGroupCard({ group, boardState, preset, onRun }: CommandGroupCardProps) {
  const theme = PRESET_THEME[preset];
  const busOnline = group.busField ? Boolean(boardState[group.busField]) : true;
  const glyph = GROUP_GLYPH[group.title] ?? "system";
  const columns = splitIntoColumns(group.commands, PRESET_COLUMN_COUNT[preset]);

  return (
    <View style={[styles.card, !busOnline ? styles.cardOffline : undefined, { borderColor: theme.railBorder, backgroundColor: theme.panelAccent }]}>
      <View style={styles.cardHeader}>
        <View style={styles.cardHeadingBlock}>
          <View style={styles.cardTitleRow}>
            <GroupGlyph type={glyph} accent={theme.actionAccent} />
            <Text style={styles.cardTitle}>{group.title}</Text>
          </View>
          <Text style={styles.cardSubtitle}>{group.commands.length} controls</Text>
        </View>
        <View style={[styles.busBadge, { borderColor: busOnline ? group.busColor : colors.statusDisconnected }]}>
          <Text style={[styles.busBadgeText, { color: busOnline ? group.busColor : colors.statusDisconnected }]}>
            {group.busLabel}
          </Text>
        </View>
      </View>
      {!busOnline && group.busField ? (
        <View style={styles.busOfflineBanner}>
          <Text style={styles.busOfflineText}>⊘  {group.busLabel} bus offline — all commands unavailable</Text>
        </View>
      ) : null}
      <View style={[styles.columnRack, !busOnline ? styles.chipRowOffline : undefined]}>
        {columns.map((column, index) => (
          <View key={`${group.title}-col-${index}`} style={styles.boxColumn}>
            {column.map((cmd) => {
              const gate = getCommandGate(cmd.name, boardState);
              return (
                <CommandGridCell
                  key={cmd.name}
                  commandName={cmd.name}
                  label={cmd.label}
                  preset={preset}
                  requiresArgs={Boolean(cmd.requiresArgs)}
                  available={gate.available}
                  fillMode="default"
                  capsuleBackground={theme.capsuleBg}
                  capsuleBorder={theme.capsuleBorder}
                  onPress={() => onRun(cmd, gate.available ? null : gate.reason)}
                />
              );
            })}
          </View>
        ))}
      </View>
    </View>
  );
}

interface CommandPaletteModalProps {
  visible: boolean;
  query: string;
  preset: ControlsPreset;
  entries: PaletteEntry[];
  onChangeQuery: (query: string) => void;
  onRun: (item: CommandItemShape, gateReason: string | null) => void;
  onTogglePin: (name: CommandName) => void;
  onRequestClose: () => void;
}

export function CommandPaletteModal({
  visible,
  query,
  preset,
  entries,
  onChangeQuery,
  onRun,
  onTogglePin,
  onRequestClose,
}: CommandPaletteModalProps) {
  const theme = PRESET_THEME[preset];
  return (
    <Modal visible={visible} transparent animationType="fade" onRequestClose={onRequestClose}>
      <View style={styles.paletteOverlay}>
        <Pressable style={styles.paletteBackdrop} onPress={onRequestClose} />
        <View style={[styles.paletteCard, { borderColor: theme.railBorder, backgroundColor: theme.panelAccent }]}>
          <View style={styles.paletteHeader}>
            <Text style={styles.paletteTitle}>Command Palette</Text>
            <Pressable onPress={onRequestClose}>
              <Text style={[styles.paletteClose, { color: theme.actionAccent }]}>Close</Text>
            </Pressable>
          </View>
          <TextInput
            value={query}
            onChangeText={onChangeQuery}
            placeholder="Search command"
            placeholderTextColor={colors.dashSecondary}
            style={[styles.paletteInput, { borderColor: theme.railBorder }]}
          />
          <ScrollView style={styles.paletteList} contentContainerStyle={styles.paletteListContent}>
            {entries.map((entry) => (
              <View key={entry.item.name} style={[styles.paletteRow, { borderColor: theme.railBorder, backgroundColor: theme.capsuleBg }]}>
                <Pressable
                  style={styles.paletteCommandArea}
                  onPress={() => {
                    onRun(entry.item, entry.gateReason);
                    onRequestClose();
                  }}
                >
                  <Text style={styles.paletteCommandLabel}>{entry.item.label}</Text>
                  <Text style={styles.paletteCommandMeta}>/{entry.item.name}</Text>
                </Pressable>
                <View style={styles.paletteActionArea}>
                  <Pressable
                    onPress={() => onTogglePin(entry.item.name)}
                    style={[styles.palettePinButton, { borderColor: theme.actionAccent }]}
                  >
                    <Text style={[styles.palettePinText, { color: theme.actionAccent }]}>{entry.pinned ? "Unpin" : "Pin"}</Text>
                  </Pressable>
                </View>
              </View>
            ))}
          </ScrollView>
        </View>
      </View>
    </Modal>
  );
}

function rowActionLabel(available: boolean, requiresArgs: boolean): string {
  if (!available) return "Blocked";
  if (requiresArgs) return "Runner";
  return "Run";
}

function splitIntoColumns<T>(items: T[], columnCount: number): T[][] {
  const safeCount = Math.max(1, Math.min(columnCount, items.length || 1));
  const columns: T[][] = Array.from({ length: safeCount }, () => []);
  items.forEach((item, index) => {
    columns[index % safeCount].push(item);
  });
  return columns;
}

function CommandGridCell({
  commandName,
  label,
  preset,
  requiresArgs,
  available,
  fillMode,
  capsuleBackground,
  capsuleBorder,
  onPress,
}: {
  commandName: CommandName;
  label: string;
  preset: ControlsPreset;
  requiresArgs: boolean;
  available: boolean;
  fillMode: "default" | "compact";
  capsuleBackground?: string;
  capsuleBorder?: string;
  onPress: () => void;
}) {
  const theme = PRESET_THEME[preset];
  const action = rowActionLabel(available, requiresArgs);
  const showMeta = preset === "service";
  return (
    <Pressable
      style={({ pressed }) => [
        styles.boxCell,
        fillMode === "compact" ? styles.boxCellCompact : undefined,
        { backgroundColor: capsuleBackground ?? theme.capsuleBg, borderColor: capsuleBorder ?? theme.capsuleBorder },
        !available ? styles.boxCellGated : undefined,
        pressed && available ? styles.chipPressed : undefined,
      ]}
      onPress={onPress}
    >
      <View style={styles.boxCellHeader}>
        <Text style={[styles.boxCellLabel, !available ? styles.chipTextGated : undefined]} numberOfLines={1}>{label}</Text>
        {!available ? <Text style={[styles.boxCellBlockedMarker, { color: theme.warningAccent }]}>⊘</Text> : null}
      </View>
      <View style={styles.boxCellFooter}>
        {showMeta ? <Text style={styles.boxCellMeta}>/{commandName}</Text> : null}
        <Text style={[styles.boxCellAction, { color: available ? theme.actionAccent : colors.dashSecondary }]}>{action}</Text>
      </View>
    </Pressable>
  );
}

function GroupGlyph({ type, accent }: { type: "autopilot" | "drive" | "access" | "charge" | "climate" | "lights" | "wipers" | "advanced" | "system"; accent: string }) {
  switch (type) {
    case "autopilot":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Path d="M9 2L15 14H3L9 2Z" stroke={accent} strokeWidth={1.5} fill="none" />
          <Circle cx="9" cy="11" r="1.4" fill={accent} />
        </Svg>
      );
    case "drive":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Circle cx="9" cy="9" r="5.5" stroke={accent} strokeWidth={1.4} fill="none" />
          <Path d="M9 9L12.5 6" stroke={accent} strokeWidth={1.4} strokeLinecap="round" />
        </Svg>
      );
    case "access":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Rect x="4" y="8" width="10" height="7" rx="1.5" stroke={accent} strokeWidth={1.4} fill="none" />
          <Path d="M6.5 8V6.5A2.5 2.5 0 0 1 9 4a2.5 2.5 0 0 1 2.5 2.5V8" stroke={accent} strokeWidth={1.4} fill="none" />
        </Svg>
      );
    case "charge":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Path d="M8 2L4 10h3l-1 6 6-9H9l2-5H8Z" fill={accent} />
        </Svg>
      );
    case "climate":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Path d="M9 3v12M3 9h12M4.5 4.5l9 9M13.5 4.5l-9 9" stroke={accent} strokeWidth={1.2} strokeLinecap="round" />
        </Svg>
      );
    case "lights":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Path d="M4 9a4 4 0 0 1 8 0 4 4 0 0 1-8 0Z" stroke={accent} strokeWidth={1.4} fill="none" />
          <Path d="M13 6l2-2M14 9h2M13 12l2 2" stroke={accent} strokeWidth={1.2} strokeLinecap="round" />
        </Svg>
      );
    case "wipers":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Path d="M3 12h12M5 10l8-4" stroke={accent} strokeWidth={1.4} strokeLinecap="round" />
          <Circle cx="6" cy="13" r="1" fill={accent} />
          <Circle cx="12" cy="13" r="1" fill={accent} />
        </Svg>
      );
    case "advanced":
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Circle cx="9" cy="9" r="2.2" stroke={accent} strokeWidth={1.4} fill="none" />
          <Path d="M9 2.5v2.1M9 13.4v2.1M2.5 9h2.1M13.4 9h2.1M4.4 4.4l1.5 1.5M12.1 12.1l1.5 1.5M13.6 4.4l-1.5 1.5M5.9 12.1l-1.5 1.5" stroke={accent} strokeWidth={1.1} strokeLinecap="round" />
        </Svg>
      );
    default:
      return (
        <Svg width={18} height={18} viewBox="0 0 18 18">
          <Rect x="3" y="3" width="12" height="12" rx="2" stroke={accent} strokeWidth={1.4} fill="none" />
          <Path d="M6 7h6M6 10h6M6 13h4" stroke={accent} strokeWidth={1.1} strokeLinecap="round" />
        </Svg>
      );
  }
}

function BusDot({ label, online }: { label: string; online: boolean }) {
  return (
    <View style={styles.busDot}>
      <View style={[styles.dot, { backgroundColor: online ? colors.statusConnected : colors.statusDisconnected }]} />
      <Text style={styles.busDotLabel}>{label}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  busBar: {
    overflow: "hidden",
    paddingVertical: spacing.sm,
    paddingHorizontal: spacing.md,
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
  },
  heroStripe: {
    position: "absolute",
    top: 0,
    left: 0,
    right: 0,
    height: 3,
    backgroundColor: colors.primary,
  },
  heroHeader: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    marginBottom: spacing.sm,
  },
  heroTitle: {
    fontSize: font.size.lg,
    fontWeight: font.weight.bold,
  },
  heroSubtitle: {
    marginTop: 2,
    color: colors.dashSecondary,
    fontSize: font.size.xs,
    letterSpacing: 0.4,
  },
  busDotsRow: {
    flexDirection: "row",
    gap: spacing.lg,
  },
  paletteButton: {
    borderWidth: 1,
    borderColor: colors.primary,
    borderRadius: radius.md,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.xs2,
    backgroundColor: colors.dashCardBorder,
  },
  paletteButtonText: {
    color: colors.primary,
    fontSize: font.size.sm,
    fontWeight: font.weight.medium,
  },
  quickActionsCard: {
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    padding: spacing.md,
    gap: spacing.sm,
  },
  tuningCard: {
    borderRadius: radius.lg,
    borderWidth: 1,
    padding: spacing.md,
    gap: spacing.md,
  },
  tuningTitle: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
    fontWeight: font.weight.semibold,
    letterSpacing: 0.5,
  },
  tuningSection: {
    gap: spacing.xs,
  },
  tuningHeaderRow: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  tuningLabel: {
    fontSize: font.size.sm,
    color: colors.dashValue,
    fontWeight: font.weight.medium,
  },
  tuningValue: {
    fontSize: font.size.xs,
    color: colors.dashSecondary,
  },
  tuningTrack: {
    height: 10,
    borderRadius: radius.full,
    overflow: "hidden",
    backgroundColor: colors.dashCardBorder,
  },
  tuningFill: {
    height: "100%",
    borderRadius: radius.full,
  },
  tuningButtonWrap: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: spacing.xs,
  },
  tuningButton: {
    minWidth: 36,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.sm,
    alignItems: "center",
    justifyContent: "center",
    paddingVertical: spacing.xs2,
    paddingHorizontal: spacing.sm,
    backgroundColor: colors.dashBackground,
  },
  tuningButtonAuto: {
    minWidth: 56,
    borderWidth: 1,
    borderRadius: radius.sm,
    alignItems: "center",
    justifyContent: "center",
    paddingVertical: spacing.xs2,
    paddingHorizontal: spacing.sm,
    backgroundColor: colors.dashBackground,
  },
  tuningButtonActive: {
    borderWidth: 1,
    backgroundColor: colors.backgroundDarkSubtle,
  },
  tuningButtonPressed: {
    opacity: 0.8,
  },
  tuningButtonText: {
    fontSize: font.size.xs,
    color: colors.dashValue,
    fontWeight: font.weight.semibold,
  },
  offsetStepRow: {
    flexDirection: "row",
    gap: spacing.xs,
  },
  offsetStepButton: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.sm,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.xs2,
    backgroundColor: colors.dashBackground,
  },
  offsetStepText: {
    fontSize: font.size.xs,
    fontWeight: font.weight.semibold,
    color: colors.dashValue,
  },
  quickHeader: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
  },
  quickTitleWrap: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.xs,
  },
  quickActionsTitle: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
    fontWeight: font.weight.medium,
    letterSpacing: 0.4,
  },
  quickActionsMeta: {
    fontSize: font.size.xs,
    color: colors.dashSecondary,
    fontWeight: font.weight.medium,
  },
  busDot: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.xs,
  },
  dot: {
    width: 8,
    height: 8,
    borderRadius: radius.full,
  },
  busDotLabel: {
    fontSize: font.size.sm,
    color: colors.dashLabel,
  },
  tooltipBanner: {
    flexDirection: "row",
    alignItems: "center",
    backgroundColor: colors.dashCard,
    borderRadius: radius.md,
    borderWidth: 1,
    borderColor: colors.alarmWarning,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    gap: spacing.sm,
  },
  tooltipText: {
    flex: 1,
    fontSize: font.size.sm,
    color: colors.alarmWarning,
  },
  tooltipClose: {
    padding: spacing.xs,
  },
  tooltipCloseText: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
  },
  card: {
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    padding: spacing.md,
    gap: spacing.sm,
  },
  cardOffline: {
    opacity: 0.6,
  },
  busOfflineBanner: {
    backgroundColor: colors.dashCardBorder,
    borderRadius: radius.sm,
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.xs2,
  },
  busOfflineText: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
  },
  chipRowOffline: {
    pointerEvents: "none" as const,
  },
  cardHeader: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
  },
  cardTitleRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.xs,
  },
  cardTitle: {
    fontSize: font.size.md,
    fontWeight: font.weight.semibold,
    color: colors.dashValue,
  },
  cardHeadingBlock: {
    gap: 2,
  },
  cardSubtitle: {
    fontSize: font.size.xs,
    color: colors.dashSecondary,
    fontWeight: font.weight.medium,
    letterSpacing: 0.4,
  },
  busBadge: {
    borderWidth: 1,
    borderRadius: radius.full,
    paddingHorizontal: spacing.sm,
    paddingVertical: 2,
  },
  busBadgeText: {
    fontSize: font.size.xs,
    fontWeight: font.weight.medium,
    letterSpacing: 0.6,
  },
  boxStack: {
    gap: spacing.xs,
  },
  columnRack: {
    flexDirection: "row",
    alignItems: "flex-start",
    gap: spacing.xs,
  },
  boxColumn: {
    flex: 1,
    minWidth: 0,
    gap: spacing.xs,
  },
  boxCell: {
    width: "100%",
    minHeight: 74,
    borderWidth: 1,
    borderRadius: radius.md,
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.sm,
    justifyContent: "space-between",
  },
  boxCellCompact: {
    minHeight: 64,
  },
  boxCellGated: {
    opacity: 0.45,
  },
  chipPressed: {
    opacity: 0.7,
  },
  boxCellHeader: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    gap: spacing.xs,
  },
  boxCellLabel: {
    fontSize: font.size.sm,
    color: colors.dashValue,
    fontWeight: font.weight.medium,
  },
  boxCellMeta: {
    fontSize: font.size.xs,
    color: colors.dashSecondary,
  },
  boxCellFooter: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    gap: spacing.xs,
  },
  boxCellAction: {
    fontSize: font.size.xs,
    fontWeight: font.weight.semibold,
    letterSpacing: 0.4,
  },
  boxCellBlockedMarker: {
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  chipTextGated: {
    color: colors.dashSecondary,
  },
  paletteOverlay: {
    flex: 1,
    justifyContent: "center",
    paddingHorizontal: spacing.lg,
  },
  paletteBackdrop: {
    position: "absolute",
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: "rgba(0,0,0,0.45)",
  },
  paletteCard: {
    maxHeight: "80%",
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    padding: spacing.md,
    gap: spacing.sm,
  },
  paletteHeader: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  paletteTitle: {
    fontSize: font.size.md,
    fontWeight: font.weight.semibold,
    color: colors.dashValue,
  },
  paletteClose: {
    fontSize: font.size.sm,
    color: colors.primary,
  },
  paletteInput: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.md,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    color: colors.dashValue,
    backgroundColor: colors.dashBackground,
  },
  paletteList: {
    maxHeight: 420,
  },
  paletteListContent: {
    gap: spacing.xs,
  },
  paletteRow: {
    flexDirection: "row",
    alignItems: "center",
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.md,
    backgroundColor: colors.dashBackground,
  },
  paletteCommandArea: {
    flex: 1,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
  },
  paletteCommandLabel: {
    fontSize: font.size.sm,
    color: colors.dashValue,
    fontWeight: font.weight.medium,
  },
  paletteCommandMeta: {
    marginTop: 2,
    fontSize: font.size.xs,
    color: colors.dashSecondary,
  },
  paletteActionArea: {
    paddingHorizontal: spacing.sm,
  },
  palettePinButton: {
    borderWidth: 1,
    borderColor: colors.primary,
    borderRadius: radius.sm,
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.xs2,
  },
  palettePinText: {
    fontSize: font.size.xs,
    color: colors.primary,
    fontWeight: font.weight.medium,
  },
});
