/**
 * CAN Diagnostic Reducer Tests
 * Tests that the reducer correctly maps the firmware-emitted canDiag fields
 * (nagEchoCount, eapModCount, txFailCount, busOffCount, frames, hz, hzMin, hzMax)
 * from boot/status messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from "../../src/reducer.js";
import type { BootMessage, StatusMessage, BoardState } from "../../src/types.js";

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, "00:00:00");

describe("reducer: canDiag counters", () => {
	beforeEach(() => {
		nextId = 0;
	});

	describe("initialBoardState defaults", () => {
		it("zeroes aggregate canDiag counters", () => {
			expect(initialBoardState.canNagEchoCount).toBe(0);
			expect(initialBoardState.canEapModCount).toBe(0);
			expect(initialBoardState.canTxFailCount).toBe(0);
			expect(initialBoardState.canBusOffCount).toBe(0);
		});

		it("starts with empty per-bus metric maps", () => {
			expect(initialBoardState.canFrames).toEqual({});
			expect(initialBoardState.canHz).toEqual({});
			expect(initialBoardState.canHzMin).toEqual({});
			expect(initialBoardState.canHzMax).toEqual({});
		});
	});

	describe("applyBoot with canDiag fields", () => {
		it("maps aggregate counters from boot", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				canNagEchoCount: 12,
				canEapModCount: 4,
				canTxFailCount: 2,
				canBusOffCount: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.canNagEchoCount).toBe(12);
			expect(state.canEapModCount).toBe(4);
			expect(state.canTxFailCount).toBe(2);
			expect(state.canBusOffCount).toBe(1);
		});

		it("maps per-bus metrics from boot", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				canFrames: { chassis: 100, vehicle: 200, body: 0 },
				canHz: { chassis: 450, vehicle: 220, body: 0 },
				canHzMin: { chassis: 400, vehicle: 200, body: 0 },
				canHzMax: { chassis: 500, vehicle: 250, body: 0 },
			};
			const state = reduce(initialBoardState, msg);
			expect(state.canFrames).toEqual({ chassis: 100, vehicle: 200, body: 0 });
			expect(state.canHz).toEqual({ chassis: 450, vehicle: 220, body: 0 });
			expect(state.canHzMin).toEqual({ chassis: 400, vehicle: 200, body: 0 });
			expect(state.canHzMax).toEqual({ chassis: 500, vehicle: 250, body: 0 });
		});
	});

	describe("applyStatus with canDiag fields", () => {
		it("updates aggregate counters from status", () => {
			const state = reduce(initialBoardState, {
				t: "status",
				canNagEchoCount: 100,
				canEapModCount: 50,
				canTxFailCount: 5,
				canBusOffCount: 2,
			} satisfies Partial<StatusMessage>);
			expect(state.canNagEchoCount).toBe(100);
			expect(state.canEapModCount).toBe(50);
			expect(state.canTxFailCount).toBe(5);
			expect(state.canBusOffCount).toBe(2);
		});

		it("merges per-bus metrics incrementally on status updates", () => {
			const first = reduce(initialBoardState, {
				t: "status",
				canFrames: { chassis: 100, vehicle: 200, body: 0 },
			} satisfies Partial<StatusMessage>);
			const second = reduce(first, {
				t: "status",
				canHz: { chassis: 450, vehicle: 220, body: 80 },
			} satisfies Partial<StatusMessage>);
			expect(second.canFrames).toEqual({ chassis: 100, vehicle: 200, body: 0 });
			expect(second.canHz).toEqual({ chassis: 450, vehicle: 220, body: 80 });
			expect(second.canHzMin).toEqual({});
		});

		it("preserves previous values when canDiag fields are absent", () => {
			const seeded = reduce(initialBoardState, {
				t: "status",
				canNagEchoCount: 99,
				canTxFailCount: 7,
			} satisfies Partial<StatusMessage>);
			const after = reduce(seeded, { t: "status" } satisfies Partial<StatusMessage>);
			expect(after.canNagEchoCount).toBe(99);
			expect(after.canTxFailCount).toBe(7);
		});
	});
});
