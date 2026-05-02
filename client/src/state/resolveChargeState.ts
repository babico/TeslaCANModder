/**
 * resolveChargeState — pure state machine for charge status display.
 *
 * Charge state derivation rules (all derived from BoardState signals):
 *  - unavailable:  hasBms false OR chassisOnline false
 *  - fully_charged: bmsFullyCharged true AND bmsPower >= 0
 *  - charging:     bmsPower < -0.5 kW (active charge draw)
 *  - standby:      bmsContactorState > 0 (contactor engaged, not actively charging)
 *  - disconnected: default (no contactor, no charging draw)
 */

import type { BoardState } from "@teslacanmodder/protocol";

export type ChargeState = "charging" | "fully_charged" | "standby" | "disconnected" | "unavailable";

export interface ChargeStatusResult {
	state: ChargeState;
	/** kW being drawn from charger (positive number, 0 when not charging) */
	chargeKw: number;
	/** Minutes remaining to full charge — 0 means unknown/unavailable */
	minutesToFull: number;
	/** kWh left to add — 0 means unknown */
	energyToCharge: number;
	/** SOC percent, 0 when unavailable */
	soc: number;
	/** Short label for the state, e.g. "CHARGING", "FULL", "STANDBY" */
	label: string;
	/** Multi-word status line, e.g. "Charging · 7.4 kW" */
	statusLine: string;
}

export function resolveChargeState(state: BoardState): ChargeStatusResult {
	const fallback: ChargeStatusResult = {
		state: "unavailable",
		chargeKw: 0,
		minutesToFull: 0,
		energyToCharge: 0,
		soc: 0,
		label: "NO DATA",
		statusLine: "BMS unavailable",
	};

	if (!state.hasBms || !state.chassisOnline) return fallback;

	const soc = Math.round(state.bmsSoc * 100);
	// bmsPower is negative when drawing from charger
	const chargeKw = state.bmsPower < 0 ? Math.abs(state.bmsPower) : 0;
	const minutesToFull =
		Number.isFinite(state.bmsChargeTimeToFull) && state.bmsChargeTimeToFull > 0
			? Math.round(state.bmsChargeTimeToFull)
			: 0;
	const energyToCharge = Number.isFinite(state.bmsEnergyToCharge) ? state.bmsEnergyToCharge : 0;

	if (state.bmsFullyCharged && chargeKw < 0.5) {
		return {
			state: "fully_charged",
			chargeKw: 0,
			minutesToFull: 0,
			energyToCharge: 0,
			soc,
			label: "FULL",
			statusLine: "Fully charged",
		};
	}

	if (chargeKw >= 0.5) {
		const ttfLabel =
			minutesToFull > 0
				? minutesToFull >= 60
					? `${Math.floor(minutesToFull / 60)}h ${minutesToFull % 60}m`
					: `${minutesToFull}m`
				: null;
		return {
			state: "charging",
			chargeKw,
			minutesToFull,
			energyToCharge,
			soc,
			label: "CHARGING",
			statusLine: ttfLabel
				? `Charging · ${chargeKw.toFixed(1)} kW · ${ttfLabel}`
				: `Charging · ${chargeKw.toFixed(1)} kW`,
		};
	}

	if (state.bmsContactorState > 0) {
		return {
			state: "standby",
			chargeKw: 0,
			minutesToFull,
			energyToCharge,
			soc,
			label: "STANDBY",
			statusLine: "Connected · not charging",
		};
	}

	return {
		state: "disconnected",
		chargeKw: 0,
		minutesToFull: 0,
		energyToCharge: 0,
		soc,
		label: "UNPLUGGED",
		statusLine: "Not connected",
	};
}
