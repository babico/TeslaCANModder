import { Pressable } from "react-native";
import { Sun, Moon, Monitor } from "lucide-react-native";
import { useThemeState } from "../state/ThemeContext";

export function ThemeSwitch() {
	const { isDark, override, toggle } = useThemeState();

	const iconSize = 16;
	const iconColor = isDark ? "#fbbf24" : "#64748b";

	const icon =
		override === "dark" ? (
			<Moon size={iconSize} color={iconColor} fill={iconColor} />
		) : override === "light" ? (
			<Sun size={iconSize} color={iconColor} fill={iconColor} />
		) : (
			<Monitor size={iconSize} color={iconColor} />
		);

	return (
		<Pressable
			onPress={toggle}
			className="w-9 h-9 items-center justify-center rounded-full bg-muted border border-border"
			accessibilityLabel={
				override === "dark"
					? "Switch to light mode"
					: override === "light"
						? "Follow system theme"
						: "Switch to dark mode"
			}
		>
			{icon}
		</Pressable>
	);
}
