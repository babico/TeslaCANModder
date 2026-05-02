/**
 * Body Feature Commands Tests
 * Tests all new command builders for turn signals, seatbelt, air recirc,
 * wiper persist, mirror auto-fold, powertrain, and CAN simulation.
 */

import { commands } from "../../src/commands.js";

describe("body feature commands", () => {
	describe("Turn Signals", () => {
		it("turnLeft3", () => expect(commands.turnLeft3()).toBe("turn:left3"));
		it("turnRight3", () => expect(commands.turnRight3()).toBe("turn:right3"));
		it("turnHazard", () => expect(commands.turnHazard()).toBe("turn:hazard"));
		it("turnOff", () => expect(commands.turnOff()).toBe("turn:off"));
	});

	describe("Seatbelt Emulation", () => {
		it("on", () => expect(commands.seatbelt(true)).toBe("seatbelt:on"));
		it("off", () => expect(commands.seatbelt(false)).toBe("seatbelt:off"));
	});

	describe("Air Recirculation", () => {
		it("on", () => expect(commands.airRecirc(true)).toBe("airecirc:on"));
		it("off", () => expect(commands.airRecirc(false)).toBe("airecirc:off"));
	});

	describe("Wiper Persist", () => {
		it("on", () => expect(commands.wiperPersist(true)).toBe("wiperpersist:on"));
		it("off", () => expect(commands.wiperPersist(false)).toBe("wiperpersist:off"));
	});

	describe("Mirror Auto-Fold", () => {
		it("on", () => expect(commands.mirrorAutoFold(true)).toBe("mirror:autofold:on"));
		it("off", () => expect(commands.mirrorAutoFold(false)).toBe("mirror:autofold:off"));
	});

	describe("Powertrain", () => {
		it("powertrain", () => expect(commands.powertrain()).toBe("powertrain"));
	});

	describe("CAN Simulation", () => {
		it("start", () => expect(commands.canSimStart()).toBe("simu:start"));
		it("stop", () => expect(commands.canSimStop()).toBe("simu:stop"));
	});
});
