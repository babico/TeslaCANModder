import type { CandumpProcess, CandumpProcessFactory } from "../../src/can/candump.js";

/** One scripted step emitted by a fake `candump` process. */
export type CandumpStep = { kind: "line"; line: string } | { kind: "error"; error: Error };

export interface FakeCandump {
	factory: CandumpProcessFactory;
	calls: () => Array<[string, readonly string[]]>;
	killCount: () => number;
}

/**
 * Injectable `candump` process factory. Each factory call consumes the next
 * script; the fake stdout yields those lines then ends cleanly. No real child
 * process or CAN interface is touched.
 */
export function fakeCandump(scriptPerOpen: CandumpStep[][]): FakeCandump {
	const calls: Array<[string, readonly string[]]> = [];
	let kills = 0;

	const factory: CandumpProcessFactory = (command, args) => {
		calls.push([command, args]);
		const steps = scriptPerOpen[calls.length - 1] ?? [];
		const stdout: AsyncIterable<string> = {
			async *[Symbol.asyncIterator](): AsyncGenerator<string> {
				for (const step of steps) {
					if (step.kind === "error") throw step.error;
					yield `${step.line}\n`; // candump stdout is newline-delimited
				}
			},
		};
		const process: CandumpProcess = {
			stdout,
			kill: () => {
				kills += 1;
			},
		};
		return process;
	};

	return { factory, calls: () => calls, killCount: () => kills };
}
