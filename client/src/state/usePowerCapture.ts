/**
 * usePowerCapture — D-08 power min/max capture hook.
 *
 * Tracks session-lifetime min and max power (kW) with a user-resettable
 * capture. Integrates with the PowerBarV2 component.
 */

import { useCallback, useRef, useState } from "react";

export interface PowerCaptureState {
	/** Live power value (kW, positive = drive, negative = regen) */
	power: number;
	/** Session capture: maximum drive power seen (kW, ≥ 0) */
	maxDrive: number;
	/** Session capture: maximum regen power seen (abs kW, ≥ 0) */
	maxRegen: number;
	/** Reset both captured extremes */
	reset: () => void;
}

export function usePowerCapture(livePower: number): PowerCaptureState {
	const [maxDrive, setMaxDrive] = useState(0);
	const [maxRegen, setMaxRegen] = useState(0);
	const maxDriveRef = useRef(0);
	const maxRegenRef = useRef(0);

	// Update peaks inline (avoid stale closure in render)
	if (Number.isFinite(livePower)) {
		if (livePower > maxDriveRef.current) {
			maxDriveRef.current = livePower;
			// Batch update — will re-render
			if (livePower > maxDrive) setMaxDrive(livePower);
		} else if (livePower < 0 && Math.abs(livePower) > maxRegenRef.current) {
			maxRegenRef.current = Math.abs(livePower);
			if (Math.abs(livePower) > maxRegen) setMaxRegen(Math.abs(livePower));
		}
	}

	const reset = useCallback(() => {
		maxDriveRef.current = 0;
		maxRegenRef.current = 0;
		setMaxDrive(0);
		setMaxRegen(0);
	}, []);

	return { power: livePower, maxDrive, maxRegen, reset };
}
