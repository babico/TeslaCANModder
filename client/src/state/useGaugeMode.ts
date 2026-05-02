/**
 * useGaugeMode — D-10
 *
 * Cycles between three display layouts for the drive cluster's center
 * and side panels:
 *  - "simple"  — speed + gear + pedal (default)
 *  - "power"   — speed + gear + large power kW + power bar
 *  - "perf"    — speed + gear + steering angle + dual pedal bars
 *
 * Selection is persisted module-level so it survives re-renders and
 * tab switches within a session.
 */

import { useState, useCallback } from "react";

export type GaugeMode = "simple" | "power" | "perf";

const MODES: GaugeMode[] = ["simple", "power", "perf"];
const MODE_LABELS: Record<GaugeMode, string> = {
	simple: "Simple",
	power: "Power",
	perf: "Perf",
};

let cachedMode: GaugeMode = "simple";

export interface GaugeModeState {
	mode: GaugeMode;
	modeLabel: string;
	cycleMode: () => void;
}

export function useGaugeMode(): GaugeModeState {
	const [mode, setMode] = useState<GaugeMode>(cachedMode);

	const cycleMode = useCallback(() => {
		setMode((prev) => {
			const next = MODES[(MODES.indexOf(prev) + 1) % MODES.length];
			cachedMode = next;
			return next;
		});
	}, []);

	return {
		mode,
		modeLabel: MODE_LABELS[mode],
		cycleMode,
	};
}
