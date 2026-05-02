import { Pressable, ScrollView, StyleSheet, Text, View } from "react-native";
import type { ReactNode } from "react";
import { colors, font, radius, spacing } from "../design/tokens";

// ── Types ─────────────────────────────────────────────────────────────────────

export interface TabItem<T extends string = string> {
	key: T;
	label: string;
	/** Optional badge count / label */
	badge?: string | number;
}

export interface TabsProps<T extends string = string> {
	items: TabItem<T>[];
	activeKey: T;
	onSelect: (key: T) => void;
	variant?: "default" | "dark" | "underline";
	/** Allow horizontal scrolling when tabs overflow (default: false) */
	scrollable?: boolean;
}

// ── Component ─────────────────────────────────────────────────────────────────

export function Tabs<T extends string = string>({
	items,
	activeKey,
	onSelect,
	variant = "default",
	scrollable = false,
}: TabsProps<T>): ReactNode {
	const isDark = variant === "dark" || variant === "underline";
	const isUnderline = variant === "underline";

	const tabBar = (
		<View
			style={[
				styles.bar,
				isDark ? styles.barDark : styles.barLight,
				isUnderline ? styles.barUnderline : undefined,
			]}
		>
			{items.map((item) => {
				const active = item.key === activeKey;
				return (
					<Pressable
						key={item.key}
						onPress={() => onSelect(item.key)}
						style={[
							styles.tab,
							isUnderline ? styles.tabUnderline : styles.tabSegmented,
							active && !isUnderline
								? isDark
									? styles.tabActiveDark
									: styles.tabActiveLight
								: undefined,
							active && isUnderline ? styles.tabActiveUnderline : undefined,
						]}
					>
						<Text
							style={[
								styles.label,
								active
									? isDark
										? styles.labelActiveDark
										: styles.labelActiveLight
									: isDark
										? styles.labelInactiveDark
										: styles.labelInactiveLight,
							]}
						>
							{item.label}
						</Text>
						{item.badge !== undefined ? (
							<View style={styles.badge}>
								<Text style={styles.badgeText}>{item.badge}</Text>
							</View>
						) : null}
					</Pressable>
				);
			})}
		</View>
	);

	if (scrollable) {
		return (
			<ScrollView horizontal showsHorizontalScrollIndicator={false}>
				{tabBar}
			</ScrollView>
		);
	}
	return tabBar;
}

// ── Styles ────────────────────────────────────────────────────────────────────

const styles = StyleSheet.create({
	bar: {
		flexDirection: "row",
		borderRadius: radius.md,
		overflow: "hidden",
	},
	barLight: {
		backgroundColor: colors.backgroundSubtle,
		borderWidth: 1,
		borderColor: colors.border,
		gap: 0,
	},
	barDark: {
		backgroundColor: colors.dashCard,
		borderWidth: 1,
		borderColor: colors.dashCardBorder,
		gap: 0,
	},
	barUnderline: {
		borderRadius: 0,
		borderWidth: 0,
		borderBottomWidth: 1,
		borderBottomColor: colors.border,
	},
	tab: {
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "center",
		gap: spacing.xs,
	},
	tabSegmented: {
		flex: 1,
		paddingVertical: spacing.sm,
		paddingHorizontal: spacing.md,
	},
	tabUnderline: {
		paddingVertical: spacing.sm2,
		paddingHorizontal: spacing.md2,
		borderBottomWidth: 2,
		borderBottomColor: "transparent",
		marginBottom: -1,
	},
	tabActiveLight: {
		backgroundColor: colors.background,
		borderRadius: radius.sm,
		margin: 2,
		elevation: 1,
	},
	tabActiveDark: {
		backgroundColor: colors.dashCardBorder,
		borderRadius: radius.sm,
		margin: 2,
	},
	tabActiveUnderline: {
		borderBottomColor: colors.primary,
	},
	label: {
		fontSize: font.size.sm,
		fontWeight: font.weight.semibold,
	},
	labelActiveLight: {
		color: colors.foreground,
	},
	labelInactiveLight: {
		color: colors.foregroundMuted,
	},
	labelActiveDark: {
		color: colors.dashValue,
	},
	labelInactiveDark: {
		color: colors.dashSecondary,
	},
	badge: {
		backgroundColor: colors.primary,
		borderRadius: radius.full,
		minWidth: 16,
		height: 16,
		alignItems: "center",
		justifyContent: "center",
		paddingHorizontal: 4,
	},
	badgeText: {
		color: colors.primaryForeground,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
	},
});
