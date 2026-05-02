/**
 * Config Feature Reducer Tests
 * Tests that the reducer correctly maps new feature fields from
 * boot/status/fwcompat/vehicle messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from "../../src/reducer.js";
import type {
	BootMessage,
	StatusMessage,
	FwCompatMessage,
	VehicleConfigMessage,
	BoardState,
} from "../../src/types.js";

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, "00:00:00");

describe("reducer: config features", () => {
	beforeEach(() => {
		nextId = 0;
	});

	describe("initialBoardState defaults", () => {
		it("has singleShot default off", () => {
			expect(initialBoardState.singleShot).toBe(false);
		});

		it("has fw version defaults", () => {
			expect(initialBoardState.fwYear).toBe(0);
			expect(initialBoardState.fwRelease).toBe(0);
			expect(initialBoardState.fwMinor).toBe(0);
			expect(initialBoardState.fwCompat).toBe(0);
			expect(initialBoardState.hasFwVersion).toBe(false);
		});

		it("has mqtt defaults", () => {
			expect(initialBoardState.mqtt).toBe(false);
			expect(initialBoardState.mqttConnected).toBe(false);
		});

		it("has vehicle config defaults", () => {
			expect(initialBoardState.vehicleModel).toBe(0);
			expect(initialBoardState.vehicleYear).toBe(0);
			expect(initialBoardState.hasVehicleConfig).toBe(false);
		});
	});

	describe("applyBoot with config fields", () => {
		it("maps new fields from boot", () => {
			const msg = {
				t: "boot",
				hw: "ESP32S",
				drv: "MCP2515",
				var: "hw4",
				brd: "devkit",
				up: 100,
				feat: "F",
				singleShot: 1,
				fwYear: 2024,
				fwRelease: 44,
				fwMinor: 12,
				fwCompat: 1,
				hasFwVersion: 1,
				mqtt: 1,
				mqttConnected: 0,
				vehicleModel: 1,
				vehicleYear: 2024,
				hasVehicleConfig: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.singleShot).toBe(true);
			expect(state.fwYear).toBe(2024);
			expect(state.fwRelease).toBe(44);
			expect(state.fwMinor).toBe(12);
			expect(state.fwCompat).toBe(1);
			expect(state.hasFwVersion).toBe(true);
			expect(state.mqtt).toBe(true);
			expect(state.mqttConnected).toBe(false);
			expect(state.vehicleModel).toBe(1);
			expect(state.vehicleYear).toBe(2024);
			expect(state.hasVehicleConfig).toBe(true);
		});
	});

	describe("applyStatus with config fields", () => {
		it("maps new status fields", () => {
			const msg = {
				t: "status",
				fsd: 1,
				nag: 0,
				sp: 0,
				spPin: 0,
				offset: 0,
				offsetPin: 0,
				isaChime: 0,
				feat: "F",
				chassisOnline: 1,
				standby: 0,
				vehicleOnline: 1,
				summonInject: 0,
				nagKiller: 0,
				nagKillerMode: "legacy",
				dasHandsOn: 0,
				precondition: 0,
				trackMode: 0,
				otaInProgress: 0,
				txPaused: 0,
				detectedHW: 0,
				gtwAutopilotTier: -1,
				canClockReqMHz: 8,
				canClockMHz: 8,
				banShield: 0,
				banThreat: 0,
				banDetectCount: 0,
				bmsV: 0,
				bmsA: 0,
				bmsW: 0,
				bmsSoC: 0,
				bmsN: 0,
				bmsX: 0,
				bmsEff: 0,
				hasBms: 0,
				fsdForce: 0,
				sumI: 0,
				steeringMode: 0,
				hasSteeringMode: 0,
				driveMode: 0,
				currentDriveMode: 0,
				eceR79: 0,
				regionCode: 0,
				hasRegion: 0,
				cnLocked: 0,
				rateLimit: 1,
				hasTpms: 0,
				seatbeltEmulation: 0,
				wiperPersist: 0,
				mirrorAutoFold: 0,
				canSim: 0,
				hasPowertrain: 0,
				singleShot: 1,
				fwYear: 2024,
				fwRelease: 44,
				fwMinor: 12,
				fwCompat: 2,
				hasFwVersion: 1,
				mqtt: 1,
				mqttConnected: 1,
				vehicleModel: 2,
				vehicleYear: 2023,
				hasVehicleConfig: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.singleShot).toBe(true);
			expect(state.fwYear).toBe(2024);
			expect(state.fwRelease).toBe(44);
			expect(state.fwCompat).toBe(2);
			expect(state.hasFwVersion).toBe(true);
			expect(state.mqtt).toBe(true);
			expect(state.mqttConnected).toBe(true);
			expect(state.vehicleModel).toBe(2);
			expect(state.vehicleYear).toBe(2023);
			expect(state.hasVehicleConfig).toBe(true);
		});
	});

	describe("fwcompat message", () => {
		it("decodes firmware compatibility", () => {
			const msg: FwCompatMessage = {
				t: "fwcompat",
				year: 2024,
				release: 44,
				minor: 12,
				build: 3001,
				compat: 1,
				ok: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.fwYear).toBe(2024);
			expect(state.fwRelease).toBe(44);
			expect(state.fwMinor).toBe(12);
			expect(state.fwCompat).toBe(1);
			expect(state.hasFwVersion).toBe(true);
		});

		it("handles zero values", () => {
			const msg: FwCompatMessage = {
				t: "fwcompat",
				year: 0,
				release: 0,
				minor: 0,
				build: 0,
				compat: 0,
				ok: 0,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.fwYear).toBe(0);
			expect(state.fwCompat).toBe(0);
			expect(state.hasFwVersion).toBe(false);
		});
	});

	describe("vehicle message", () => {
		it("decodes vehicle config", () => {
			const msg: VehicleConfigMessage = {
				t: "vehicle",
				model: 1,
				year: 2024,
				ok: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.vehicleModel).toBe(1);
			expect(state.vehicleYear).toBe(2024);
			expect(state.hasVehicleConfig).toBe(true);
		});

		it("handles unknown model", () => {
			const msg: VehicleConfigMessage = {
				t: "vehicle",
				model: 0,
				year: 0,
				ok: 0,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.vehicleModel).toBe(0);
			expect(state.hasVehicleConfig).toBe(false);
		});
	});
});
