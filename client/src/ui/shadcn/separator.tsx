import { View, type ViewProps } from "react-native";
import { cn } from "../cn";

export function Separator({ className, ...props }: ViewProps) {
	return <View className={cn("h-px bg-border", className)} {...props} />;
}
