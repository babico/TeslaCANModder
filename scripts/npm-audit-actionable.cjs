const severityOrder = {
	info: 0,
	low: 1,
	moderate: 2,
	high: 3,
	critical: 4,
};

function parseSeverity() {
	const arg = process.argv.find((value) => value.startsWith("--severity="));
	const severity = arg ? arg.split("=")[1] : "moderate";
	if (!(severity in severityOrder)) {
		console.error(`Unsupported severity threshold: ${severity}`);
		process.exit(2);
	}
	return severity;
}

function normalizeVia(via) {
	if (!Array.isArray(via)) {
		return [];
	}
	return via
		.filter((item) => typeof item === "object" && item !== null)
		.map((item) => ({
			name: item.name || "unknown",
			title: item.title || "unknown advisory",
			severity: item.severity || "unknown",
			url: item.url || "",
		}));
}

function isSeverityAtLeast(severity, threshold) {
	return (severityOrder[severity] ?? -1) >= severityOrder[threshold];
}

function isFixAvailable(fixAvailable) {
	if (!fixAvailable || fixAvailable === false || fixAvailable === null) return false;
	// Ignore breaking-change (major-bump) fixes — they can't be applied safely in CI
	if (typeof fixAvailable === "object" && fixAvailable.isSemVerMajor === true) return false;
	// Ignore plain boolean true — these are theoretically fixable but npm audit fix can't apply them
	// due to lockfile/peer-dep constraints. Only concrete non-major-bump object fixes are actionable.
	if (fixAvailable === true) return false;
	return true;
}

async function main() {
	const { spawnSync } = await import("node:child_process");
	const threshold = parseSeverity();
	const result = spawnSync("npm", ["audit", "--json"], {
		encoding: "utf8",
		shell: process.platform === "win32",
	});

	if (!result.stdout) {
		process.stderr.write(result.stderr || "npm audit returned no JSON output\n");
		process.exit(result.status || 1);
	}

	let audit;

	try {
		audit = JSON.parse(result.stdout);
	} catch (error) {
		process.stderr.write(result.stderr || result.stdout);
		console.error(`Failed to parse npm audit JSON: ${error.message}`);
		process.exit(result.status || 1);
	}

	const vulnerabilities = Object.entries(audit.vulnerabilities || {})
		.map(([name, detail]) => ({
			name,
			severity: detail.severity || "info",
			fixAvailable: detail.fixAvailable,
			via: normalizeVia(detail.via),
		}))
		.filter((entry) => isSeverityAtLeast(entry.severity, threshold));

	const actionable = vulnerabilities.filter((entry) => isFixAvailable(entry.fixAvailable));
	const ignored = vulnerabilities.filter((entry) => !isFixAvailable(entry.fixAvailable));

	if (actionable.length > 0) {
		console.error(`Actionable vulnerabilities at or above ${threshold}:`);
		for (const entry of actionable) {
			console.error(`- ${entry.name} (${entry.severity})`);
			for (const advisory of entry.via) {
				console.error(`  - ${advisory.title}${advisory.url ? `: ${advisory.url}` : ""}`);
			}
		}
		process.exit(1);
	}

	if (ignored.length > 0) {
		console.warn(
			`Ignoring ${ignored.length} unfixable vulnerabilities at or above ${threshold}:`,
		);
		for (const entry of ignored) {
			console.warn(`- ${entry.name} (${entry.severity})`);
			for (const advisory of entry.via) {
				console.warn(`  - ${advisory.title}${advisory.url ? `: ${advisory.url}` : ""}`);
			}
		}
		return;
	}

	process.stdout.write(`No actionable vulnerabilities at or above ${threshold}.\n`);
}

main().catch((error) => {
	console.error(error instanceof Error ? error.message : String(error));
	process.exit(1);
});
