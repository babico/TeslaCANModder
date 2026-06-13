/**
 * New Wire Field Reducer Tests
 *
 * Tests that the reducer correctly maps newly-emitted firmware wire fields
 * (`fwBuild` from the firmware section, `vehicleLockedState` from the
 * vehicle section, `gtwAutopilotSeen` from the state section) from
 * boot/status messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from "../../src/reducer.js";
import type { BootMessage, StatusMessage, BoardState } from "../../src/types.js";

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, "00:00:00");

describe("reducer: new wire fields", () => {
	beforeEach(() => {
		nextId = 0;
	});

	describe("initialBoardState defaults", () => {
		it("zeroes fwBuild", () => {
			expect(initialBoardState.fwBuild).toBe(0);
		});

		it("defaults vehicleLockedState to false", () => {
			expect(initialBoardState.vehicleLockedState).toBe(false);
		});

		it("defaults gtwAutopilotSeen to false", () => {
			expect(initialBoardState.gtwAutopilotSeen).toBe(false);
		});
	});

	describe("applyBoot with new wire fields", () => {
		it("maps fwBuild from boot firmware section", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				fwBuild: 4242,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.fwBuild).toBe(4242);
		});

		it("maps vehicleLockedState from boot vehicle section", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				vehicleLockedState: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.vehicleLockedState).toBe(true);
		});

		it("maps gtwAutopilotSeen from boot state section", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				gtwAutopilotSeen: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.gtwAutopilotSeen).toBe(true);
		});

		it("maps all three new wire fields together", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				fwBuild: 99999,
				vehicleLockedState: 0,
				gtwAutopilotSeen: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.fwBuild).toBe(99999);
			expect(state.vehicleLockedState).toBe(false);
			expect(state.gtwAutopilotSeen).toBe(true);
		});
	});

	describe("applyStatus with new wire fields", () => {
		it("maps fwBuild from status firmware section", () => {
			const state = reduce(initialBoardState, {
				t: "status",
				fwBuild: 8888,
			} satisfies Partial<StatusMessage>);
			expect(state.fwBuild).toBe(8888);
		});

		it("maps vehicleLockedState from status vehicle section", () => {
			const state = reduce(initialBoardState, {
				t: "status",
				vehicleLockedState: 1,
			} satisfies Partial<StatusMessage>);
			expect(state.vehicleLockedState).toBe(true);
		});

		it("maps gtwAutopilotSeen from status state section", () => {
			const state = reduce(initialBoardState, {
				t: "status",
				gtwAutopilotSeen: 0,
			} satisfies Partial<StatusMessage>);
			expect(state.gtwAutopilotSeen).toBe(false);
		});

		it("preserves previous values when fields are absent", () => {
			const seeded = reduce(initialBoardState, {
				t: "status",
				fwBuild: 1234,
				vehicleLockedState: 1,
				gtwAutopilotSeen: 1,
			} satisfies Partial<StatusMessage>);
			const after = reduce(seeded, { t: "status" } satisfies Partial<StatusMessage>);
			expect(after.fwBuild).toBe(1234);
			expect(after.vehicleLockedState).toBe(true);
			expect(after.gtwAutopilotSeen).toBe(true);
		});
	});
});
