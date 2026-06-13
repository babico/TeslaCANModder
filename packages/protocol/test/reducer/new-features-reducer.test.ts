/**
 * New Features Reducer Tests
 *
 * Tests that the reducer correctly maps TPMS, drive mode, ECE R79, region,
 * and rate limit fields from boot/status/tpms messages to board state.
 */

import { reduceBoardMessage, initialBoardState } from "../../src/reducer.js";
import type { BootMessage, StatusMessage, TpmsMessage, BoardState } from "../../src/types.js";

let nextMessageId = 0;
function boardReducer(
	state: BoardState,
	msg: BootMessage | StatusMessage | TpmsMessage,
): BoardState {
	return reduceBoardMessage(state, msg, () => ++nextMessageId, "00:00:00");
}

describe("reducer: new feature fields", () => {
	describe("initialBoardState", () => {
		it("has TPMS defaults", () => {
			expect(initialBoardState.hasTpms).toBe(false);
			expect(initialBoardState.tpmsPressureFL).toBe(0);
			expect(initialBoardState.tpmsPressureFR).toBe(0);
			expect(initialBoardState.tpmsPressureRL).toBe(0);
			expect(initialBoardState.tpmsPressureRR).toBe(0);
			expect(initialBoardState.tpmsTempFL).toBe(0);
			expect(initialBoardState.tpmsTempFR).toBe(0);
			expect(initialBoardState.tpmsTempRL).toBe(0);
			expect(initialBoardState.tpmsTempRR).toBe(0);
		});

		it("has drive mode defaults", () => {
			expect(initialBoardState.driveMode).toBe(0);
			expect(initialBoardState.currentDriveMode).toBe(0);
		});

		it("has region defaults", () => {
			expect(initialBoardState.regionCode).toBe(0);
			expect(initialBoardState.hasRegion).toBe(false);
			expect(initialBoardState.cnLocked).toBe(false);
		});

		it("has ECE R79 default", () => {
			expect(initialBoardState.eceR79).toBe(false);
		});

		it("has AP unlock/restore defaults", () => {
			expect(initialBoardState.gtwShieldArmed).toBe(false);
			expect(initialBoardState.gtwShieldBlocks).toBe(0);
			expect(initialBoardState.enhancedAutopilot).toBe(false);
			expect(initialBoardState.evdEnabled).toBe(false);
			expect(initialBoardState.tlsscRestore).toBe(false);
		});
	});

	describe("applyBoot with new fields", () => {
		it("maps drive mode fields from boot", () => {
			const msg: BootMessage = {
				t: "boot",
				hw: "ESP32S",
				drv: "MCP2515",
				variant: "hw4",
				driveMode: 2,
				currentDriveMode: 1,
				eceR79: 1,
				regionCode: 2,
				hasRegion: 1,
				cnLocked: 0,
				hasTpms: 1,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.driveMode).toBe(2);
			expect(state.currentDriveMode).toBe(1);
			expect(state.eceR79).toBe(true);
			expect(state.regionCode).toBe(2);
			expect(state.hasRegion).toBe(true);
			expect(state.cnLocked).toBe(false);
			expect(state.hasTpms).toBe(true);
		});

		it("maps AP unlock/restore fields from boot", () => {
			const msg: BootMessage = {
				t: "boot",
				hw: "ESP32S",
				drv: "MCP2515",
				variant: "hw4",
				gtwShieldArmed: 1,
				gtwShieldBlocks: 9,
				eap: 1,
				evd: 1,
				tlssc: 1,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.gtwShieldArmed).toBe(true);
			expect(state.gtwShieldBlocks).toBe(9);
			expect(state.enhancedAutopilot).toBe(true);
			expect(state.evdEnabled).toBe(true);
			expect(state.tlsscRestore).toBe(true);
		});
	});

	describe("applyStatus with new fields", () => {
		it("maps all new status fields", () => {
			const msg: StatusMessage = {
				t: "status",
				fsd: 1,
				up: 500,
				driveMode: 3,
				currentDriveMode: 3,
				eceR79: 1,
				regionCode: 3,
				hasRegion: 1,
				cnLocked: 1,
				hasTpms: 0,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.driveMode).toBe(3);
			expect(state.currentDriveMode).toBe(3);
			expect(state.eceR79).toBe(true);
			expect(state.regionCode).toBe(3);
			expect(state.hasRegion).toBe(true);
			expect(state.cnLocked).toBe(true);
			expect(state.hasTpms).toBe(false);
		});

		it("maps AP unlock/restore fields from status", () => {
			const msg: StatusMessage = {
				t: "status",
				fsd: 1,
				up: 100,
				gtwShieldArmed: 0,
				gtwShieldBlocks: 14,
				eap: 1,
				evd: 0,
				tlssc: 1,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.gtwShieldArmed).toBe(false);
			expect(state.gtwShieldBlocks).toBe(14);
			expect(state.enhancedAutopilot).toBe(true);
			expect(state.evdEnabled).toBe(false);
			expect(state.tlsscRestore).toBe(true);
		});
	});

	describe("TPMS message", () => {
		it("applies TPMS pressure and temperature", () => {
			const msg: TpmsMessage = {
				t: "tpms",
				fl: 300,
				fr: 280,
				rl: 310,
				rr: 290,
				tfl: 25,
				tfr: 27,
				trl: 23,
				trr: 26,
				ok: 1,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.hasTpms).toBe(true);
			expect(state.tpmsPressureFL).toBe(3.0);
			expect(state.tpmsPressureFR).toBe(2.8);
			expect(state.tpmsPressureRL).toBe(3.1);
			expect(state.tpmsPressureRR).toBe(2.9);
			expect(state.tpmsTempFL).toBe(25);
			expect(state.tpmsTempFR).toBe(27);
			expect(state.tpmsTempRL).toBe(23);
			expect(state.tpmsTempRR).toBe(26);
		});

		it("sets hasTpms from ok field", () => {
			const msg: TpmsMessage = {
				t: "tpms",
				fl: 0,
				fr: 0,
				rl: 0,
				rr: 0,
				tfl: 0,
				tfr: 0,
				trl: 0,
				trr: 0,
				ok: 0,
			};
			const state = boardReducer({ ...initialBoardState }, msg);
			expect(state.hasTpms).toBe(false);
		});
	});
});
