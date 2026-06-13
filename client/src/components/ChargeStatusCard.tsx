import React from "react";
import { StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { selectDashColors, font, spacing } from "../design/tokens";
import { useThemeState } from "../state/ThemeContext";
import { resolveChargeState } from "../state/resolveChargeState";

interface ChargeStatusCardProps {
	state: BoardState;
}

const STATE_ICON: Record<string, string> = {
	charging: "\u26A1",
	fully_charged: "\u2713",
	standby: "\u25CB",
	disconnected: "\u2013",
	unavailable: "\u2013",
};

export function ChargeStatusCard({ state }: ChargeStatusCardProps) {
	const { isDark } = useThemeState();
	const c = selectDashColors(isDark);
	const cs = resolveChargeState(state);

	const stateColor =
		{
			charging: c.powerPositive,
			fully_charged: c.apActive,
			standby: c.alarmWarning,
			disconnected: c.dashMuted,
			unavailable: c.dashMuted,
		}[cs.state] ?? c.dashMuted;

	const icon = STATE_ICON[cs.state] ?? "\u2013";

	return (
		<View
			style={[
				styles.card,
				{
					borderLeftColor: stateColor,
					backgroundColor: c.dashCard,
					borderColor: c.dashCardBorder,
				},
			]}
		>
			<View style={styles.headerRow}>
				<Text style={[styles.icon, { color: stateColor }]}>{icon}</Text>
				<Text style={[styles.label, { color: stateColor }]}>{cs.label}</Text>
				{cs.chargeKw > 0 ? (
					<Text style={[styles.kw, { color: c.dashValue }]}>
						{cs.chargeKw.toFixed(1)} kW
					</Text>
				) : null}
			</View>

			{cs.state !== "unavailable" ? (
				<View style={styles.detailRow}>
					<Text style={[styles.detailText, { color: c.dashSecondary }]}>
						SOC {cs.soc}%
					</Text>
					{cs.minutesToFull > 0 ? (
						<Text style={[styles.detailText, { color: c.dashSecondary }]}>
							ETA {cs.minutesToFull}m
						</Text>
					) : null}
					{cs.energyToCharge > 0 ? (
						<Text style={[styles.detailText, { color: c.powerPositive }]}>
							+{cs.energyToCharge.toFixed(1)} kWh
						</Text>
					) : null}
				</View>
			) : null}
		</View>
	);
}

const styles = StyleSheet.create({
	card: {
		borderLeftWidth: 3,
		borderWidth: 1,
		borderRadius: 8,
		padding: 10,
		gap: 6,
	},
	headerRow: {
		flexDirection: "row",
		alignItems: "center",
		gap: 6,
	},
	icon: {
		fontSize: 16,
		fontWeight: "700",
	},
	label: {
		fontSize: font.size.sm,
		fontWeight: "600",
		flex: 1,
	},
	kw: {
		fontSize: font.size.sm,
		fontWeight: "700",
	},
	detailRow: {
		flexDirection: "row",
		gap: spacing.md,
	},
	detailText: {
		fontSize: font.size.xs,
	},
});
