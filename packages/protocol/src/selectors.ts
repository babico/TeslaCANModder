import type { BoardState } from "./types.js";

export interface ConnectionSummary {
	onlineBuses: number;
	totalBuses: number;
	anyOnline: boolean;
	allOnline: boolean;
	status: "offline" | "partial" | "online";
}

export interface DriveSnapshot {
	speedKmh: number;
	gear: number;
	accelPedalPct: number;
	steeringDeg: number;
	rearMotorRpm: number;
	frontMotorRpm: number;
}

export interface ChargeSnapshot {
	state: "unavailable" | "charging" | "fully_charged" | "standby" | "disconnected";
	chargeKw: number;
	socPercent: number;
	minutesToFull: number;
	energyToChargeKwh: number;
}

export type ApClusterState = "unavailable" | "inactive" | "active" | "hands_warning";

export type IndicatorVariant = "success" | "warning" | "destructive" | "muted";

export interface AutopilotIndicatorState {
	apState: ApClusterState;
	label: string;
	variant: IndicatorVariant;
	showHandsWarning: boolean;
	showNag: boolean;
}

export function selectConnectionSummary(state: BoardState): ConnectionSummary {
	const buses = [state.chassisOnline, state.vehicleOnline, state.bodyOnline];
	const onlineBuses = buses.filter(Boolean).length;
	const anyOnline = onlineBuses > 0;
	const allOnline = onlineBuses === buses.length;

	let status: ConnectionSummary["status"];
	if (!anyOnline) {
		status = "offline";
	} else if (allOnline) {
		status = "online";
	} else {
		status = "partial";
	}

	return {
		onlineBuses,
		totalBuses: buses.length,
		anyOnline,
		allOnline,
		status,
	};
}

export function selectDriveSnapshot(state: BoardState): DriveSnapshot | null {
	if (!state.hasPowertrain) {
		return null;
	}

	return {
		speedKmh: state.vehicleSpeed,
		gear: state.gearState,
		accelPedalPct: state.accelPedal,
		steeringDeg: state.steeringAngle,
		rearMotorRpm: state.rearMotorRpm,
		frontMotorRpm: state.frontMotorRpm,
	};
}

export function selectChargeSnapshot(state: BoardState): ChargeSnapshot {
	if (!state.hasBms || !state.chassisOnline) {
		return {
			state: "unavailable",
			chargeKw: 0,
			socPercent: 0,
			minutesToFull: 0,
			energyToChargeKwh: 0,
		};
	}

	const chargeKw = state.bmsPower < 0 ? Math.abs(state.bmsPower) : 0;
	const minutesToFull =
		Number.isFinite(state.bmsChargeTimeToFull) && state.bmsChargeTimeToFull > 0
			? Math.round(state.bmsChargeTimeToFull)
			: 0;

	if (state.bmsFullyCharged && chargeKw < 0.5) {
		return {
			state: "fully_charged",
			chargeKw: 0,
			socPercent: Math.round(state.bmsSoc * 100),
			minutesToFull: 0,
			energyToChargeKwh: 0,
		};
	}

	if (chargeKw >= 0.5) {
		return {
			state: "charging",
			chargeKw,
			socPercent: Math.round(state.bmsSoc * 100),
			minutesToFull,
			energyToChargeKwh: state.bmsEnergyToCharge,
		};
	}

	if (state.bmsContactorState > 0) {
		return {
			state: "standby",
			chargeKw: 0,
			socPercent: Math.round(state.bmsSoc * 100),
			minutesToFull,
			energyToChargeKwh: state.bmsEnergyToCharge,
		};
	}

	return {
		state: "disconnected",
		chargeKw: 0,
		socPercent: Math.round(state.bmsSoc * 100),
		minutesToFull: 0,
		energyToChargeKwh: 0,
	};
}

function resolveApState(tier: number, dasHandsOn: number, chassisOnline: boolean): ApClusterState {
	if (!chassisOnline || tier === 0) return "unavailable";
	if (dasHandsOn >= 1) return "hands_warning";
	if (tier >= 3) return "active";
	return "inactive";
}

function formatAutopilotLabel(state: ApClusterState, tier: number): string {
	switch (state) {
		case "unavailable":
			return "AP N/A";
		case "hands_warning":
			return "Hands On";
		case "active":
			return tier >= 3 ? "SELF_DRIVING" : "ACTIVE";
		case "inactive":
			if (tier === 1) return "HIGHWAY";
			if (tier === 2) return "ENHANCED";
			return "INACTIVE";
	}
}

function resolveIndicatorVariant(state: ApClusterState): IndicatorVariant {
	switch (state) {
		case "active":
			return "success";
		case "hands_warning":
			return "warning";
		case "inactive":
			return "muted";
		case "unavailable":
			return "muted";
	}
}

export function selectAutopilotIndicatorState(state: BoardState): AutopilotIndicatorState {
	const apState = resolveApState(state.gtwAutopilotTier, state.dasHandsOn, state.chassisOnline);
	return {
		apState,
		label: formatAutopilotLabel(apState, state.gtwAutopilotTier),
		variant: resolveIndicatorVariant(apState),
		showHandsWarning: apState === "hands_warning",
		showNag: state.nag,
	};
}
