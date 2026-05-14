import { Pressable, View, Text, Modal, type ModalProps } from "react-native";
import { Button } from "./button";

interface SheetProps extends ModalProps {
	open: boolean;
	onClose: () => void;
	title?: string;
}

export function Sheet({ open, onClose, title, children, ...props }: SheetProps) {
	return (
		<Modal visible={open} transparent animationType="slide" onRequestClose={onClose} {...props}>
			<Pressable className="flex-1 bg-black/50" onPress={onClose} />
			<View className="rounded-t-xl border-t border-border bg-card p-4 gap-4 max-h-[80%]">
				<View className="flex-row items-center justify-between">
					{title ? (
						<Text className="text-lg font-semibold text-card-foreground">{title}</Text>
					) : (
						<View />
					)}
					<Button label="Done" variant="ghost" size="sm" onPress={onClose} />
				</View>
				{children}
			</View>
		</Modal>
	);
}
