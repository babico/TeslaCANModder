/**
 * ChargeStatusCard — compact charge telemetry card for the Drive screen.
 *
 * Shows the resolved charge state with appropriate color coding and key metrics.
 * Renders nothing meaningful when state is "unavailable" (hasBms false).
 *
 * Layout:
 *   ┌──────────────────────────────┐
 *   │ ⚡ CHARGING · 7.4 kW        │
 *   │ SOC 82%   ETA 42m   +18kWh  │
 *   └──────────────────────────────┘
 */
import React from "react";
import { StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { colors, font, radius, spacing } from "../design/tokens";
import { resolveChargeState } from "../state/resolveChargeState";

interface ChargeStatusCardProps {
  state: BoardState;
}

const STATE_COLOR: Record<string, string> = {
  charging:      colors.powerPositive,
  fully_charged: colors.apActive,
  standby:       colors.alarmWarning,
  disconnected:  colors.dashMuted,
  unavailable:   colors.dashMuted,
};

const STATE_ICON: Record<string, string> = {
  charging:      "⚡",
  fully_charged: "✓",
  standby:       "○",
  disconnected:  "–",
  unavailable:   "–",
};

export function ChargeStatusCard({ state }: ChargeStatusCardProps) {
  const cs = resolveChargeState(state);
  const accentColor = STATE_COLOR[cs.state] ?? colors.dashMuted;
  const icon = STATE_ICON[cs.state] ?? "–";

  return (
    <View style={[styles.card, { borderLeftColor: accentColor }]}>
      {/* Header row */}
      <View style={styles.headerRow}>
        <Text style={[styles.icon, { color: accentColor }]}>{icon}</Text>
        <Text style={[styles.label, { color: accentColor }]}>{cs.label}</Text>
        {cs.chargeKw > 0 ? (
          <Text style={styles.kw}>{cs.chargeKw.toFixed(1)} kW</Text>
        ) : null}
      </View>

      {/* Detail row — only shown when data is meaningful */}
      {cs.state !== "unavailable" ? (
        <View style={styles.detailRow}>
          {cs.soc > 0 ? (
            <View style={styles.stat}>
              <Text style={styles.statLabel}>SOC</Text>
              <Text style={styles.statValue}>{cs.soc}%</Text>
            </View>
          ) : null}

          {cs.minutesToFull > 0 ? (
            <View style={styles.stat}>
              <Text style={styles.statLabel}>ETA</Text>
              <Text style={styles.statValue}>
                {cs.minutesToFull >= 60
                  ? `${Math.floor(cs.minutesToFull / 60)}h ${cs.minutesToFull % 60}m`
                  : `${cs.minutesToFull}m`}
              </Text>
            </View>
          ) : null}

          {cs.energyToCharge > 0.1 ? (
            <View style={styles.stat}>
              <Text style={styles.statLabel}>TO ADD</Text>
              <Text style={styles.statValue}>{cs.energyToCharge.toFixed(1)} kWh</Text>
            </View>
          ) : null}
        </View>
      ) : (
        <Text style={styles.unavailableText}>BMS data unavailable</Text>
      )}
    </View>
  );
}

const styles = StyleSheet.create({
  card: {
    backgroundColor: colors.dashCard,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderLeftWidth: 3,
    paddingHorizontal: spacing.md,
    paddingVertical: spacing.sm,
    gap: spacing.xs,
  },
  headerRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: spacing.xs,
  },
  icon: {
    fontSize: font.size.md,
  },
  label: {
    fontSize: font.size.sm,
    fontWeight: font.weight.bold,
    letterSpacing: 0.8,
  },
  kw: {
    fontSize: font.size.sm,
    color: colors.dashSecondary,
    marginLeft: spacing.xs,
  },
  detailRow: {
    flexDirection: "row",
    gap: spacing.lg,
  },
  stat: {
    gap: 2,
  },
  statLabel: {
    fontSize: font.size.xs,
    color: colors.dashLabel,
    letterSpacing: 0.5,
  },
  statValue: {
    fontSize: font.size.sm2,
    color: colors.dashValue,
    fontWeight: font.weight.semibold,
  },
  unavailableText: {
    fontSize: font.size.sm,
    color: colors.dashMuted,
  },
});
