/**
 * drive-context command — capture runtime evidence for D-05 / D-11 / D-13.
 *
 * Watches status messages for drive-context fields and emits a report with:
 * - latest snapshot
 * - observed feature coverage during capture window
 * - basic consistency checks
 */

import { writeFileSync } from "node:fs";
import path from "node:path";

export const DRIVE_CONTEXT_NUMERIC_FIELDS = [
	"bsmLeftLevel",
	"bsmRightLevel",
	"cruiseSetSpeedKph",
	"accSpeedLimitKph",
	"mapSpeedLimitKph",
	"maxSpeedKph",
];

export const DRIVE_CONTEXT_BOOLEAN_FIELDS = [
	"turnSignalLeft",
	"turnSignalRight",
	"doorFrontLeftOpen",
	"doorFrontRightOpen",
	"doorRearLeftOpen",
	"doorRearRightOpen",
	"driverDoorOpen",
	"anyDoorOpen",
	"frunkOpen",
	"trunkOpen",
];

export function extractDriveContext(status) {
	const snapshot = {};

	for (const key of DRIVE_CONTEXT_BOOLEAN_FIELDS) {
		snapshot[key] = typeof status?.[key] === "boolean" ? status[key] : false;
	}

	for (const key of DRIVE_CONTEXT_NUMERIC_FIELDS) {
		const val = Number(status?.[key]);
		snapshot[key] = Number.isFinite(val) ? val : 0;
	}

	return snapshot;
}

export function assessDriveContextSnapshot(snapshot) {
	const errors = [];

	if (snapshot.bsmLeftLevel < 0 || snapshot.bsmRightLevel < 0) {
		errors.push("BSM levels must be >= 0");
	}

	if (
		!Number.isFinite(snapshot.cruiseSetSpeedKph) ||
		!Number.isFinite(snapshot.accSpeedLimitKph) ||
		!Number.isFinite(snapshot.mapSpeedLimitKph) ||
		!Number.isFinite(snapshot.maxSpeedKph)
	) {
		errors.push("Speed context values must be finite numbers");
	}

	const anyPhysicalOpen =
		snapshot.driverDoorOpen ||
		snapshot.doorFrontLeftOpen ||
		snapshot.doorFrontRightOpen ||
		snapshot.doorRearLeftOpen ||
		snapshot.doorRearRightOpen ||
		snapshot.frunkOpen ||
		snapshot.trunkOpen;

	if (anyPhysicalOpen && !snapshot.anyDoorOpen) {
		errors.push("anyDoorOpen=false while one or more openings are true");
	}

	return {
		ok: errors.length === 0,
		errors,
	};
}

