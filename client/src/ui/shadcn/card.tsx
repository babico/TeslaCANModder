import { View, Text, type ViewProps, type TextProps } from "react-native";
import { cn } from "../cn";

export function Card({ className, ...props }: ViewProps) {
	return (
		<View className={cn("rounded-lg border border-border bg-card p-4", className)} {...props} />
	);
}

export function CardHeader({ className, ...props }: ViewProps) {
	return <View className={cn("gap-1.5 pb-3", className)} {...props} />;
}

export function CardTitle({ className, ...props }: TextProps) {
	return (
		<Text className={cn("text-lg font-semibold text-card-foreground", className)} {...props} />
	);
}

export function CardDescription({ className, ...props }: TextProps) {
	return <Text className={cn("text-sm text-muted-foreground", className)} {...props} />;
}

export function CardContent({ className, ...props }: ViewProps) {
	return <View className={cn("", className)} {...props} />;
}

export function CardFooter({ className, ...props }: ViewProps) {
	return <View className={cn("flex-row items-center pt-3", className)} {...props} />;
}
