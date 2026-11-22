/**
 * Repo-level guard: no command may be advertised without a firmware dispatch
 * branch, unless the docs row explicitly marks it UNSUPPORTED.
 *
 * This test greps firmware source at test time (unlike the hand-maintained list
 * in cross-check.test.ts) so a docs/protocol-only command cannot ship silently.
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
const docsContent: string = fs.readFileSync(
	path.join(repoRoot, "docs", "reference", "commands.md"),
	"utf-8",
);
const dispatchSource: string = fs.readFileSync(
	path.join(repoRoot, "firmware", "lib", "client", "command", "dispatch.h"),
	"utf-8",
);

interface DispatchLiteral {
	value: string;
	/** True when the firmware compares a prefix (strncmp) rather than the whole string. */
	prefix: boolean;
}

function walk(dir: string, out: string[]): string[] {
	if (!fs.existsSync(dir)) return out;
	for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
		const entryPath = path.join(dir, entry.name);
		if (entry.isDirectory()) walk(entryPath, out);
		else if (/\.(c|cc|cpp|h)$/.test(entry.name)) out.push(entryPath);
	}
	return out;
}

/** Collect command string literals passed as the second arg of strcmp/strncmp. */
function collectDispatchLiterals(): DispatchLiteral[] {
	const files = [
		...walk(path.join(repoRoot, "firmware", "lib"), []),
		...walk(path.join(repoRoot, "firmware", "src"), []),
	];
	const literals = new Map<string, boolean>();
	const compareRe = /\bstrn?cmp\s*\([^,;]*?,\s*"([^"]+)"(?:\s*,\s*(\d+))?\s*\)/g;
	for (const file of files) {
		const src: string = fs.readFileSync(file, "utf-8");
		let match: RegExpExecArray | null;
		while ((match = compareRe.exec(src)) !== null) {
			const value = match[1];
			const explicitLen = match[2] ? Number.parseInt(match[2], 10) : undefined;
			const prefix = value.endsWith(":") || explicitLen === value.length;
			literals.set(value, (literals.get(value) ?? false) || prefix);
		}
	}
	return [...literals].map(([value, prefix]) => ({ value, prefix }));
}

const dispatchLiterals = collectDispatchLiterals();

function isDispatched(command: string): boolean {
	return dispatchLiterals.some(
		(lit) => lit.value === command || (lit.prefix && command.startsWith(lit.value)),
	);
}

/** Bare commands (no ':') that are valid wire commands. */
const BARE_COMMANDS = new Set([
	"ping",
	"status",
	"summon",
	"bms",
	"lock",
	"unlock",
	"horn",
	"glovebox",
	"chargeport",
	"frunk",
	"tpms",
	"powertrain",
	"fwcompat",
	"vehicle",
	"platform",
	"log",
]);

interface DocRow {
	command: string;
	description: string;
	line: number;
}

/** Extract `command` tokens from the first column of docs markdown tables. */
function docCommandRows(): DocRow[] {
	const rows: DocRow[] = [];
	docsContent.split(/\r?\n/).forEach((line, index) => {
		if (!line.trimStart().startsWith("|")) return;
		const cells = line.split("|");
		const match = /`([^`]+)`/.exec(cells[1] ?? "");
		if (!match) return;
		const command = match[1].trim();
		// Skip placeholders, ranges, and field names.
		if (command.includes("<") || command.includes(">")) return;
		if (command.includes("–") || command.includes("...") || command.includes("..")) return;
		if (/(^|:)N$/.test(command)) return;
		if (!command.includes(":") && !BARE_COMMANDS.has(command)) return;
		rows.push({
			command,
			description: `${cells[2] ?? ""} ${cells[3] ?? ""}`,
			line: index + 1,
		});
	});
	return rows;
}

describe("command dispatch coverage (repo grep guard)", () => {
	it("firmware sources contain strcmp/strncmp dispatch literals", () => {
		expect(dispatchLiterals.length).toBeGreaterThan(100);
	});

	it("every documented command has a dispatch branch or an UNSUPPORTED marker", () => {
		const offenders = docCommandRows()
			.filter((row) => !isDispatched(row.command))
			.filter((row) => !/UNSUPPORTED/i.test(row.description))
			.map((row) => `${row.command} (commands.md:${row.line})`);
		expect(offenders).toEqual([]);
	});

	it("does not let a dispatched command hide behind an UNSUPPORTED marker", () => {
		const stale = docCommandRows()
			.filter((row) => isDispatched(row.command))
			.filter((row) => /UNSUPPORTED/i.test(row.description))
			.map((row) => `${row.command} (commands.md:${row.line})`);
		expect(stale).toEqual([]);
	});
});

describe("canonical docs must not contain stale alias wire tokens in prose", () => {
	// C1-10 aliased the old `apfirst:*` / `lanegraph:*` spellings to the firmware
	// names `ap-first:*` / `lane-graph:*`. docCommandRows() only scans table first
	// columns, so a stale alias left in prose would escape that guard. Scan prose
	// backtick tokens across canonical docs (generated bundle is not a source).
	const DEPRECATED_ALIASES = ["apfirst", "lanegraph"];
	const CANONICAL_DOC_DIRS = ["docs", "docs/guides", "docs/reference", "docs/architecture"];

	function canonicalDocs(): string[] {
		const files: string[] = [];
		for (const rel of CANONICAL_DOC_DIRS) {
			const dir = path.join(repoRoot, rel);
			if (!fs.existsSync(dir)) continue;
			for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
				const full = path.join(dir, entry.name);
				if (entry.isFile() && entry.name.endsWith(".md")) files.push(full);
			}
		}
		return files;
	}

	function staleProseAliases(): string[] {
		const offenders: string[] = [];
		for (const file of canonicalDocs()) {
			const rel = path.relative(repoRoot, file).replace(/\\/g, "/");
			fs.readFileSync(file, "utf-8")
				.split(/\r?\n/)
				.forEach((line: string, index: number) => {
					if (line.trimStart().startsWith("|")) return; // table rows handled by docCommandRows()
					for (const token of line.match(/`[^`]+`/g) ?? []) {
						const value = token.slice(1, -1);
						if (
							DEPRECATED_ALIASES.some((alias) => new RegExp(`^${alias}:`).test(value))
						) {
							offenders.push(`${value} (${rel}:${index + 1})`);
						}
					}
				});
		}
		return offenders;
	}

	it("finds canonical docs to scan", () => {
		expect(canonicalDocs().length).toBeGreaterThan(0);
	});

	it("no deprecated alias wire token remains in canonical prose", () => {
		expect(staleProseAliases()).toEqual([]);
	});
});

