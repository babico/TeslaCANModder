import { TextInput, type TextInputProps } from "react-native";
import { cn } from "../cn";

export function Input({ className, ...props }: TextInputProps) {
	return (
		<TextInput
			className={cn(
				"h-10 rounded-md border border-input bg-background px-3 py-2 text-sm text-foreground placeholder:text-muted-foreground",
				className,
			)}
			{...props}
		/>
	);
}
