/**
 * Contract guard for C1-06: the OTA TX guard is NOT implemented, so no doc,
 * wire schema, or protocol type may advertise it as shipped behavior.
 *
 * `0x318` identity is disputed (cross-cutting C14: GTW_carState vs RTC frame
 * depending on model/era) and no real vehicle capture exists. The raw ID
 * define must stay for future capture-first work, but every claim that the
 * firmware pauses TX on OTA was removed. This test stops the claim returning
 * silently through a docs or schema edit.
 */
// @ts-expect-error — Node.js built-in, no @types/node
import * as fs from "fs";
// @ts-expect-error — Node.js built-in, no @types/node
import * as path from "path";

declare const process: { cwd(): string };

function findRepoRoot(): string {
	let dir = process.cwd();
	for (let i = 0; i < 8; i++) {
		if (
			fs.existsSync(path.join(dir, "firmware", "platformio.ini")) &&
			fs.existsSync(path.join(dir, "docs", "reference", "commands.md"))
		) {
			return dir;
		}
		const parent = path.dirname(dir);
		if (parent === dir) break;
		dir = parent;
	}
	throw new Error(`Could not locate repository root from ${process.cwd()}`);
}

const repoRoot = findRepoRoot();

/** Legacy research docs are read-only records, not advertised product behavior. */
function walkDocs(dir: string, out: string[]): string[] {
	if (!fs.existsSync(dir)) return out;
	for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
		const entryPath = path.join(dir, entry.name);
		if (entry.isDirectory()) {
			if (entry.name === "legacy") continue;
			walkDocs(entryPath, out);
		} else if (/\.md$/.test(entry.name)) {
			out.push(entryPath);
		}
	}
	return out;
}

function walkTs(dir: string, out: string[]): string[] {
	if (!fs.existsSync(dir)) return out;
	for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
		const entryPath = path.join(dir, entry.name);
		if (entry.isDirectory()) walkTs(entryPath, out);
		else if (/\.ts$/.test(entry.name)) out.push(entryPath);
	}
	return out;
}

const FORBIDDEN_CLAIMS: ReadonlyArray<{ label: string; re: RegExp }> = [
	{ label: "otaInProgress state field", re: /\botaInProgress\b/ },
	{ label: "txPaused state field", re: /\btxPaused\b/ },
	{ label: "GTW_updateInProgress decode", re: /\bGTW_updateInProgress\b/ },
	{
		label: "named OTA guard/safety section",
		re: /OTA\s*(?:Guard|Paused\s+TX|Safety(?:\s+Check)?|Detection)\b/i,
	},
	{ label: "all-TX-paused claim", re: /all CAN TX is paused/i },
	{ label: "injection-pause claim", re: /pauses all CAN frame injection/i },
	{ label: "automatic-pause claim", re: /(?:paused|pause)\s+automatically\s+when\s+OTA/i },
	{ label: "auto-resume claim", re: /TX resumes(?:\s+automatically)?\s+when\s+OTA/i },
	{ label: "0x318 OTA monitoring claim", re: /monitored for OTA update status/i },
	{ label: "OTA-in-progress status claim", re: /OTA in progress/i },
	{ label: "OTA pause state claim", re: /OTA pause state/i },
	{ label: "board OTA auto-detect claim", re: /auto-?detects? OTA/i },
];

function forbiddenClaimsFound(text: string): string[] {
	return FORBIDDEN_CLAIMS.filter(({ re }) => re.test(text)).map(({ label }) => label);
}

const claimTargets: Array<{ label: string; text: string }> = [
	...walkDocs(path.join(repoRoot, "docs"), []).map((file) => ({
		label: path.relative(repoRoot, file),
		text: fs.readFileSync(file, "utf-8"),
	})),
	{
		label: "firmware/lib/io/schemas/io.schema.json",
		text: fs.readFileSync(
			path.join(repoRoot, "firmware", "lib", "io", "schemas", "io.schema.json"),
			"utf-8",
		),
	},
];

describe("OTA TX guard claim removal (C1-06)", () => {
	for (const target of claimTargets) {
		it(`${target.label} advertises no OTA TX guard`, () => {
			expect(forbiddenClaimsFound(target.text)).toEqual([]);
		});
	}

	it("protocol source advertises no OTA state fields", () => {
		const offenders: string[] = [];
		for (const file of walkTs(path.join(repoRoot, "packages", "protocol", "src"), [])) {
			const text: string = fs.readFileSync(file, "utf-8");
			if (/\b(?:otaInProgress|txPaused|txSuppressed)\b/.test(text)) {
				offenders.push(path.relative(repoRoot, file));
			}
		}
		expect(offenders).toEqual([]);
	});

	it("retains the raw CAN_ID_GTW_CAR_STATE define for capture-first work", () => {
		const ids: string = fs.readFileSync(
			path.join(repoRoot, "firmware", "lib", "vehicle", "can", "ids.h"),
			"utf-8",
		);
		expect(ids).toMatch(/#define\s+CAN_ID_GTW_CAR_STATE\s+0x318/);
	});
});
