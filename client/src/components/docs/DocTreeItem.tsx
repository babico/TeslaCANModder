import { Pressable, StyleSheet, Text, View } from "react-native";

import { colors, font, radius, spacing } from "../../design/tokens";

type DocTreeItemProps = {
	title: string;
	depth: number;
	hasNextSiblings: boolean[];
	isFolder: boolean;
	expanded?: boolean;
	active?: boolean;
	onPress: () => void;
};

export function DocTreeItem({
	title,
	depth,
	hasNextSiblings,
	isFolder,
	expanded,
	active,
	onPress,
}: DocTreeItemProps) {
	return (
		<Pressable onPress={onPress} style={[styles.row, active ? styles.rowActive : undefined]}>
			<View style={styles.guides}>
				{Array.from({ length: depth }).map((_, index) => (
					<View key={`${title}-guide-${index}`} style={styles.guideSlot}>
						<View
							style={[
								styles.guideLine,
								hasNextSiblings[index] ? styles.guideLineVisible : undefined,
							]}
						/>
					</View>
				))}
				<View style={styles.connectorSlot}>
					<View style={styles.connectorHorizontal} />
				</View>
			</View>
			<Text style={[styles.icon, active ? styles.textActive : undefined]}>
				{isFolder ? (expanded ? "▾" : "▸") : "•"}
			</Text>
			<Text
				numberOfLines={1}
				style={[
					styles.title,
					isFolder ? styles.folderTitle : undefined,
					active ? styles.textActive : undefined,
				]}
			>
				{title}
			</Text>
		</Pressable>
	);
}

const styles = StyleSheet.create({
	row: {
		minHeight: 30,
		flexDirection: "row",
		alignItems: "center",
		borderRadius: radius.sm,
		paddingHorizontal: spacing.xs,
	},
	rowActive: {
		backgroundColor: colors.backgroundDarkSubtle,
	},
	guides: {
		flexDirection: "row",
		alignItems: "center",
		marginRight: spacing.xs,
	},
	guideSlot: {
		width: 14,
		alignItems: "center",
	},
	guideLine: {
		width: 1,
		height: 24,
		backgroundColor: "transparent",
	},
	guideLineVisible: {
		backgroundColor: colors.dashCardBorder,
	},
	connectorSlot: {
		width: 14,
		alignItems: "center",
	},
	connectorHorizontal: {
		width: 10,
		height: 1,
		backgroundColor: colors.dashCardBorder,
	},
	icon: {
		width: 16,
		color: colors.dashMuted,
		fontSize: font.size.md,
		marginRight: spacing.sm2,
	},
	title: {
		flex: 1,
		color: colors.dashValue,
		fontSize: font.size.md,
		lineHeight: 20,
	},
	folderTitle: {
		color: colors.dashLabel,
		fontWeight: font.weight.semibold,
	},
	textActive: {
		color: colors.dashPrimary,
	},
});
