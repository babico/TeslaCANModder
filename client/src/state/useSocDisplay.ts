/**
 * useSocDisplay — D-04 SOC/range toggle hook.
 *
 * Tap-to-cycle display mode: soc% → range (expected km) → range (ideal km) → soc%.
 * Falls back gracefully when range data is unavailable.
 * Persists selection in a module-level cache for session continuity.
 */

import { useCallback, useState } from "react";
import type { BoardState } from "@teslacanmodder/protocol";

export type SocDisplayMode = "soc" | "range_expected" | "range_ideal";

// Module-level cache — survives component remounts within a session
let cachedMode: SocDisplayMode = "soc";

export interface SocDisplayResult {
	mode: SocDisplayMode;
	/** Cycle to the next available mode */
	cycleMode: () => void;
	/** Formatted primary value string (e.g. "82.3%" or "312 km") */
	primaryValue: string;
	/** Short label for the current mode (e.g. "SOC", "RANGE", "IDEAL") */
	modeLabel: string;
	/** True when the current mode's data is unavailable */
	isUnavailable: boolean;
	/** Alarm level derived from SOC regardless of mode */
	socAlarm: "critical" | "warning" | "ok";
}

function formatRange(km: number): string {
	if (!Number.isFinite(km) || km <= 0) return "—";
	return `${Math.round(km)} km`;
}

function formatSoc(soc: number): string {
	if (!Number.isFinite(soc)) return "—";
	return `${soc.toFixed(1)}%`;
}

function socAlarm(soc: number): "critical" | "warning" | "ok" {
	if (soc < 15) return "critical";
	if (soc < 25) return "warning";
	return "ok";
}

export function useSocDisplay(state: BoardState): SocDisplayResult {
	const [mode, setMode] = useState<SocDisplayMode>(cachedMode);

	const hasBms = state.hasBms;
	const hasRange =
		hasBms && Number.isFinite(state.bmsExpectedRange) && state.bmsExpectedRange > 0;

	const cycleMode = useCallback(() => {
		setMode((prev) => {
			let next: SocDisplayMode;
			if (prev === "soc") {
				next = hasRange ? "range_expected" : "soc";
			} else if (prev === "range_expected") {
				next = hasRange ? "range_ideal" : "soc";
			} else {
				next = "soc";
			}
			cachedMode = next;
			return next;
		});
	}, [hasRange]);

	const alarm = socAlarm(state.bmsSoc);
	let primaryValue: string;
	let modeLabel: string;
	let isUnavailable = false;

	switch (mode) {
		case "soc":
			primaryValue = formatSoc(state.bmsSoc);
			modeLabel = "SOC";
			isUnavailable = !hasBms;
			break;
		case "range_expected":
			primaryValue = hasRange ? formatRange(state.bmsExpectedRange) : "—";
			modeLabel = "RANGE";
			isUnavailable = !hasRange;
			break;
		case "range_ideal":
			primaryValue = hasRange ? formatRange(state.bmsIdealRange) : "—";
			modeLabel = "IDEAL";
			isUnavailable = !hasRange || !Number.isFinite(state.bmsIdealRange);
			break;
	}

	return { mode, cycleMode, primaryValue, modeLabel, isUnavailable, socAlarm: alarm };
}