describe("client must not emit UNSUPPORTED wire commands", () => {
	// UNSUPPORTED rows in commands.md have no firmware dispatcher branch and the
	// device answers `Unknown command`. The client must never put them on the wire.
	const unsupportedPrefixes = Array.from(
		new Set(
			docCommandRows()
				.filter((row) => /UNSUPPORTED/i.test(row.description))
				.map((row) => row.command.split(":")[0]),
		),
	);

	function clientSourceFiles(dir: string, out: string[] = []): string[] {
		if (!fs.existsSync(dir)) return out;
		for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
			const full = path.join(dir, entry.name);
			if (entry.isDirectory()) {
				if (entry.name !== "docs") clientSourceFiles(full, out); // generated prose bundle
			} else if (/\.(ts|tsx|js|mjs)$/.test(entry.name)) {
				out.push(full);
			}
		}
		return out;
	}

	it("derives UNSUPPORTED prefixes from commands.md", () => {
		expect(unsupportedPrefixes.length).toBeGreaterThan(0);
	});

	it("no client source string literal starts with an UNSUPPORTED prefix", () => {
		const offenders: string[] = [];
		const re = new RegExp(`["'\`](?:${unsupportedPrefixes.join("|")}):`, "g");
		for (const file of clientSourceFiles(path.join(repoRoot, "client", "src"))) {
			const rel = path.relative(repoRoot, file).replace(/\\/g, "/");
			const src = fs.readFileSync(file, "utf-8");
			src.split(/\r?\n/).forEach((line: string, index: number) => {
				if (re.test(line)) offenders.push(`${rel}:${index + 1}`);
				re.lastIndex = 0;
			});
		}
		expect(offenders).toEqual([]);
	});
});

describe("AP-active gate uses one time base in the DAS-status handler", () => {
	// The 0x39B handler stamps apGateApActiveMs and evaluates the ALC gate in the
	// same breath. Both must use the handler-local `now`; mixing in the loop-cached
	// gateNowMs can read the gate closed via unsigned underflow.
	const vehicleHandler: string = fs.readFileSync(
		path.join(repoRoot, "firmware", "lib", "vehicle", "can", "handler", "bus", "vehicle.h"),
		"utf-8",
	);

	it("stamps apGateApActiveMs and checks apGateOpen from the same `now`", () => {
		expect(vehicleHandler).toContain("s.apGateApActiveMs = now;");
		expect(vehicleHandler).toContain("s.apGateOpen(now)");
	});
});

describe("fresh-boot commands must work without 0x273 CAN traffic", () => {
	// C1-07a replaced the bare `hasCtrl` early return with the single freshness
	// gate `hasFreshCtrl(now)` (lastCtrlMs + AP_GATE_FRESHNESS_MS). The guard below
	// still enforces the same rule: diagnostics needing no vehicle CAN traffic must
	// be dispatched before this gate so they work without a fresh 0x273 frame.
	//
	// C1-19: primary guard is now the native test test_native_cmd_freshness
	// (firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp), which really
	// calls executeCommand with hasCtrl=false and asserts log/platform/recorder JSON.
	// The source-order assertions below are kept as a secondary check.
	const hasFreshCtrlIndex = dispatchSource.indexOf("if (!s.hasFreshCtrl(now))");

	it("locates the hasFreshCtrl early return in dispatch.h", () => {
		expect(hasFreshCtrlIndex).toBeGreaterThan(-1);
	});

	it.each([
		['strcmp(cmd, "log")', "log"],
		['strcmp(cmd, "platform")', "platform"],
		['strncmp(cmd, "recorder:",', "recorder:*"],
	])("dispatches %s before the hasFreshCtrl gate", (marker, label) => {
		const index = dispatchSource.indexOf(marker);
		expect({ label, branchFound: index > -1 }).toEqual({ label, branchFound: true });
		if (index > -1) {
			expect({ label, beforeFreshCtrlGate: index < hasFreshCtrlIndex }).toEqual({
				label,
				beforeFreshCtrlGate: true,
			});
		}
	});
});
