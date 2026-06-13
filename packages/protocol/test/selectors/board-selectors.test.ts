import { initialBoardState } from "../../src/reducer.js";
import {
	selectChargeSnapshot,
	selectConnectionSummary,
	selectDriveSnapshot,
} from "../../src/selectors.js";

describe("selectors: selectConnectionSummary", () => {
	it("reports offline when all buses are down", () => {
		const summary = selectConnectionSummary({
			...initialBoardState,
			chassisOnline: false,
			vehicleOnline: false,
			bodyOnline: false,
		});

		expect(summary.onlineBuses).toBe(0);
		expect(summary.status).toBe("offline");
		expect(summary.anyOnline).toBe(false);
	});

	it("reports partial when some buses are online", () => {
		const summary = selectConnectionSummary({
			...initialBoardState,
			chassisOnline: true,
			vehicleOnline: true,
			bodyOnline: false,
		});

		expect(summary.onlineBuses).toBe(2);
		expect(summary.status).toBe("partial");
		expect(summary.allOnline).toBe(false);
	});

	it("reports online when all buses are online and tx is active", () => {
		const summary = selectConnectionSummary({
			...initialBoardState,
			chassisOnline: true,
			vehicleOnline: true,
			bodyOnline: true,
		});

		expect(summary.onlineBuses).toBe(3);
		expect(summary.status).toBe("online");
		expect(summary.allOnline).toBe(true);
	});
});

describe("selectors: selectDriveSnapshot", () => {
	it("returns null when powertrain telemetry is unavailable", () => {
		const snapshot = selectDriveSnapshot({
			...initialBoardState,
			hasPowertrain: false,
		});

		expect(snapshot).toBeNull();
	});

	it("returns drive snapshot when powertrain telemetry is available", () => {
		const snapshot = selectDriveSnapshot({
			...initialBoardState,
			hasPowertrain: true,
			vehicleSpeed: 83.4,
			gearState: 4,
			accelPedal: 21,
			steeringAngle: -9.5,
			rearMotorRpm: 5100,
			frontMotorRpm: 1200,
		});

		expect(snapshot).toEqual({
			speedKmh: 83.4,
			gear: 4,
			accelPedalPct: 21,
			steeringDeg: -9.5,
			rearMotorRpm: 5100,
			frontMotorRpm: 1200,
		});
	});
});

describe("selectors: selectChargeSnapshot", () => {
	it("returns unavailable when BMS data is unavailable", () => {
		const snapshot = selectChargeSnapshot({
			...initialBoardState,
			hasBms: false,
		});

		expect(snapshot.state).toBe("unavailable");
		expect(snapshot.chargeKw).toBe(0);
	});

	it("returns charging while pack power is negative", () => {
		const snapshot = selectChargeSnapshot({
			...initialBoardState,
			hasBms: true,
			chassisOnline: true,
			bmsSoc: 0.62,
			bmsPower: -7.4,
			bmsChargeTimeToFull: 55,
			bmsEnergyToCharge: 12.6,
		});

		expect(snapshot.state).toBe("charging");
		expect(snapshot.chargeKw).toBe(7.4);
		expect(snapshot.socPercent).toBe(62);
		expect(snapshot.minutesToFull).toBe(55);
		expect(snapshot.energyToChargeKwh).toBe(12.6);
	});

	it("returns fully_charged when fully charged and not actively charging", () => {
		const snapshot = selectChargeSnapshot({
			...initialBoardState,
			hasBms: true,
			chassisOnline: true,
			bmsSoc: 0.99,
			bmsFullyCharged: true,
			bmsPower: 0,
		});

		expect(snapshot.state).toBe("fully_charged");
		expect(snapshot.chargeKw).toBe(0);
		expect(snapshot.socPercent).toBe(99);
	});

	it("returns standby when contactor is engaged but no charge draw exists", () => {
		const snapshot = selectChargeSnapshot({
			...initialBoardState,
			hasBms: true,
			chassisOnline: true,
			bmsSoc: 0.58,
			bmsContactorState: 1,
			bmsPower: 0,
			bmsEnergyToCharge: 4.2,
			bmsChargeTimeToFull: 20,
		});

		expect(snapshot.state).toBe("standby");
		expect(snapshot.chargeKw).toBe(0);
		expect(snapshot.socPercent).toBe(58);
		expect(snapshot.minutesToFull).toBe(20);
	});

	it("returns disconnected when not charging and contactor is open", () => {
		const snapshot = selectChargeSnapshot({
			...initialBoardState,
			hasBms: true,
			chassisOnline: true,
			bmsSoc: 0.47,
			bmsPower: 0,
			bmsContactorState: 0,
		});

		expect(snapshot.state).toBe("disconnected");
		expect(snapshot.chargeKw).toBe(0);
		expect(snapshot.socPercent).toBe(47);
	});
});
