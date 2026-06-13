import {
	createContext,
	useCallback,
	useContext,
	useEffect,
	useMemo,
	useState,
	type ReactNode,
} from "react";
import { Appearance, Platform } from "react-native";

export type ThemeOverride = "dark" | "light" | null;
type ThemeSource = "force" | "override" | "system" | "default";

export interface ThemeState {
	isDark: boolean;
	source: ThemeSource;
	override: ThemeOverride;
	toggle: () => void;
	setOverride: (v: ThemeOverride) => void;
}

const ThemeContext = createContext<ThemeState>({
	isDark: false,
	source: "default",
	override: null,
	toggle: () => {},
	setOverride: () => {},
});

export function useThemeState(): ThemeState {
	return useContext(ThemeContext);
}

function applyDarkClass(dark: boolean) {
	if (Platform.OS === "web" && typeof document !== "undefined") {
		document.documentElement.classList.toggle("dark", dark);
	}
}

export function ThemeProvider({
	children,
	forceNight,
}: {
	children: ReactNode;
	forceNight?: boolean;
}) {
	const [override, _setOverride] = useState<ThemeOverride>(null);
	const [systemDark, setSystemDark] = useState(
		Platform.OS === "web" && typeof window !== "undefined"
			? window.matchMedia("(prefers-color-scheme: dark)").matches
			: Appearance.getColorScheme() === "dark",
	);

	useEffect(() => {
		if (Platform.OS === "web" && typeof window !== "undefined") {
			const mq = window.matchMedia("(prefers-color-scheme: dark)");
			const handler = (e: MediaQueryListEvent) => setSystemDark(e.matches);
			mq.addEventListener("change", handler);
			return () => mq.removeEventListener("change", handler);
		}
		const sub = Appearance.addChangeListener(({ colorScheme }) => {
			setSystemDark(colorScheme === "dark");
		});
		return () => sub.remove();
	}, []);

	const setOverride = useCallback((v: ThemeOverride) => {
		_setOverride(v);
	}, []);

	const isDark = forceNight ? true : override !== null ? override === "dark" : systemDark;

	const source: ThemeSource = forceNight
		? "force"
		: override !== null
			? "override"
			: systemDark
				? "system"
				: "default";

	const toggle = useCallback(() => {
		_setOverride((prev) => {
			const next: ThemeOverride =
				prev === "dark" ? "light" : prev === "light" ? null : "dark";
			return next;
		});
	}, []);

	useEffect(() => {
		applyDarkClass(isDark);
	}, [isDark]);

	const value = useMemo<ThemeState>(
		() => ({ isDark, source, override, toggle, setOverride }),
		[isDark, source, override, toggle, setOverride],
	);

	return <ThemeContext.Provider value={value}>{children}</ThemeContext.Provider>;
}
