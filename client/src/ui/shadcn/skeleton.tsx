import { View, type ViewProps } from "react-native";
import { cn } from "../cn";

export function Skeleton({ className, ...props }: ViewProps) {
	return <View className={cn("h-4 rounded bg-muted animate-pulse", className)} {...props} />;
}
