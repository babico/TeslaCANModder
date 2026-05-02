/**
 * B-04 Command Bus unit tests
 * Covers: reducer lifecycle transitions, selectors, purge, counters, COMMAND_BUS_MAX_ENTRIES cap.
 */

import {
	commandBusReducer,
	initialCommandBusState,
	selectPendingCommands,
	selectSettledCommands,
	selectLastCommand,
	selectSuccessRate,
	generateCommandId,
	COMMAND_BUS_MAX_ENTRIES,
	type CommandBusState,
	type CommandBusAction,
} from "../../src/state/commandBus";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function dispatch(state: CommandBusState, ...actions: CommandBusAction[]): CommandBusState {
	return actions.reduce(commandBusReducer, state);
}

const T = 1000; // base timestamp

// ---------------------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------------------

describe("initialCommandBusState", () => {
	it("starts empty with zero counters", () => {
		const s = initialCommandBusState();
		expect(s.entries).toHaveLength(0);
		expect(s.totalDispatched).toBe(0);
		expect(s.totalAcked).toBe(0);
		expect(s.totalFailed).toBe(0);
	});
});

// ---------------------------------------------------------------------------
// COMMAND_PENDING
// ---------------------------------------------------------------------------

describe("COMMAND_PENDING", () => {
	it("adds entry with status=pending and increments totalDispatched", () => {
		const s = dispatch(initialCommandBusState(), {
			type: "COMMAND_PENDING",
			payload: { id: "1", command: "fsdOn", startedAt: T },
		});
		expect(s.entries).toHaveLength(1);
		expect(s.entries[0]).toMatchObject({
			id: "1",
			command: "fsdOn",
			status: "pending",
			startedAt: T,
		});
		expect(s.totalDispatched).toBe(1);
	});

	it("prepends newest entry first (newest-first order)", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "b", startedAt: T + 1 } },
		);
		expect(s.entries[0].id).toBe("2");
		expect(s.entries[1].id).toBe("1");
	});

	it("trims entries to COMMAND_BUS_MAX_ENTRIES", () => {
		let s = initialCommandBusState();
		for (let i = 0; i < COMMAND_BUS_MAX_ENTRIES + 5; i++) {
			s = commandBusReducer(s, {
				type: "COMMAND_PENDING",
				payload: { id: String(i), command: `cmd${i}`, startedAt: T + i },
			});
		}
		expect(s.entries.length).toBe(COMMAND_BUS_MAX_ENTRIES);
		expect(s.totalDispatched).toBe(COMMAND_BUS_MAX_ENTRIES + 5);
	});
});

// ---------------------------------------------------------------------------
// COMMAND_ACKED
// ---------------------------------------------------------------------------

describe("COMMAND_ACKED", () => {
	it("transitions pending entry to acked with finishedAt and detail", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "fsdOn", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 100, detail: "ok" } },
		);
		const entry = s.entries.find((e) => e.id === "1")!;
		expect(entry.status).toBe("acked");
		expect(entry.finishedAt).toBe(T + 100);
		expect(entry.detail).toBe("ok");
		expect(s.totalAcked).toBe(1);
	});

	it("updates command string if provided", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "raw", startedAt: T } },
			{
				type: "COMMAND_ACKED",
				payload: { id: "1", command: "built_cmd", finishedAt: T + 50 },
			},
		);
		expect(s.entries[0].command).toBe("built_cmd");
	});

	it("does not affect other entries", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "b", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 10 } },
		);
		const other = s.entries.find((e) => e.id === "2")!;
		expect(other.status).toBe("pending");
	});

	it("ignores ack for unknown id (no error, no state mutation)", () => {
		const s = dispatch(initialCommandBusState(), {
			type: "COMMAND_ACKED",
			payload: { id: "unknown", finishedAt: T },
		});
		expect(s.entries).toHaveLength(0);
		expect(s.totalAcked).toBe(1); // counter still increments (idempotent)
	});
});

// ---------------------------------------------------------------------------
// COMMAND_FAILED
// ---------------------------------------------------------------------------

describe("COMMAND_FAILED", () => {
	it("transitions pending entry to failed", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "fsdOn", startedAt: T } },
			{
				type: "COMMAND_FAILED",
				payload: { id: "1", finishedAt: T + 200, detail: "timeout" },
			},
		);
		const entry = s.entries[0];
		expect(entry.status).toBe("failed");
		expect(entry.detail).toBe("timeout");
		expect(s.totalFailed).toBe(1);
	});
});

