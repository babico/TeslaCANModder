import { Pressable, Text, View } from "react-native";

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
		<Pressable
			onPress={onPress}
			className={`min-h-[30px] flex-row items-center rounded-md px-1 ${active ? "bg-muted" : ""}`}
		>
			<View className="flex-row items-center mr-1">
				{Array.from({ length: depth }).map((_, index) => (
					<View key={`${title}-guide-${index}`} className="w-[14px] items-center">
						<View
							className={`w-px h-6 ${hasNextSiblings[index] ? "bg-border" : "bg-transparent"}`}
						/>
					</View>
				))}
				<View className="w-[14px] items-center">
					<View className="w-2.5 h-px bg-border" />
				</View>
			</View>
			<Text
				className={`w-4 text-base mr-1.5 ${active ? "text-primary" : "text-muted-foreground"}`}
			>
				{isFolder ? (expanded ? "\u25BE" : "\u25B8") : "\u2022"}
			</Text>
			<Text
				numberOfLines={1}
				className={`flex-1 text-sm leading-5 ${active ? "text-primary" : isFolder ? "text-card-foreground font-semibold" : "text-card-foreground"}`}
			>
				{title}
			</Text>
		</Pressable>
	);
}
