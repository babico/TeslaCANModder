import { StyleSheet, Text, View } from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

export type AlertVariant = "default" | "success" | "warning" | "destructive";

export interface AlertProps {
	title?: string;
	description: string;
	variant?: AlertVariant;
}

export function Alert({ title, description, variant = "default" }: AlertProps) {
	return (
		<View style={[styles.base, styles[`variant_${variant}`]]}>
			{title ? <Text style={[styles.title, styles[`title_${variant}`]]}>{title}</Text> : null}
			<Text style={[styles.description, styles[`description_${variant}`]]}>
				{description}
			</Text>
		</View>
	);
}

const styles = StyleSheet.create({
	base: {
		borderRadius: radius.md,
		borderWidth: 1,
		paddingHorizontal: spacing.md,
		paddingVertical: spacing.sm,
		gap: spacing.xs2,
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
	title: {
		fontWeight: font.weight.bold,
		fontSize: font.size.sm,
	},
	title_default: { color: colors.foreground },
	title_success: { color: colors.success },
	title_warning: { color: colors.warningForeground },
	title_destructive: { color: colors.destructive },
	description: {
		fontSize: font.size.sm,
		lineHeight: font.size.sm * font.lineHeight.normal,
	},
	description_default: { color: colors.foregroundMuted },
	description_success: { color: colors.success },
	description_warning: { color: colors.warningForeground },
	description_destructive: { color: colors.destructive },
});
