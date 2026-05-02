import {
	formatDriveMode,
	formatRegion,
	formatPressureBar,
	formatPressurePsi,
} from "../../src/format.js";

describe("new format functions", () => {
	describe("formatDriveMode", () => {
		it("0 → OFF", () => expect(formatDriveMode(0)).toBe("OFF"));
		it("1 → CHILL", () => expect(formatDriveMode(1)).toBe("CHILL"));
		it("2 → STANDARD", () => expect(formatDriveMode(2)).toBe("STANDARD"));
		it("3 → PERFORMANCE", () => expect(formatDriveMode(3)).toBe("PERFORMANCE"));
		it("unknown → UNKNOWN", () => expect(formatDriveMode(99)).toBe("UNKNOWN"));
	});

	describe("formatRegion", () => {
		it("0 → UNKNOWN", () => expect(formatRegion(0)).toBe("UNKNOWN"));
		it("1 → NA", () => expect(formatRegion(1)).toBe("NA"));
		it("2 → EU", () => expect(formatRegion(2)).toBe("EU"));
		it("3 → CN", () => expect(formatRegion(3)).toBe("CN"));
		it("4 → APAC", () => expect(formatRegion(4)).toBe("APAC"));
		it("5 → ME", () => expect(formatRegion(5)).toBe("ME"));
		it("unknown → UNKNOWN", () => expect(formatRegion(255)).toBe("UNKNOWN"));
	});

	describe("formatPressureBar", () => {
		it("formats to 2 decimal places with unit", () => {
			expect(formatPressureBar(2.5)).toBe("2.50 bar");
		});
		it("handles zero", () => {
			expect(formatPressureBar(0)).toBe("0.00 bar");
		});
	});

	describe("formatPressurePsi", () => {
		it("converts bar to psi", () => {
			// 1 bar = 14.5038 psi
			const result = formatPressurePsi(1.0);
			expect(result).toMatch(/^14\.\d+ psi$/);
		});
		it("handles zero", () => {
			expect(formatPressurePsi(0)).toBe("0.0 psi");
		});
		it("typical tire pressure", () => {
			// 2.5 bar ≈ 36.3 psi
			const result = formatPressurePsi(2.5);
			expect(result).toMatch(/^36\.\d+ psi$/);
		});
	});
});
