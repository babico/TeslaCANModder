import { StyleSheet, Text, View } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

export type BadgeVariant = "default" | "success" | "warning" | "destructive" | "muted" | "dark";

export interface BadgeProps {
	children: string;
	variant?: BadgeVariant;
}

export function Badge({ children, variant = "default" }: BadgeProps) {
	return (
		<View style={[styles.base, styles[`variant_${variant}`]]}>
			<Text style={[styles.label, styles[`label_${variant}`]]}>{children}</Text>
		</View>
	);
}

const styles = StyleSheet.create({
	base: {
		borderRadius: radius.full,
		paddingHorizontal: spacing.sm,
		paddingVertical: spacing.xs2,
		borderWidth: 1,
		alignSelf: "flex-start",
	},
	variant_default: {
		backgroundColor: colors.backgroundSubtle,
		borderColor: colors.border,
	},
	variant_success: {
		backgroundColor: colors.successSubtle,
		borderColor: colors.successBorder,
	},
	variant_warning: {
		backgroundColor: colors.warningSubtle,
		borderColor: colors.warningBorder,
	},
	variant_destructive: {
		backgroundColor: colors.destructiveSubtle,
		borderColor: colors.destructiveBorder,
	},
	variant_muted: {
		backgroundColor: colors.muted,
		borderColor: colors.border,
	},
	variant_dark: {
		backgroundColor: colors.dashCard,
		borderColor: colors.dashCardBorder,
	},
	label: {
		fontSize: font.size.xs,
		fontWeight: font.weight.semibold,
	},
	label_default: {
		color: colors.foreground,
	},
	label_success: {
		color: colors.success,
	},
	label_warning: {
		color: colors.warningForeground,
	},
	label_destructive: {
		color: colors.destructive,
	},
	label_muted: {
		color: colors.mutedForeground,
	},
	label_dark: {
		color: colors.dashSecondary,
	},
});
