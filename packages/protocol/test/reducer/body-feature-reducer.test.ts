/**
 * Body Feature Reducer Tests
 * Tests that the reducer correctly maps new feature fields from
 * boot/status/powertrain messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from "../../src/reducer.js";
import type { BootMessage, StatusMessage, PowertrainMessage, BoardState } from "../../src/types.js";

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, "00:00:00");

describe("reducer: body features", () => {
	beforeEach(() => {
		nextId = 0;
	});
	describe("initialBoardState defaults", () => {
		it("has seatbelt default off", () => {
			expect(initialBoardState.seatbeltEmulation).toBe(false);
		});

		it("has wiper persist default off", () => {
			expect(initialBoardState.wiperPersist).toBe(false);
		});

		it("has mirror auto-fold default off", () => {
			expect(initialBoardState.mirrorAutoFold).toBe(false);
		});

		it("has CAN sim default off", () => {
			expect(initialBoardState.canSim).toBe(false);
		});

		it("has powertrain defaults", () => {
			expect(initialBoardState.hasPowertrain).toBe(false);
			expect(initialBoardState.vehicleSpeed).toBe(0);
			expect(initialBoardState.gearState).toBe(0);
			expect(initialBoardState.accelPedal).toBe(0);
			expect(initialBoardState.steeringAngle).toBe(0);
			expect(initialBoardState.rearMotorRpm).toBe(0);
			expect(initialBoardState.frontMotorRpm).toBe(0);
		});
	});

	describe("applyBoot with body feature fields", () => {
		it("maps new boolean fields from boot", () => {
			const msg = {
				t: "boot",
				hw: "ESP32S",
				drv: "MCP2515",
				var: "hw4",
				brd: "devkit",
				up: 100,
				feat: "F",
				seatbeltEmulation: 1,
				wiperPersist: 1,
				mirrorAutoFold: 1,
				canSim: 0,
				hasPowertrain: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.seatbeltEmulation).toBe(true);
			expect(state.wiperPersist).toBe(true);
			expect(state.mirrorAutoFold).toBe(true);
			expect(state.canSim).toBe(false);
			expect(state.hasPowertrain).toBe(true);
		});
	});

	describe("applyStatus with body feature fields", () => {
		it("maps new status fields", () => {
			const msg = {
				t: "status",
				fsd: 1,
				nag: 0,
				prof: 0,
				profP: 0,
				off: 0,
				offP: 0,
				isa: 0,
				feat: "F",
				chas: 1,
				stb: 0,
				veh: 1,
				sum: 0,
				nagk: 0,
				nagkm: "legacy",
				dho: 0,
				pc: 0,
				trk: 0,
				ota: 0,
				txp: 0,
				hw: 0,
				gtw: -1,
				clkR: 8,
				clk: 8,
				ban: 0,
				banT: 0,
				banD: 0,
				bmsV: 0,
				bmsA: 0,
				bmsW: 0,
				bmsSoC: 0,
				bmsN: 0,
				bmsX: 0,
				bmsEff: 0,
				hasBms: 0,
				fsdF: 0,
				sumI: 0,
				steerM: 0,
				hasSteerM: 0,
				driveMode: 0,
				currentDriveMode: 0,
				eceR79: 0,
				regionCode: 0,
				hasRegion: 0,
				cnLocked: 0,
				rateLimit: 1,
				hasTpms: 0,
				seatbeltEmulation: 1,
				wiperPersist: 0,
				mirrorAutoFold: 1,
				canSim: 1,
				hasPowertrain: 0,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.seatbeltEmulation).toBe(true);
			expect(state.wiperPersist).toBe(false);
			expect(state.mirrorAutoFold).toBe(true);
			expect(state.canSim).toBe(true);
			expect(state.hasPowertrain).toBe(false);
		});
	});

	describe("powertrain message", () => {
		it("decodes powertrain telemetry", () => {
			const msg: PowertrainMessage = {
				t: "powertrain",
				speed: 6000,
				gear: 4,
				pedal: 15,
				brake: 0,
				steer: 450,
				rpmR: 5000,
				rpmF: 3000,
				wsFL: 0,
				wsFR: 0,
				wsRL: 0,
				wsRR: 0,
				hasWs: 0,
				rInvT: 0,
				rStatT: 0,
				rHsT: 0,
				fInvT: 0,
				fStatT: 0,
				fHsT: 0,
				hasMotorT: 0,
				ok: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.hasPowertrain).toBe(true);
			expect(state.vehicleSpeed).toBeCloseTo(60.0, 1);
			expect(state.gearState).toBe(4);
			expect(state.accelPedal).toBe(15);
			expect(state.steeringAngle).toBeCloseTo(45.0, 1);
			expect(state.rearMotorRpm).toBe(5000);
			expect(state.frontMotorRpm).toBe(3000);
		});

		it("handles zero powertrain values", () => {
			const msg: PowertrainMessage = {
				t: "powertrain",
				speed: 0,
				gear: 0,
				pedal: 0,
				brake: 0,
				steer: 0,
				rpmR: 0,
				rpmF: 0,
				wsFL: 0,
				wsFR: 0,
				wsRL: 0,
				wsRR: 0,
				hasWs: 0,
				rInvT: 0,
				rStatT: 0,
				rHsT: 0,
				fInvT: 0,
				fStatT: 0,
				fHsT: 0,
				hasMotorT: 0,
				ok: 1,
			};
			const state = reduce({ ...initialBoardState }, msg);
			expect(state.vehicleSpeed).toBe(0);
			expect(state.gearState).toBe(0);
			expect(state.rearMotorRpm).toBe(0);
		});
	});
});
