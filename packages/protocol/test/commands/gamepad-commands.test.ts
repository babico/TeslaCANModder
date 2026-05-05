import { commands } from "../../src/commands.js";

describe("Gamepad commands", () => {
	describe("scan / pair / unpair", () => {
		it("gamepadScan", () => expect(commands.gamepadScan()).toBe("gamepad:scan"));
		it("gamepadUnpair", () => expect(commands.gamepadUnpair()).toBe("gamepad:unpair"));
		it("gamepadStatus", () => expect(commands.gamepadStatus()).toBe("gamepad:status"));
		it("gamepadCancel", () => expect(commands.gamepadCancel()).toBe("gamepad:cancel"));

		it("gamepadPair lowercases the MAC address", () =>
			expect(commands.gamepadPair("AA:BB:CC:DD:EE:FF")).toBe(
				"gamepad:pair:aa:bb:cc:dd:ee:ff",
			));
		it("gamepadPair accepts already-lowercase", () =>
			expect(commands.gamepadPair("11:22:33:44:55:66")).toBe(
				"gamepad:pair:11:22:33:44:55:66",
			));
		it("gamepadPair rejects malformed addresses", () => {
			expect(() => commands.gamepadPair("AA-BB-CC-DD-EE-FF")).toThrow(RangeError);
			expect(() => commands.gamepadPair("not-a-mac")).toThrow(RangeError);
			expect(() => commands.gamepadPair("AA:BB:CC:DD:EE")).toThrow(RangeError);
			expect(() => commands.gamepadPair("")).toThrow(RangeError);
		});
	});

	describe("enable / disable", () => {
		it("on", () => expect(commands.gamepad(true)).toBe("gamepad:on"));
		it("off", () => expect(commands.gamepad(false)).toBe("gamepad:off"));
	});

	describe("bind / hold", () => {
		it("gamepadBind", () =>
			expect(commands.gamepadBind(0, "lock")).toBe("gamepad:bind:0:lock"));
		it("gamepadHold", () =>
			expect(commands.gamepadHold(7, "horn")).toBe("gamepad:hold:7:horn"));
		it("gamepadBind accepts max button index", () =>
			expect(commands.gamepadBind(15, "unlock")).toBe("gamepad:bind:15:unlock"));

		it("gamepadBind rejects out-of-range indexes", () => {
			expect(() => commands.gamepadBind(-1, "lock")).toThrow(RangeError);
			expect(() => commands.gamepadBind(16, "lock")).toThrow(RangeError);
			expect(() => commands.gamepadBind(1.5, "lock")).toThrow(RangeError);
		});
		it("gamepadHold rejects out-of-range indexes", () => {
			expect(() => commands.gamepadHold(-1, "lock")).toThrow(RangeError);
			expect(() => commands.gamepadHold(16, "lock")).toThrow(RangeError);
		});
	});

	describe("axis tuning", () => {
		it("gamepadAxis dz", () =>
			expect(commands.gamepadAxis(0, "dz", 10)).toBe("gamepad:axis:0:dz:10"));
		it("gamepadAxis expo", () =>
			expect(commands.gamepadAxis(2, "expo", 50)).toBe("gamepad:axis:2:expo:50"));
		it("gamepadAxis inv", () =>
			expect(commands.gamepadAxis(5, "inv", 1)).toBe("gamepad:axis:5:inv:1"));
		it("gamepadAxis accepts boundary indexes 0..5", () => {
			expect(commands.gamepadAxis(0, "dz", 0)).toBe("gamepad:axis:0:dz:0");
			expect(commands.gamepadAxis(5, "dz", 0)).toBe("gamepad:axis:5:dz:0");
		});
		it("gamepadAxis rejects out-of-range axis", () => {
			expect(() => commands.gamepadAxis(-1, "dz", 5)).toThrow(RangeError);
			expect(() => commands.gamepadAxis(6, "dz", 5)).toThrow(RangeError);
		});
		it("gamepadAxis rejects unknown kind", () => {
			expect(() => commands.gamepadAxis(0, "weird" as any, 5)).toThrow(RangeError);
		});
	});
});
