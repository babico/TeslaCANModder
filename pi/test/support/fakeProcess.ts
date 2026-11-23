import type { CandumpProcess, CandumpProcessFactory } from "../../src/can/candump.js";

/** One scripted step emitted by a fake `candump` process. */
export type CandumpStep =
	| { kind: "line"; line: string }
	| { kind: "error"; error: Error }
	/** Simulate the child's `exit` event after its stdout ends. */
	| { kind: "exit"; code?: number | null; signal?: NodeJS.Signals | null };

type FakeExit = { code: number | null; signal: NodeJS.Signals | null };

export interface FakeCandump {
	factory: CandumpProcessFactory;
	calls: () => Array<[string, readonly string[]]>;
	killCount: () => number;
}

/**
 * Injectable `candump` process factory. Each factory call consumes the next
 * script; the fake stdout yields those lines then ends. No real child process
 * or CAN interface is touched. Exit status is set only by an explicit `exit`
 * step so callers choose whether the child appears to have died.
 */
export function fakeCandump(scriptPerOpen: CandumpStep[][]): FakeCandump {
	const calls: Array<[string, readonly string[]]> = [];
	let kills = 0;

	const factory: CandumpProcessFactory = (command, args) => {
		calls.push([command, args]);
		const steps = scriptPerOpen[calls.length - 1] ?? [];
		let exitStatus: FakeExit | undefined;
		const stdout: AsyncIterable<string> = {
			async *[Symbol.asyncIterator](): AsyncGenerator<string> {
				for (const step of steps) {
					if (step.kind === "error") throw step.error;
					if (step.kind === "exit") {
						exitStatus = { code: step.code ?? 0, signal: step.signal ?? null };
						continue;
					}
					yield `${step.line}\n`; // candump stdout is newline-delimited
				}
			},
		};
		const process: CandumpProcess & { exit?: () => FakeExit | undefined } = {
			stdout,
			kill: () => {
				kills += 1;
			},
			exit: () => exitStatus,
		};
		return process;
	};

	return { factory, calls: () => calls, killCount: () => kills };
}
