import { describe, it, expect } from "@jest/globals";
import {
	extractDriveContext,
	assessDriveContextSnapshot,
	evaluateDriveClosureReadiness,
	buildDriveClosureChecklist,
	buildRoadmapEvidenceNote,
} from "../commands/drive-context.js";

describe("extractDriveContext", () => {
	it("maps known fields with defaults", () => {
		const snapshot = extractDriveContext({
			turnSignalLeft: true,
			bsmLeftLevel: 2,
			cruiseSetSpeedKph: 102.5,
			anyDoorOpen: false,
		});

		expect(snapshot.turnSignalLeft).toBe(true);
		expect(snapshot.turnSignalRight).toBe(false);
		expect(snapshot.bsmLeftLevel).toBe(2);
		expect(snapshot.bsmRightLevel).toBe(0);
		expect(snapshot.cruiseSetSpeedKph).toBe(102.5);
		expect(snapshot.anyDoorOpen).toBe(false);
		expect(snapshot.frunkOpen).toBe(false);
	});

	it("coerces invalid numeric values to 0", () => {
		const snapshot = extractDriveContext({
			bsmLeftLevel: "x",
			maxSpeedKph: NaN,
		});

		expect(snapshot.bsmLeftLevel).toBe(0);
		expect(snapshot.maxSpeedKph).toBe(0);
	});
});

describe("assessDriveContextSnapshot", () => {
	it("passes valid snapshot", () => {
		const assessment = assessDriveContextSnapshot({
			turnSignalLeft: false,
			turnSignalRight: false,
			doorFrontLeftOpen: false,
			doorFrontRightOpen: false,
			doorRearLeftOpen: false,
			doorRearRightOpen: false,
			driverDoorOpen: false,
			anyDoorOpen: false,
			frunkOpen: false,
			trunkOpen: false,
			bsmLeftLevel: 0,
			bsmRightLevel: 1,
			cruiseSetSpeedKph: 95,
			accSpeedLimitKph: 0,
			mapSpeedLimitKph: 90,
			maxSpeedKph: 110,
		});

		expect(assessment.ok).toBe(true);
		expect(assessment.errors).toEqual([]);
	});

	it("flags contradiction when opening is true but anyDoorOpen is false", () => {
		const assessment = assessDriveContextSnapshot({
			turnSignalLeft: false,
			turnSignalRight: false,
			doorFrontLeftOpen: true,
			doorFrontRightOpen: false,
			doorRearLeftOpen: false,
			doorRearRightOpen: false,
			driverDoorOpen: false,
			anyDoorOpen: false,
			frunkOpen: false,
			trunkOpen: false,
			bsmLeftLevel: 0,
			bsmRightLevel: 0,
			cruiseSetSpeedKph: 0,
			accSpeedLimitKph: 0,
			mapSpeedLimitKph: 0,
			maxSpeedKph: 0,
		});

		expect(assessment.ok).toBe(false);
		expect(assessment.errors).toContain(
			"anyDoorOpen=false while one or more openings are true",
		);
	});

	it("flags negative BSM levels", () => {
		const assessment = assessDriveContextSnapshot({
			turnSignalLeft: false,
			turnSignalRight: false,
			doorFrontLeftOpen: false,
			doorFrontRightOpen: false,
			doorRearLeftOpen: false,
			doorRearRightOpen: false,
			driverDoorOpen: false,
			anyDoorOpen: false,
			frunkOpen: false,
			trunkOpen: false,
			bsmLeftLevel: -1,
			bsmRightLevel: 0,
			cruiseSetSpeedKph: 0,
			accSpeedLimitKph: 0,
			mapSpeedLimitKph: 0,
			maxSpeedKph: 0,
		});

		expect(assessment.ok).toBe(false);
		expect(assessment.errors).toContain("BSM levels must be >= 0");
	});
});

