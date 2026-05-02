/**
 * Variant-based feature availability tests.
 *
 * Mirrors the firmware Features struct and getFeatures() logic.
 * Ensures the protocol layer is aware of which commands are valid
 * per variant, that all features default to OFF, and that variant
 * selection (including manual override) works correctly.
 */
import { commands, VALID_VARIANTS, COMMAND_RANGES } from "../../src/commands.js";

// ── Feature availability matrix (mirrors firmware getFeatures()) ─────────────
// true = feature available for this variant

interface VariantFeatures {
	fsd: boolean;
	fsdForce: boolean;
	offset: boolean;
	profile: boolean;
	nag: boolean;
	isaChime: boolean;
	summon: boolean;
}

const FEATURE_MATRIX: Record<string, VariantFeatures> = {
	hw4: {
		fsd: true,
		fsdForce: true,
		offset: true,
		profile: true,
		nag: true,
		isaChime: true,
		summon: true,
	},
	hw3: {
		fsd: true,
		fsdForce: true,
		offset: true,
		profile: true,
		nag: true,
		isaChime: false,
		summon: true,
	},
	legacy: {
		fsd: true,
		fsdForce: true,
		offset: false,
		profile: true,
		nag: true,
		isaChime: false,
		summon: false,
	},
};

// ── Default state (mirrors firmware State constructor) ───────────────────────
// Every boolean feature must default to OFF/false/disabled.

interface DefaultState {
	fsdEnabled: boolean;
	fsdForceEnabled: boolean;
	nagSuppress: boolean;
	isaChimeSuppress: boolean;
	summonInject: boolean;
	nagKillerEnabled: boolean;
	preconditionEnabled: boolean;
	trackModeEnabled: boolean;
	banShieldEnabled: boolean;
	streamEnabled: boolean;
	rawCanListen: boolean;
	profileOverride: boolean;
	offsetOverride: boolean;
	speedOffset: number;
	speedProfile: number;
}

const FIRMWARE_DEFAULTS: DefaultState = {
	fsdEnabled: false,
	fsdForceEnabled: false,
	nagSuppress: false,
	isaChimeSuppress: false,
	summonInject: false,
	nagKillerEnabled: false,
	preconditionEnabled: false,
	trackModeEnabled: false,
	banShieldEnabled: false,
	streamEnabled: false,
	rawCanListen: false,
	profileOverride: false,
	offsetOverride: false,
	speedOffset: 0,
	speedProfile: 1,
};

// ── Tests ────────────────────────────────────────────────────────────────────

describe("Variant feature availability", () => {
	describe.each(["hw4", "hw3", "legacy"] as const)("variant: %s", (variant) => {
		const features = FEATURE_MATRIX[variant];

		it("FSD available", () => {
			expect(features.fsd).toBe(true);
			expect(commands.fsd(true)).toBe("fsd:on");
		});

		it("FSD Force available", () => {
			expect(features.fsdForce).toBe(true);
			expect(commands.fsdForce(true)).toBe("fsd:force:on");
		});

		it("nag available", () => {
			expect(features.nag).toBe(true);
			expect(commands.nag(true)).toBe("nag:on");
		});

		it("profile available", () => {
			expect(features.profile).toBe(true);
			expect(commands.profile(0)).toBe("profile:0");
		});

		it(`offset ${features.offset ? "available" : "NOT available"}`, () => {
			expect(features.offset).toBe(variant !== "legacy");
		});

		it(`isaChime ${features.isaChime ? "available" : "NOT available"}`, () => {
			expect(features.isaChime).toBe(variant === "hw4");
		});

		it(`summon ${features.summon ? "available" : "NOT available"}`, () => {
			expect(features.summon).toBe(variant !== "legacy");
		});
	});

	it("feature matrix covers all valid non-auto variants", () => {
		const matrixKeys = Object.keys(FEATURE_MATRIX).sort();
		const expectedKeys = VALID_VARIANTS.filter((v) => v !== "auto")
			.slice()
			.sort();
		expect(matrixKeys).toEqual(expectedKeys);
	});
});

describe("All features default OFF", () => {
	it("fsdEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.fsdEnabled).toBe(false);
	});

	it("fsdForceEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.fsdForceEnabled).toBe(false);
	});

	it("nagSuppress defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.nagSuppress).toBe(false);
	});

	it("isaChimeSuppress defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.isaChimeSuppress).toBe(false);
	});

	it("summonInject defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.summonInject).toBe(false);
	});

	it("nagKillerEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.nagKillerEnabled).toBe(false);
	});

	it("preconditionEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.preconditionEnabled).toBe(false);
	});

	it("trackModeEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.trackModeEnabled).toBe(false);
	});

	it("banShieldEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.banShieldEnabled).toBe(false);
	});

	it("streamEnabled defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.streamEnabled).toBe(false);
	});

	it("rawCanListen defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.rawCanListen).toBe(false);
	});

	it("profileOverride defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.profileOverride).toBe(false);
	});

	it("offsetOverride defaults to false", () => {
		expect(FIRMWARE_DEFAULTS.offsetOverride).toBe(false);
	});

	it("speedOffset defaults to 0", () => {
		expect(FIRMWARE_DEFAULTS.speedOffset).toBe(0);
	});

	it("speedProfile defaults to 1 (Normal)", () => {
		expect(FIRMWARE_DEFAULTS.speedProfile).toBe(1);
	});
});

describe("Manual variant selection", () => {
	it("variant:hw4 is a valid command", () => {
		expect(commands.variant("hw4")).toBe("variant:hw4");
	});

	it("variant:hw3 is a valid command", () => {
		expect(commands.variant("hw3")).toBe("variant:hw3");
	});

	it("variant:legacy is a valid command", () => {
		expect(commands.variant("legacy")).toBe("variant:legacy");
	});

	it("variant:auto re-enables auto-detection", () => {
		expect(commands.variant("auto")).toBe("variant:auto");
	});

	it("rejects invalid variant", () => {
		expect(() => commands.variant("hw5")).toThrow(RangeError);
		expect(() => commands.variant("")).toThrow(RangeError);
		expect(() => commands.variant("HW4")).toThrow(RangeError); // case-sensitive
	});

	it("VALID_VARIANTS includes all accepted values", () => {
		expect(VALID_VARIANTS).toContain("hw3");
		expect(VALID_VARIANTS).toContain("hw4");
		expect(VALID_VARIANTS).toContain("legacy");
		expect(VALID_VARIANTS).toContain("auto");
		expect(VALID_VARIANTS).toHaveLength(4);
	});
});

describe("Offset range by variant", () => {
	it("HW3 offset range is 0-100", () => {
		expect(COMMAND_RANGES.offset.min).toBe(0);
		expect(COMMAND_RANGES.offset.max).toBe(100);
		expect(commands.offset(100)).toBe("offset:100");
		expect(() => commands.offset(101)).toThrow(RangeError);
	});

	it("offset:off disables override", () => {
		expect(commands.offsetOff()).toBe("offset:off");
	});

	it("offset:auto enables auto-tracking", () => {
		expect(commands.offsetAuto()).toBe("offset:auto");
	});
});