// ---------------------------------------------------------------------------
// PURGE_SETTLED
// ---------------------------------------------------------------------------

describe("PURGE_SETTLED", () => {
	it("with no payload — removes all settled entries, keeps pending", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "pending", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "done", command: "b", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "done", finishedAt: T + 10 } },
			{ type: "PURGE_SETTLED" },
		);
		expect(s.entries).toHaveLength(1);
		expect(s.entries[0].id).toBe("pending");
	});

	it("with olderThanMs — keeps recently settled entries", () => {
		const now = Date.now();
		const s1 = dispatch(
			initialCommandBusState(),
			{
				type: "COMMAND_PENDING",
				payload: { id: "recent", command: "a", startedAt: now - 100 },
			},
			{
				type: "COMMAND_PENDING",
				payload: { id: "old", command: "b", startedAt: now - 60_000 },
			},
			{ type: "COMMAND_ACKED", payload: { id: "recent", finishedAt: now - 50 } },
			{ type: "COMMAND_ACKED", payload: { id: "old", finishedAt: now - 59_000 } },
			// purge settled older than 1000 ms
			{ type: "PURGE_SETTLED", payload: { olderThanMs: 1000 } },
		);
		expect(s1.entries.find((e) => e.id === "recent")).toBeDefined();
		expect(s1.entries.find((e) => e.id === "old")).toBeUndefined();
	});
});

// ---------------------------------------------------------------------------
// Selectors
// ---------------------------------------------------------------------------

describe("selectPendingCommands", () => {
	it("returns only pending entries", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "b", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 10 } },
		);
		const pending = selectPendingCommands(s);
		expect(pending).toHaveLength(1);
		expect(pending[0].id).toBe("2");
	});
});

describe("selectSettledCommands", () => {
	it("returns acked and failed entries", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "b", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "3", command: "c", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 10 } },
			{ type: "COMMAND_FAILED", payload: { id: "2", finishedAt: T + 20 } },
		);
		const settled = selectSettledCommands(s);
		expect(settled).toHaveLength(2);
	});
});

describe("selectLastCommand", () => {
	it("returns the most recently dispatched entry", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "first", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "last", startedAt: T + 1 } },
		);
		expect(selectLastCommand(s)?.id).toBe("2");
	});

	it("returns undefined on empty state", () => {
		expect(selectLastCommand(initialCommandBusState())).toBeUndefined();
	});
});

describe("selectSuccessRate", () => {
	it("returns null when no commands have settled", () => {
		const s = dispatch(initialCommandBusState(), {
			type: "COMMAND_PENDING",
			payload: { id: "1", command: "a", startedAt: T },
		});
		expect(selectSuccessRate(s)).toBeNull();
	});

	it("returns 1 when all commands acked", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 10 } },
		);
		expect(selectSuccessRate(s)).toBe(1);
	});

	it("returns 0 when all commands failed", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_FAILED", payload: { id: "1", finishedAt: T + 10 } },
		);
		expect(selectSuccessRate(s)).toBe(0);
	});

	it("returns 0.5 for 1 acked 1 failed", () => {
		const s = dispatch(
			initialCommandBusState(),
			{ type: "COMMAND_PENDING", payload: { id: "1", command: "a", startedAt: T } },
			{ type: "COMMAND_PENDING", payload: { id: "2", command: "b", startedAt: T } },
			{ type: "COMMAND_ACKED", payload: { id: "1", finishedAt: T + 10 } },
			{ type: "COMMAND_FAILED", payload: { id: "2", finishedAt: T + 20 } },
		);
		expect(selectSuccessRate(s)).toBe(0.5);
	});
});

// ---------------------------------------------------------------------------
// generateCommandId
// ---------------------------------------------------------------------------

describe("generateCommandId", () => {
	it("generates unique IDs on successive calls", () => {
		const ids = new Set(Array.from({ length: 50 }, () => generateCommandId()));
		expect(ids.size).toBe(50);
	});

	it("generated ID is a non-empty string", () => {
		expect(typeof generateCommandId()).toBe("string");
		expect(generateCommandId().length).toBeGreaterThan(0);
	});
});
