import { commands } from "../../src/commands.js";

describe("new commands", () => {
	describe("Drive Mode", () => {
		it("driveModeOff", () => expect(commands.driveModeOff()).toBe("drivemode:off"));
		it("driveModeChill", () => expect(commands.driveModeChill()).toBe("drivemode:chill"));
		it("driveModeStandard", () =>
			expect(commands.driveModeStandard()).toBe("drivemode:standard"));
		it("driveModePerformance", () =>
			expect(commands.driveModePerformance()).toBe("drivemode:performance"));
	});

	describe("TPMS", () => {
		it("tpms", () => expect(commands.tpms()).toBe("tpms"));
	});

	describe("ECE R79", () => {
		it("on", () => expect(commands.eceR79(true)).toBe("ecer79:on"));
		it("off", () => expect(commands.eceR79(false)).toBe("ecer79:off"));
	});

	describe("AP unlock and restore controls", () => {
		it("gtwShieldArm", () => expect(commands.gtwShieldArm()).toBe("gtwshield:arm"));
		it("gtwShieldDisarm", () => expect(commands.gtwShieldDisarm()).toBe("gtwshield:disarm"));
		it("gtwShieldReset", () => expect(commands.gtwShieldReset()).toBe("gtwshield:reset"));
		it("apGate on", () => expect(commands.apGate(true)).toBe("apgate:on"));
		it("apGate off", () => expect(commands.apGate(false)).toBe("apgate:off"));
		it("apGate status", () => expect(commands.apGateStatus()).toBe("apgate:status"));
		it("tlssc on", () => expect(commands.tlssc(true)).toBe("tlssc:on"));
		it("tlssc off", () => expect(commands.tlssc(false)).toBe("tlssc:off"));
		it("eap on", () => expect(commands.eap(true)).toBe("eap:on"));
		it("eap off", () => expect(commands.eap(false)).toBe("eap:off"));
		it("evd on", () => expect(commands.evd(true)).toBe("evd:on"));
		it("evd off", () => expect(commands.evd(false)).toBe("evd:off"));
	});

	describe("Log", () => {
		it("log", () => expect(commands.log()).toBe("log"));
	});

	describe("Button Remapping", () => {
		it("btnMap lamp short trunk", () =>
			expect(commands.btnMap("lamp", "short", "trunk")).toBe("btnmap:lamp:short:trunk"));
		it("btnMap parking long sentry", () =>
			expect(commands.btnMap("parking", "long", "sentry")).toBe(
				"btnmap:parking:long:sentry",
			));
		it("btnMap lamp double horn", () =>
			expect(commands.btnMap("lamp", "double", "horn")).toBe("btnmap:lamp:double:horn"));
		it("btnMapQuery", () => expect(commands.btnMapQuery()).toBe("btnmap:query"));
		it("btnMapReset", () => expect(commands.btnMapReset()).toBe("btnmap:reset"));
		it("rejects invalid button", () =>
			expect(() => commands.btnMap("invalid" as never, "short", "trunk")).toThrow());
		it("rejects invalid press", () =>
			expect(() => commands.btnMap("lamp", "triple" as never, "trunk")).toThrow());
		it("rejects invalid action", () =>
			expect(() => commands.btnMap("lamp", "short", "fly" as never)).toThrow());
	});

	describe("Speed Camera Alert", () => {
		it("on", () => expect(commands.speedAlert(true)).toBe("speedalert:on"));
		it("off", () => expect(commands.speedAlert(false)).toBe("speedalert:off"));
	});

	describe("GVRET", () => {
		it("on", () => expect(commands.gvret(true)).toBe("gvret:on"));
		it("off", () => expect(commands.gvret(false)).toBe("gvret:off"));
		it("port 23", () => expect(commands.gvretPort(23)).toBe("gvret:port:23"));
		it("port 65535", () => expect(commands.gvretPort(65535)).toBe("gvret:port:65535"));
		it("rejects port 0", () => expect(() => commands.gvretPort(0)).toThrow());
		it("rejects port 70000", () => expect(() => commands.gvretPort(70000)).toThrow());
	});

	describe("ESP-NOW", () => {
		it("on", () => expect(commands.espNow(true)).toBe("espnow:on"));
		it("off", () => expect(commands.espNow(false)).toBe("espnow:off"));
		it("channel 1", () => expect(commands.espNowChannel(1)).toBe("espnow:channel:1"));
		it("channel 13", () => expect(commands.espNowChannel(13)).toBe("espnow:channel:13"));
		it("rejects channel 0", () => expect(() => commands.espNowChannel(0)).toThrow());
		it("rejects channel 14", () => expect(() => commands.espNowChannel(14)).toThrow());
	});

	describe("ScanMyTesla", () => {
		it("on", () => expect(commands.scanMyTesla(true)).toBe("smt:on"));
		it("off", () => expect(commands.scanMyTesla(false)).toBe("smt:off"));
	});

	describe("ELM327", () => {
		it("on", () => expect(commands.elm327(true)).toBe("elm327:on"));
		it("off", () => expect(commands.elm327(false)).toBe("elm327:off"));
	});

	describe("Tesla BLE", () => {
		it("on", () => expect(commands.teslaBle(true)).toBe("teslable:on"));
		it("off", () => expect(commands.teslaBle(false)).toBe("teslable:off"));
		it("auth", () => expect(commands.teslaBleAuth()).toBe("teslable:auth"));
		it("forget", () => expect(commands.teslaBleForget()).toBe("teslable:forget"));
	});

	describe("Home Assistant", () => {
		it("on", () => expect(commands.homeAssistant(true)).toBe("ha:on"));
		it("off", () => expect(commands.homeAssistant(false)).toBe("ha:off"));
		it("discovery", () => expect(commands.haDiscovery()).toBe("ha:discovery"));
		it("interval 1000", () => expect(commands.haInterval(1000)).toBe("ha:interval:1000"));
		it("interval 5000", () => expect(commands.haInterval(5000)).toBe("ha:interval:5000"));
		it("rejects interval 499", () => expect(() => commands.haInterval(499)).toThrow());
		it("rejects interval 60001", () => expect(() => commands.haInterval(60001)).toThrow());
	});

	describe("Encrypted BLE", () => {
		it("on", () => expect(commands.bleEncrypt(true)).toBe("bleencrypt:on"));
		it("off", () => expect(commands.bleEncrypt(false)).toBe("bleencrypt:off"));
		it("pair", () => expect(commands.blePair()).toBe("bleencrypt:pair"));
		it("unpair", () => expect(commands.bleUnpair()).toBe("bleencrypt:unpair"));
	});
});
