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

describe("fresh-boot commands must work without 0x273 CAN traffic", () => {
	const hasCtrlIndex = dispatchSource.indexOf("if (!s.hasCtrl)");

	it("locates the hasCtrl early return in dispatch.h", () => {
		expect(hasCtrlIndex).toBeGreaterThan(-1);
	});

	it.each([
		['strcmp(cmd, "log")', "log"],
		['strcmp(cmd, "platform")', "platform"],
		['strncmp(cmd, "recorder:",', "recorder:*"],
	])("dispatches %s before the hasCtrl gate", (marker, label) => {
		const index = dispatchSource.indexOf(marker);
		expect({ label, branchFound: index > -1 }).toEqual({ label, branchFound: true });
		if (index > -1) {
			expect({ label, beforeHasCtrl: index < hasCtrlIndex }).toEqual({
				label,
				beforeHasCtrl: true,
			});
		}
	});
});
