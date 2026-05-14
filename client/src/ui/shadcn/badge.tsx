import { cva, type VariantProps } from "class-variance-authority";
import { Text, View, type ViewProps } from "react-native";
import { cn } from "../cn";

const badgeVariants = cva("inline-flex items-center rounded-full px-2.5 py-0.5", {
	variants: {
		variant: {
			default: "bg-primary",
			secondary: "bg-secondary",
			destructive: "bg-destructive",
			outline: "border border-border bg-transparent",
		},
	},
	defaultVariants: { variant: "default" },
});

const badgeTextVariants = cva("text-xs font-semibold", {
	variants: {
		variant: {
			default: "text-primary-foreground",
			secondary: "text-secondary-foreground",
			destructive: "text-destructive-foreground",
			outline: "text-foreground",
		},
	},
	defaultVariants: { variant: "default" },
});

interface BadgeProps extends ViewProps, VariantProps<typeof badgeVariants> {
	label: string;
}

export function Badge({ variant, label, className, ...props }: BadgeProps) {
	return (
		<View className={cn(badgeVariants({ variant }), className)} {...props}>
			<Text className={cn(badgeTextVariants({ variant }))}>{label}</Text>
		</View>
	);
}
