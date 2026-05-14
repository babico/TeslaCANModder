import { useState } from "react";
import { View, Text, Pressable, Modal, ScrollView } from "react-native";
import { cn } from "../cn";
import { ChevronDown } from "lucide-react-native";

interface SelectOption {
	value: string;
	label: string;
}

interface SelectProps {
	value: string;
	options: SelectOption[];
	onChange: (value: string) => void;
	placeholder?: string;
	className?: string;
}

export function Select({ value, options, onChange, placeholder, className }: SelectProps) {
	const [open, setOpen] = useState(false);
	const selected = options.find((o) => o.value === value);

	return (
		<View className={className}>
			<Pressable
				onPress={() => setOpen(true)}
				className="flex-row items-center justify-between h-10 rounded-md border border-input bg-background px-3"
			>
				<Text
					className={cn(
						"text-sm",
						selected ? "text-foreground" : "text-muted-foreground",
					)}
				>
					{selected?.label ?? placeholder ?? "Select..."}
				</Text>
				<ChevronDown size={16} className="text-muted-foreground" />
			</Pressable>
			<Modal
				visible={open}
				transparent
				animationType="fade"
				onRequestClose={() => setOpen(false)}
			>
				<Pressable className="flex-1 bg-black/50" onPress={() => setOpen(false)} />
				<View className="bg-card rounded-t-xl max-h-[60%]">
					<ScrollView>
						{options.map((opt) => (
							<Pressable
								key={opt.value}
								onPress={() => {
									onChange(opt.value);
									setOpen(false);
								}}
								className={cn(
									"px-4 py-3 border-b border-border/50",
									opt.value === value ? "bg-primary/10" : "",
								)}
							>
								<Text
									className={cn(
										"text-sm",
										opt.value === value
											? "text-primary font-medium"
											: "text-foreground",
									)}
								>
									{opt.label}
								</Text>
							</Pressable>
						))}
					</ScrollView>
				</View>
			</Modal>
		</View>
	);
}
