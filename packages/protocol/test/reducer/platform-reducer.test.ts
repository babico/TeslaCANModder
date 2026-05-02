/**
 * Platform & CAN Health Protocol Tests
 * Tests that the reducer correctly maps platform identity and CAN health
 * fields from boot/status messages to board state.
 */

import { initialBoardState, reduceBoardMessage } from "../../src/reducer.js";
import type {
	BootMessage,
	StatusMessage,
	PlatformMessage,
	StatusCanMessage,
	StatusStateMessage,
	StatusCompactMessage,
	BoardState,
} from "../../src/types.js";

let nextId = 0;
const id = () => ++nextId;
const reduce = (prev: BoardState, msg: any) => reduceBoardMessage(prev, msg, id, "00:00:00");

describe("reducer: vehicle platform", () => {
	beforeEach(() => {
		nextId = 0;
	});

	describe("initialBoardState defaults", () => {
		it("has platform defaults", () => {
			expect(initialBoardState.platformModel).toBe(0);
			expect(initialBoardState.platformHwGen).toBe(0);
			expect(initialBoardState.platformSwYear).toBe(0);
			expect(initialBoardState.platformSwWeek).toBe(0);
			expect(initialBoardState.platformSwRelease).toBe(0);
			expect(initialBoardState.platformFsdProto).toBe(0);
			expect(initialBoardState.platformSwCompat).toBe(0);
			expect(initialBoardState.platformResolved).toBe(false);
		});

		it("has empty canHealth", () => {
			expect(initialBoardState.canHealth).toEqual({});
		});
	});

	describe("applyBoot with platform fields", () => {
		it("maps platform identity from boot", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				platformModel: 4, // MODEL_Y
				platformHwGen: 3, // HW_4
				platformSwYear: 2026,
				platformSwWeek: 14,
				platformSwRelease: 1,
				platformFsdProto: 3, // FSD_PROTO_V14
				platformSwCompat: 1, // SW_COMPAT_OK
				platformResolved: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.platformModel).toBe(4);
			expect(state.platformHwGen).toBe(3);
			expect(state.platformSwYear).toBe(2026);
			expect(state.platformSwWeek).toBe(14);
			expect(state.platformSwRelease).toBe(1);
			expect(state.platformFsdProto).toBe(3);
			expect(state.platformSwCompat).toBe(1);
			expect(state.platformResolved).toBe(true);
		});

		it("maps canHealth from boot", () => {
			const msg: Partial<BootMessage> = {
				t: "boot",
				hw: "ESP32S",
				canHealth: {
					chassis: { on: 1, det: 1 },
					vehicle: { on: 1, det: 0 },
					body: { on: 0, det: 0 },
				},
			};
			const state = reduce(initialBoardState, msg);
			expect(state.canHealth.chassis).toEqual({ on: true, det: true });
			expect(state.canHealth.vehicle).toEqual({ on: true, det: false });
			expect(state.canHealth.body).toEqual({ on: false, det: false });
		});

		it("preserves platform fields when not in message", () => {
			const prev = { ...initialBoardState, platformModel: 3, platformResolved: true };
			const msg: Partial<BootMessage> = { t: "boot", hw: "ESP32S" };
			const state = reduce(prev, msg);
			expect(state.platformModel).toBe(3);
			expect(state.platformResolved).toBe(true);
		});
	});

	describe("applyStatus with platform fields", () => {
		it("maps platform identity from status", () => {
			const msg: Partial<StatusMessage> = {
				t: "status",
				platformModel: 5, // MODEL_CYBERTRUCK
				platformHwGen: 3, // HW_4
				platformSwYear: 2026,
				platformSwWeek: 2,
				platformSwRelease: 9,
				platformFsdProto: 3, // FSD_PROTO_V14
				platformSwCompat: 1,
				platformResolved: 1,
			};
			const state = reduce(initialBoardState, msg);
			expect(state.platformModel).toBe(5);
			expect(state.platformHwGen).toBe(3);
			expect(state.platformResolved).toBe(true);
		});

		it("maps canHealth from status", () => {
			const msg: Partial<StatusMessage> = {
				t: "status",
				canHealth: {
					chassis: { on: 1, det: 1 },
					vehicle: { on: 1, det: 1 },
					body: { on: 1, det: 1 },
				},
			};
			const state = reduce(initialBoardState, msg);
			expect(Object.keys(state.canHealth)).toHaveLength(3);
			expect(state.canHealth.chassis?.det).toBe(true);
		});
	});

	describe("split status and platform query reducers", () => {
		it("maps direct platform query responses", () => {
			const msg: PlatformMessage = {
				t: "platform",
				model: 1,
				hwGen: 3,
				swYear: 2026,
				swWeek: 14,
				swRelease: 1,
				fsdProto: 3,
				swCompat: 1,
				resolved: 1,
				canHealth: {
					chassis: { on: 1, det: 1 },
					vehicle: { on: 1, det: 1 },
					body: { on: 0, det: 0 },
				},
			};

			const state = reduce(initialBoardState, msg);
			expect(state.platformModel).toBe(1);
			expect(state.platformResolved).toBe(true);
			expect(state.canHealth.body).toEqual({ on: false, det: false });
		});

		it("applies status_can patches without clobbering other state", () => {
			const prev = { ...initialBoardState, fsd: true, profile: 3 };
			const msg: StatusCanMessage = {
				t: "status_can",
				canClockReqMHz: 16,
				canClockMHz: 8,
				canHealth: {
					chassis: { on: 1, det: 1 },
					vehicle: { on: 1, det: 0 },
				},
			};

			const state = reduce(prev, msg);
			expect(state.fsd).toBe(true);
			expect(state.profile).toBe(3);
			expect(state.canClockReqMHz).toBe(16);
			expect(state.canHealth.vehicle).toEqual({ on: true, det: false });
		});

		it("applies status_state patches without resetting unrelated values", () => {
			const prev = { ...initialBoardState, platformModel: 4, canClockMHz: 20 };
			const msg: StatusStateMessage = {
				t: "status_state",
				fsd: 1,
				fsdForce: 0,
				nag: 1,
				nagKiller: 1,
				sp: 2,
				spPin: 1,
				offset: 7,
				offsetPin: 0,
				precondition: 1,
				trackMode: 0,
				apGateEnabled: 1,
				apGateOpen: 0,
				apGateReason: "waiting",
			};

			const state = reduce(prev, msg);
			expect(state.fsd).toBe(true);
			expect(state.nag).toBe(true);
			expect(state.profile).toBe(2);
			expect(state.profilePinned).toBe(true);
			expect(state.offset).toBe(7);
			expect(state.apGateEnabled).toBe(true);
			expect(state.apGateOpen).toBe(false);
			expect(state.apGateReason).toBe("waiting");
			expect(state.platformModel).toBe(4);
			expect(state.canClockMHz).toBe(20);
		});

		it("applies compact status patches", () => {
			const prev = { ...initialBoardState, nagKiller: true, platformModel: 2 };
			const msg: StatusCompactMessage = {
				t: "status_compact",
				hw: "ESP32S_DevKit",
				variant: "hw4",
				up: 321,
				chassisOnline: 1,
				vehicleOnline: 1,
				bodyOnline: 0,
				standby: 0,
				fsd: 1,
				fsdForce: 1,
				nag: 0,
				sp: 3,
				offset: 9,
				state: { apGateEnabled: 1, apGateOpen: 1, apGateReason: "parked" },
				features: {
					fsd: true,
					fsdForce: true,
					offset: true,
					profile: true,
					nag: true,
					isaSpeedChime: true,
					summon: true,
				},
				canClockReqMHz: 8,
				canClockMHz: 8,
				canHealth: {
					chassis: { on: 1, det: 1 },
					vehicle: { on: 1, det: 1 },
					body: { on: 0, det: 0 },
				},
				stream: { on: 1, emitted: 44 },
			};

			const state = reduce(prev, msg);
			expect(state.hardware).toBe("ESP32S_DevKit");
			expect(state.uptime).toBe(321);
			expect(state.fsd).toBe(true);
			expect(state.fsdForce).toBe(true);
			expect(state.nag).toBe(false);
			expect(state.profile).toBe(3);
			expect(state.apGateEnabled).toBe(true);
			expect(state.apGateOpen).toBe(true);
			expect(state.apGateReason).toBe("parked");
			expect(state.streaming).toBe(true);
			expect(state.canHealth.chassis).toEqual({ on: true, det: true });
			expect(state.platformModel).toBe(2);
			expect(state.nagKiller).toBe(true);
		});
	});
});

describe("commands: platform", () => {
	it("has platform command", async () => {
		const { commands } = await import("../../src/commands.js");
		expect(commands.platform()).toBe("platform");
	});
});