describe("evaluateDriveClosureReadiness", () => {
	it("returns allReady=true when all gates are satisfied", () => {
		const result = evaluateDriveClosureReadiness(
			{
				d05: {
					turnLeftSeen: true,
					turnRightSeen: true,
					bsmLeftNonZeroSeen: true,
					bsmRightNonZeroSeen: true,
				},
				d11: {
					anyDoorOpenSeen: true,
					driverDoorSeen: true,
					frontLeftSeen: true,
					frontRightSeen: true,
					rearLeftSeen: true,
					rearRightSeen: true,
					frunkSeen: true,
					trunkSeen: true,
				},
				d13: {
					cruiseSetSeen: true,
					accLimitSeen: false,
					mapLimitSeen: true,
					maxSpeedSeen: true,
				},
			},
			true,
		);

		expect(result.d05).toBe(true);
		expect(result.d11).toBe(true);
		expect(result.d13).toBe(true);
		expect(result.allReady).toBe(true);
	});

	it("keeps allReady=false when one gate is missing", () => {
		const result = evaluateDriveClosureReadiness(
			{
				d05: {
					turnLeftSeen: true,
					turnRightSeen: false,
					bsmLeftNonZeroSeen: true,
					bsmRightNonZeroSeen: true,
				},
				d11: {
					anyDoorOpenSeen: true,
					driverDoorSeen: true,
					frontLeftSeen: true,
					frontRightSeen: true,
					rearLeftSeen: true,
					rearRightSeen: true,
					frunkSeen: true,
					trunkSeen: true,
				},
				d13: {
					cruiseSetSeen: true,
					accLimitSeen: true,
					mapLimitSeen: false,
					maxSpeedSeen: true,
				},
			},
			true,
		);

		expect(result.d05).toBe(false);
		expect(result.allReady).toBe(false);
	});

	it("requires D-13 limit evidence in addition to cruise and max speed", () => {
		const result = evaluateDriveClosureReadiness(
			{
				d05: {
					turnLeftSeen: true,
					turnRightSeen: true,
					bsmLeftNonZeroSeen: true,
					bsmRightNonZeroSeen: true,
				},
				d11: {
					anyDoorOpenSeen: true,
					driverDoorSeen: true,
					frontLeftSeen: true,
					frontRightSeen: true,
					rearLeftSeen: true,
					rearRightSeen: true,
					frunkSeen: true,
					trunkSeen: true,
				},
				d13: {
					cruiseSetSeen: true,
					accLimitSeen: false,
					mapLimitSeen: false,
					maxSpeedSeen: true,
				},
			},
			true,
		);

		expect(result.d13).toBe(false);
		expect(result.allReady).toBe(false);
	});
});

describe("buildRoadmapEvidenceNote", () => {
	it("formats note with gate values and artifact path", () => {
		const note = buildRoadmapEvidenceNote(
			{
				generatedAt: "2026-04-19T12:00:00.000Z",
				sampleCount: 12,
				closureReadiness: { d05: true, d11: true, d13: false, allReady: false },
			},
			"artifacts/drive-context-report.json",
		);

		expect(note).toContain("2026-04-19T12:00:00.000Z");
		expect(note).toContain("samples=12");
		expect(note).toContain("d05=true");
		expect(note).toContain("d11=true");
		expect(note).toContain("d13=false");
		expect(note).toContain("allReady=false");
		expect(note).toContain("artifact=artifacts/drive-context-report.json");
	});
});

describe("buildDriveClosureChecklist", () => {
	it("returns empty missing scenarios when all gates are satisfied", () => {
		const coverage = {
			d05: {
				turnLeftSeen: true,
				turnRightSeen: true,
				bsmLeftNonZeroSeen: true,
				bsmRightNonZeroSeen: true,
			},
			d11: {
				anyDoorOpenSeen: true,
				driverDoorSeen: true,
				frontLeftSeen: true,
				frontRightSeen: true,
				rearLeftSeen: true,
				rearRightSeen: true,
				frunkSeen: true,
				trunkSeen: true,
			},
			d13: {
				cruiseSetSeen: true,
				accLimitSeen: false,
				mapLimitSeen: true,
				maxSpeedSeen: true,
			},
		};
		const readiness = evaluateDriveClosureReadiness(coverage, true);
		const checklist = buildDriveClosureChecklist(coverage, readiness);

		expect(checklist.ready).toBe(true);
		expect(checklist.missingScenarioIds).toEqual([]);
	});

	it("maps missing signals to expected ESP32-14 scenario IDs", () => {
		const coverage = {
			d05: {
				turnLeftSeen: false,
				turnRightSeen: true,
				bsmLeftNonZeroSeen: false,
				bsmRightNonZeroSeen: true,
			},
			d11: {
				anyDoorOpenSeen: false,
				driverDoorSeen: false,
				frontLeftSeen: true,
				frontRightSeen: false,
				rearLeftSeen: false,
				rearRightSeen: true,
				frunkSeen: false,
				trunkSeen: false,
			},
			d13: {
				cruiseSetSeen: false,
				accLimitSeen: false,
				mapLimitSeen: false,
				maxSpeedSeen: false,
			},
		};
		const readiness = evaluateDriveClosureReadiness(coverage, true);
		const checklist = buildDriveClosureChecklist(coverage, readiness);

		expect(checklist.ready).toBe(false);
		expect(checklist.missingScenarioIds).toEqual(
			expect.arrayContaining([
				"14.1",
				"14.4",
				"14.6",
				"14.7",
				"14.8",
				"14.9",
				"14.11",
				"14.12",
				"14.13",
			]),
		);
	});
});
