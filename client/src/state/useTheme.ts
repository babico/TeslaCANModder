/**
 * useTheme — day/night theme with auto-detect and manual override.
 *
 * Priority:
 *  1. `forceNight` flag from BoardState (e.g. vehicle sends darkened UI signal)
 *  2. Manual user override (persisted in module-level cache, lost on restart)
 *  3. System `Appearance` (colorScheme 'dark' → dark)
 *  4. Default: light
 *
 * Returns:
 *  - `isDark`: boolean — whether to render the dark/night dashboard
 *  - `source`: 'force' | 'override' | 'system' | 'default'
 *  - `override`: null | 'dark' | 'light' — current manual override
 *  - `setOverride(v)`: set manual override (null = clear, follow system)
 */
import { useEffect, useState } from "react";
import { Appearance } from "react-native";

export type ThemeSource = "force" | "override" | "system" | "default";
export type ThemeOverride = "dark" | "light" | null;

export interface ThemeState {
	isDark: boolean;
	source: ThemeSource;
	override: ThemeOverride;
	setOverride: (v: ThemeOverride) => void;
}

// Module-level cache so override survives component remounts
let cachedOverride: ThemeOverride = null;

export function useTheme(forceNight?: boolean): ThemeState {
	const [override, _setOverride] = useState<ThemeOverride>(cachedOverride);
	const [systemDark, setSystemDark] = useState(Appearance.getColorScheme() === "dark");

	useEffect(() => {
		const sub = Appearance.addChangeListener(({ colorScheme }) => {
			setSystemDark(colorScheme === "dark");
		});
		return () => sub.remove();
	}, []);

	const setOverride = (v: ThemeOverride) => {
		cachedOverride = v;
		_setOverride(v);
	};

	let isDark: boolean;
	let source: ThemeSource;

	if (forceNight) {
		isDark = true;
		source = "force";
	} else if (override !== null) {
		isDark = override === "dark";
		source = "override";
	} else if (systemDark) {
		isDark = true;
		source = "system";
	} else {
		isDark = false;
		source = "default";
	}

	return { isDark, source, override, setOverride };
}
