import { cva, type VariantProps } from "class-variance-authority";
import { Text, Pressable, type PressableProps } from "react-native";
import { cn } from "../cn";

const buttonVariants = cva("flex-row items-center justify-center rounded-md px-4 py-2", {
	variants: {
		variant: {
			default: "bg-primary",
			destructive: "bg-destructive",
			outline: "border border-input bg-transparent",
			secondary: "bg-secondary",
			ghost: "bg-transparent",
			link: "bg-transparent",
		},
		size: {
			default: "h-10",
			sm: "h-8 px-3",
			lg: "h-12 px-8",
			icon: "h-10 w-10",
		},
	},
	defaultVariants: { variant: "default", size: "default" },
});

const buttonTextVariants = cva("text-sm font-medium", {
	variants: {
		variant: {
			default: "text-primary-foreground",
			destructive: "text-destructive-foreground",
			outline: "text-foreground",
			secondary: "text-secondary-foreground",
			ghost: "text-foreground",
			link: "text-primary underline",
		},
	},
	defaultVariants: { variant: "default" },
});

interface ButtonProps extends PressableProps, VariantProps<typeof buttonVariants> {
	label: string;
}

export function Button({ variant, size, label, className, disabled, ...props }: ButtonProps) {
	return (
		<Pressable
			className={cn(buttonVariants({ variant, size }), disabled && "opacity-50", className)}
			disabled={disabled}
			{...props}
		>
			<Text className={cn(buttonTextVariants({ variant }))}>{label}</Text>
		</Pressable>
	);
}
