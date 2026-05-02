/**
 * Config Feature Commands Tests
 * Tests command builders for single-shot TX, FW compat, MQTT bridge,
 * and vehicle config.
 */

import { commands } from "../../src/commands.js";

describe("config commands", () => {
	describe("Single-Shot TX", () => {
		it("on", () => expect(commands.singleShot(true)).toBe("singleshot:on"));
		it("off", () => expect(commands.singleShot(false)).toBe("singleshot:off"));
	});

	describe("FW Compat", () => {
		it("query", () => expect(commands.fwCompat()).toBe("fwcompat"));
	});

	describe("MQTT Bridge", () => {
		it("on", () => expect(commands.mqtt(true)).toBe("mqtt:on"));
		it("off", () => expect(commands.mqtt(false)).toBe("mqtt:off"));
		it("broker", () =>
			expect(commands.mqttBroker("192.168.1.10")).toBe("mqtt:broker:192.168.1.10"));
		it("port", () => expect(commands.mqttPort(1883)).toBe("mqtt:port:1883"));
		it("interval", () => expect(commands.mqttInterval(5000)).toBe("mqtt:interval:5000"));

		it("rejects empty broker", () => {
			expect(() => commands.mqttBroker("")).toThrow(RangeError);
		});
		it("rejects broker > 63 chars", () => {
			expect(() => commands.mqttBroker("a".repeat(64))).toThrow(RangeError);
		});
		it("rejects port 0", () => {
			expect(() => commands.mqttPort(0)).toThrow(RangeError);
		});
		it("rejects port > 65535", () => {
			expect(() => commands.mqttPort(65536)).toThrow(RangeError);
		});
		it("rejects interval < 100", () => {
			expect(() => commands.mqttInterval(50)).toThrow(RangeError);
		});
		it("rejects interval > 60000", () => {
			expect(() => commands.mqttInterval(70000)).toThrow(RangeError);
		});
	});

	describe("Vehicle Config", () => {
		it("query", () => expect(commands.vehicle()).toBe("vehicle"));
	});
});