function buildCoverage() {
	return {
		d05: {
			turnLeftSeen: false,
			turnRightSeen: false,
			bsmLeftNonZeroSeen: false,
			bsmRightNonZeroSeen: false,
		},
		d11: {
			anyDoorOpenSeen: false,
			driverDoorSeen: false,
			frontLeftSeen: false,
			frontRightSeen: false,
			rearLeftSeen: false,
			rearRightSeen: false,
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
}

function mergeCoverage(coverage, s) {
	coverage.d05.turnLeftSeen ||= s.turnSignalLeft;
	coverage.d05.turnRightSeen ||= s.turnSignalRight;
	coverage.d05.bsmLeftNonZeroSeen ||= s.bsmLeftLevel > 0;
	coverage.d05.bsmRightNonZeroSeen ||= s.bsmRightLevel > 0;

	coverage.d11.anyDoorOpenSeen ||= s.anyDoorOpen;
	coverage.d11.driverDoorSeen ||= s.driverDoorOpen;
	coverage.d11.frontLeftSeen ||= s.doorFrontLeftOpen;
	coverage.d11.frontRightSeen ||= s.doorFrontRightOpen;
	coverage.d11.rearLeftSeen ||= s.doorRearLeftOpen;
	coverage.d11.rearRightSeen ||= s.doorRearRightOpen;
	coverage.d11.frunkSeen ||= s.frunkOpen;
	coverage.d11.trunkSeen ||= s.trunkOpen;

	coverage.d13.cruiseSetSeen ||= s.cruiseSetSpeedKph > 0;
	coverage.d13.accLimitSeen ||= s.accSpeedLimitKph > 0;
	coverage.d13.mapLimitSeen ||= s.mapSpeedLimitKph > 0;
	coverage.d13.maxSpeedSeen ||= s.maxSpeedKph > 0;
}

export function evaluateDriveClosureReadiness(coverage, consistencyOk) {
	const d05Ready =
		coverage.d05.turnLeftSeen &&
		coverage.d05.turnRightSeen &&
		coverage.d05.bsmLeftNonZeroSeen &&
		coverage.d05.bsmRightNonZeroSeen;

	const d11Ready =
		coverage.d11.driverDoorSeen &&
		coverage.d11.frontLeftSeen &&
		coverage.d11.frontRightSeen &&
		coverage.d11.rearLeftSeen &&
		coverage.d11.rearRightSeen &&
		coverage.d11.frunkSeen &&
		coverage.d11.trunkSeen &&
		coverage.d11.anyDoorOpenSeen;

	const d13Ready =
		coverage.d13.cruiseSetSeen &&
		coverage.d13.maxSpeedSeen &&
		(coverage.d13.mapLimitSeen || coverage.d13.accLimitSeen);

	return {
		d05: d05Ready,
		d11: d11Ready,
		d13: d13Ready,
		consistencyOk,
		allReady: d05Ready && d11Ready && d13Ready && consistencyOk,
		notes: {
			d05: d05Ready
				? "Ready to close after evidence review"
				: "Missing one or more D-05 coverage signals",
			d11: d11Ready
				? "Ready to close after evidence review"
				: "Missing one or more D-11 opening-state signals",
			d13: d13Ready
				? "Ready to close after evidence review"
				: "Missing cruise/max/limit evidence for D-13",
			consistency: consistencyOk
				? "No contradictions detected"
				: "Consistency check failed; review errors",
		},
	};
}

export function buildDriveClosureChecklist(coverage, readiness) {
	const missing = {
		d05: [],
		d11: [],
		d13: [],
	};

	if (!coverage.d05.turnLeftSeen)
		missing.d05.push({ signal: "turnLeftSeen", scenarioId: "14.1" });
	if (!coverage.d05.turnRightSeen)
		missing.d05.push({ signal: "turnRightSeen", scenarioId: "14.2" });
	if (!coverage.d05.bsmLeftNonZeroSeen)
		missing.d05.push({ signal: "bsmLeftNonZeroSeen", scenarioId: "14.4" });
	if (!coverage.d05.bsmRightNonZeroSeen)
		missing.d05.push({ signal: "bsmRightNonZeroSeen", scenarioId: "14.5" });

	if (!coverage.d11.driverDoorSeen)
		missing.d11.push({ signal: "driverDoorSeen", scenarioId: "14.6" });
	if (!coverage.d11.frontLeftSeen)
		missing.d11.push({ signal: "frontLeftSeen", scenarioId: "14.7" });
	if (!coverage.d11.frontRightSeen)
		missing.d11.push({ signal: "frontRightSeen", scenarioId: "14.7" });
	if (!coverage.d11.rearLeftSeen)
		missing.d11.push({ signal: "rearLeftSeen", scenarioId: "14.7" });
	if (!coverage.d11.rearRightSeen)
		missing.d11.push({ signal: "rearRightSeen", scenarioId: "14.7" });
	if (!coverage.d11.frunkSeen) missing.d11.push({ signal: "frunkSeen", scenarioId: "14.8" });
	if (!coverage.d11.trunkSeen) missing.d11.push({ signal: "trunkSeen", scenarioId: "14.9" });
	if (!coverage.d11.anyDoorOpenSeen)
		missing.d11.push({ signal: "anyDoorOpenSeen", scenarioId: "14.6/14.7" });

	if (!coverage.d13.cruiseSetSeen)
		missing.d13.push({ signal: "cruiseSetSeen", scenarioId: "14.11" });
	if (!coverage.d13.maxSpeedSeen)
		missing.d13.push({ signal: "maxSpeedSeen", scenarioId: "14.12" });
	if (!(coverage.d13.mapLimitSeen || coverage.d13.accLimitSeen)) {
		missing.d13.push({ signal: "mapOrAccLimitSeen", scenarioId: "14.13" });
	}

	const missingScenarioIds = [
		...new Set([
			...missing.d05.map((m) => m.scenarioId),
			...missing.d11.map((m) => m.scenarioId),
			...missing.d13.map((m) => m.scenarioId),
		]),
	];

	return {
		ready: readiness.allReady,
		missing,
		missingScenarioIds,
	};
}

export function buildRoadmapEvidenceNote(report, artifactPath = null) {
	const gates = report?.closureReadiness ?? {};
	const stamp = report?.generatedAt ?? new Date().toISOString();
	const sampleCount = Number(report?.sampleCount) || 0;
	const pathText = artifactPath ? ` artifact=${artifactPath}` : "";

	return `- ${stamp}: drive-context strict capture completed (samples=${sampleCount}; d05=${Boolean(gates.d05)}; d11=${Boolean(gates.d11)}; d13=${Boolean(gates.d13)}; allReady=${Boolean(gates.allReady)}).${pathText}`;
}

export async function runDriveContext(session, opts, out) {
	const durationMs = opts.driveCtxDurMs;
	const outputPath = opts.driveCtxOutput;
	const noteOutputPath = opts.driveCtxNoteOutput;
	const expectFull = Boolean(opts.driveCtxExpectFull);
	const minSamples = Number(opts.driveCtxMinSamples) || 1;

	out.section("Drive Context Evidence Capture");
	out.info(`duration=${durationMs}ms`);
	out.info(`minSamples=${minSamples}`);
	out.info(`expectFull=${expectFull}`);

	session.send("status");

	const deadline = Date.now() + durationMs;
	let sampleCount = 0;
	let lastSnapshot = null;
	const consistencyErrors = [];
	const coverage = buildCoverage();

	while (Date.now() < deadline) {
		const waitMs = Math.max(100, deadline - Date.now());
		const entry = await session.waitFor((e) => e.msg?.t === "status", waitMs);
		if (!entry) break;

		const snapshot = extractDriveContext(entry.msg);
		const assessment = assessDriveContextSnapshot(snapshot);
		if (!assessment.ok) {
			consistencyErrors.push(...assessment.errors);
		}

		mergeCoverage(coverage, snapshot);
		lastSnapshot = snapshot;
		sampleCount += 1;
	}

	if (sampleCount === 0) {
		out.fail("No status samples captured", "Ensure stream/status output is available");
		return;
	}

	if (sampleCount < minSamples) {
		out.fail(
			"Insufficient evidence samples",
			`captured=${sampleCount}, required=${minSamples}`,
		);
	}

	const report = {
		generatedAt: new Date().toISOString(),
		durationMs,
		sampleCount,
		latest: lastSnapshot,
		coverage,
		consistency: {
			ok: consistencyErrors.length === 0,
			errors: [...new Set(consistencyErrors)],
		},
	};

	report.closureReadiness = evaluateDriveClosureReadiness(coverage, report.consistency.ok);
	report.closureChecklist = buildDriveClosureChecklist(coverage, report.closureReadiness);

	const d05Covered =
		coverage.d05.turnLeftSeen ||
		coverage.d05.turnRightSeen ||
		coverage.d05.bsmLeftNonZeroSeen ||
		coverage.d05.bsmRightNonZeroSeen;
	const d11Covered =
		coverage.d11.anyDoorOpenSeen ||
		coverage.d11.driverDoorSeen ||
		coverage.d11.frontLeftSeen ||
		coverage.d11.frontRightSeen ||
		coverage.d11.rearLeftSeen ||
		coverage.d11.rearRightSeen ||
		coverage.d11.frunkSeen ||
		coverage.d11.trunkSeen;
	const d13Covered =
		coverage.d13.cruiseSetSeen ||
		coverage.d13.accLimitSeen ||
		coverage.d13.mapLimitSeen ||
		coverage.d13.maxSpeedSeen;

	if (d05Covered) out.pass("D-05 signal activity observed");
	else out.warn("D-05 signal activity not observed");

	if (d11Covered) out.pass("D-11 open-state activity observed");
	else out.warn("D-11 open-state activity not observed");

	if (d13Covered) out.pass("D-13 speed-context activity observed");
	else out.warn("D-13 speed-context activity not observed");

	if (report.consistency.ok) out.pass("Consistency checks", "No contradictions detected");
	else out.warn("Consistency checks", report.consistency.errors.join("; "));

	if (report.closureReadiness.d05)
		out.pass("D-05 closure gate", report.closureReadiness.notes.d05);
	else out.warn("D-05 closure gate", report.closureReadiness.notes.d05);

	if (report.closureReadiness.d11)
		out.pass("D-11 closure gate", report.closureReadiness.notes.d11);
	else out.warn("D-11 closure gate", report.closureReadiness.notes.d11);

	if (report.closureReadiness.d13)
		out.pass("D-13 closure gate", report.closureReadiness.notes.d13);
	else out.warn("D-13 closure gate", report.closureReadiness.notes.d13);

	if (report.closureReadiness.allReady)
		out.pass("Roadmap recommendation", "D-05/D-11/D-13 can move to done after evidence review");
	else
		out.warn(
			"Roadmap recommendation",
			"Keep tasks in in-review until all closure gates are true",
		);

	if (!report.closureChecklist.ready) {
		out.info(
			`missing-scenarios: ${report.closureChecklist.missingScenarioIds.join(", ") || "none"}`,
		);
	}

	if (expectFull && !report.closureReadiness.allReady) {
		out.fail(
			"Expected full closure readiness",
			"One or more D-05/D-11/D-13 gates are not satisfied",
		);
	}

	if (outputPath) {
		const abs = path.resolve(outputPath);
		writeFileSync(abs, JSON.stringify(report, null, 2), "utf8");
		out.pass("Report written", abs);

		const noteLine = buildRoadmapEvidenceNote(report, abs);
		out.info(`roadmap-note: ${noteLine}`);
		if (noteOutputPath) {
			const noteAbs = path.resolve(noteOutputPath);
			writeFileSync(noteAbs, `${noteLine}\n`, "utf8");
			out.pass("Roadmap note written", noteAbs);
		}
	} else {
		console.log(`\n${JSON.stringify(report, null, 2)}`);
		console.log(`\n${buildRoadmapEvidenceNote(report)}`);
	}
}
