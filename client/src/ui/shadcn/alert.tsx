import { View, Text, type ViewProps } from "react-native";
import { cn } from "../cn";
import { TriangleAlert } from "lucide-react-native";

interface AlertProps extends ViewProps {
	variant?: "default" | "destructive";
	title?: string;
	description?: string;
}

export function Alert({
	variant = "default",
	title,
	description,
	className,
	...props
}: AlertProps) {
	return (
		<View
			className={cn(
				"rounded-lg border p-4",
				variant === "destructive"
					? "border-destructive/50 bg-destructive/10"
					: "border-border bg-muted/50",
				className,
			)}
			{...props}
		>
			<View className="flex-row items-center gap-2">
				{variant === "destructive" && (
					<TriangleAlert size={16} className="text-destructive" />
				)}
				{title ? (
					<Text
						className={cn(
							"text-sm font-semibold",
							variant === "destructive" ? "text-destructive" : "text-foreground",
						)}
					>
						{title}
					</Text>
				) : null}
			</View>
			{description ? (
				<Text
					className={cn(
						"text-sm mt-1",
						variant === "destructive" ? "text-destructive/80" : "text-muted-foreground",
					)}
				>
					{description}
				</Text>
			) : null}
		</View>
	);
}
