import { createContext, useContext, type ReactNode } from "react";
import { View, Text, Pressable } from "react-native";
import { cn } from "../cn";

const TabsCtx = createContext<{ value: string; onChange: (v: string) => void } | null>(null);

export function Tabs({
	value,
	onChange,
	children,
}: {
	value: string;
	onChange: (v: string) => void;
	children: ReactNode;
}) {
	return <TabsCtx.Provider value={{ value, onChange }}>{children}</TabsCtx.Provider>;
}

export function TabsList({ className, children }: { className?: string; children: ReactNode }) {
	return <View className={cn("flex-row border-b border-border", className)}>{children}</View>;
}

export function TabsTrigger({
	value,
	label,
	className,
}: {
	value: string;
	label: string;
	className?: string;
}) {
	const ctx = useContext(TabsCtx);
	if (!ctx) return null;
	const active = ctx.value === value;
	return (
		<Pressable
			onPress={() => ctx.onChange(value)}
			className={cn(
				"px-4 py-2 border-b-2",
				active ? "border-primary" : "border-transparent",
				className,
			)}
		>
			<Text
				className={cn(
					"text-sm",
					active ? "text-primary font-medium" : "text-muted-foreground",
				)}
			>
				{label}
			</Text>
		</Pressable>
	);
}

export function TabsContent({ value, children }: { value: string; children: ReactNode }) {
	const ctx = useContext(TabsCtx);
	if (!ctx || ctx.value !== value) return null;
	return <>{children}</>;
}
