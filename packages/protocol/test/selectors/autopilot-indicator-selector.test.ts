import { initialBoardState } from "../../src/reducer.js";
import { selectAutopilotIndicatorState } from "../../src/selectors.js";

describe("selectors: selectAutopilotIndicatorState", () => {
	it("reports unavailable when chassis is offline", () => {
		const indicator = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: false,
			gtwAutopilotTier: 3,
		});

		expect(indicator.apState).toBe("unavailable");
		expect(indicator.label).toBe("AP N/A");
		expect(indicator.variant).toBe("muted");
	});

	it("reports unavailable when tier is zero even if chassis is online", () => {
		const indicator = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: true,
			gtwAutopilotTier: 0,
		});

		expect(indicator.apState).toBe("unavailable");
		expect(indicator.label).toBe("AP N/A");
	});

	it("reports inactive for low non-zero tiers with no hands warning", () => {
		const highway = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: true,
			gtwAutopilotTier: 1,
			dasHandsOn: 0,
		});

		expect(highway.apState).toBe("inactive");
		expect(highway.label).toBe("HIGHWAY");
		expect(highway.variant).toBe("muted");

		const enhanced = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: true,
			gtwAutopilotTier: 2,
			dasHandsOn: 0,
		});

		expect(enhanced.apState).toBe("inactive");
		expect(enhanced.label).toBe("ENHANCED");
	});

	it("reports active for tier 3+ when no hands warning exists", () => {
		const indicator = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: true,
			gtwAutopilotTier: 3,
			dasHandsOn: 0,
		});

		expect(indicator.apState).toBe("active");
		expect(indicator.label).toBe("SELF_DRIVING");
		expect(indicator.variant).toBe("success");
		expect(indicator.showHandsWarning).toBe(false);
	});

	it("prioritizes hands warning over active/inactive labels", () => {
		const indicator = selectAutopilotIndicatorState({
			...initialBoardState,
			chassisOnline: true,
			gtwAutopilotTier: 3,
			dasHandsOn: 2,
		});

		expect(indicator.apState).toBe("hands_warning");
		expect(indicator.label).toBe("Hands On");
		expect(indicator.variant).toBe("warning");
		expect(indicator.showHandsWarning).toBe(true);
	});

	it("mirrors nag signal independently of AP state", () => {
		const indicator = selectAutopilotIndicatorState({
			...initialBoardState,
			nag: true,
			chassisOnline: true,
			gtwAutopilotTier: 1,
		});

		expect(indicator.showNag).toBe(true);
	});
});
