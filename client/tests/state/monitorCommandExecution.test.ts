import {
	buildCommandExecutionResult,
	buildQuickActionResult,
	resolveCommandExecutionReadiness,
} from "../../src/state/monitorCommandExecution";

describe("resolveCommandExecutionReadiness", () => {
	it("allows execution when all gates are open", () => {
		const result = resolveCommandExecutionReadiness({
			transportCanExecute: true,
			commandAvailable: true,
		});
		expect(result.canExecute).toBe(true);
		expect(result.blockReason).toBeUndefined();
	});

	it("blocks with transport reason if transport cannot execute", () => {
		const result = resolveCommandExecutionReadiness({
			transportCanExecute: false,
			transportBlockReason: "HTTP transport not ready",
			commandAvailable: true,
		});
		expect(result.canExecute).toBe(false);
		expect(result.blockReason).toBe("HTTP transport not ready");
	});

	it("blocks with command reason if command unavailable (and transport ok)", () => {
		const result = resolveCommandExecutionReadiness({
			transportCanExecute: true,
			commandAvailable: false,
			commandBlockReason: "Command disabled for this variant",
		});
		expect(result.canExecute).toBe(false);
		expect(result.blockReason).toBe("Command disabled for this variant");
	});

	it("prioritizes transport block reason over command block reason", () => {
		const result = resolveCommandExecutionReadiness({
			transportCanExecute: false,
			transportBlockReason: "BLE transport disconnected",
			commandAvailable: false,
			commandBlockReason: "Command disabled",
		});
		expect(result.canExecute).toBe(false);
		expect(result.blockReason).toBe("BLE transport disconnected");
	});
});

describe("buildCommandExecutionResult", () => {
	const baseInput = {
		commandName: "ping" as const,
		rawArgs: "",
		lifecycleId: "cmd-123",
		startedAt: 1000,
	};

	describe("when command cannot execute", () => {
		it("returns blocked state with no dispatch actions", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: false,
				blockReason: "Transport not ready",
				controllerResponse: { ok: true },
			});

			expect(result.canExecute).toBe(false);
			expect(result.blockReason).toBe("Transport not ready");
			expect(result.dispatchActions).toHaveLength(0);
			expect(result.displayMessage).toBe("Transport not ready");
			expect(result.historyEntry).toBeUndefined();
			expect(result.shouldApplyBoardPayload).toBe(false);
		});

		it("handles undefined blockReason gracefully", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: false,
				blockReason: undefined,
				controllerResponse: { ok: true },
			});

			expect(result.blockReason).toBeUndefined();
			expect(result.displayMessage).toBe("Command blocked");
		});
	});

	describe("when command execution fails", () => {
		it("creates pending and failed dispatch actions", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: false,
					error: "Device not responding",
				},
			});

			expect(result.canExecute).toBe(true);
			expect(result.dispatchActions).toHaveLength(2);
			expect(result.dispatchActions[0].type).toBe("COMMAND_PENDING");
			expect(result.dispatchActions[1].type).toBe("COMMAND_FAILED");

			const failedAction = result.dispatchActions[1];
			expect(failedAction.type).toBe("COMMAND_FAILED");
			if (failedAction.type === "COMMAND_FAILED") {
				expect(failedAction.payload.detail).toBe("Device not responding");
			}
		});

		it("includes command with args in pending action", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				rawArgs: "args123",
				canExecute: true,
				controllerResponse: {
					ok: false,
					error: "timeout",
				},
			});

			const pendingAction = result.dispatchActions[0];
			expect(pendingAction.type).toBe("COMMAND_PENDING");
			if (pendingAction.type === "COMMAND_PENDING") {
				expect(pendingAction.payload.command).toBe("ping args123");
			}
		});

		it("creates history entry marking command as failed", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: false,
					error: "connection lost",
				},
			});

			expect(result.historyEntry).toBeDefined();
			expect(result.historyEntry?.ok).toBe(false);
			expect(result.historyEntry?.response).toBe("Error: connection lost");
		});

		it("does not apply board payload on failure", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: false,
					error: "failed",
				},
			});

			expect(result.shouldApplyBoardPayload).toBe(false);
		});

		it("handles missing error string gracefully", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: false,
				},
			});

			expect(result.displayMessage).toBe("Error: unknown error");
		});
	});

	describe("when command succeeds", () => {
		it("creates pending and acked dispatch actions", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "unlock_doors",
					responseText: "Doors unlocked",
				},
			});

			expect(result.dispatchActions).toHaveLength(2);
			expect(result.dispatchActions[0].type).toBe("COMMAND_PENDING");
			expect(result.dispatchActions[1].type).toBe("COMMAND_ACKED");
		});

		it("truncates response detail to 120 characters", () => {
			const longResponse = "x".repeat(200);
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "test_cmd",
					responseText: longResponse,
				},
			});

			const ackedAction = result.dispatchActions[1];
			expect(ackedAction.type).toBe("COMMAND_ACKED");
			if (ackedAction.type === "COMMAND_ACKED") {
				expect(ackedAction.payload.detail).toHaveLength(120);
			}
		});

		it("applies board payload when responseData is present", () => {
			const boardData = { soc: 95 };
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "read_status",
					responseText: "Status fetched",
					responseData: boardData,
				},
			});

			expect(result.shouldApplyBoardPayload).toBe(true);
		});

		it("does not apply board payload when responseData is undefined", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "unlock_doors",
					responseText: "OK",
				},
			});

			expect(result.shouldApplyBoardPayload).toBe(false);
		});

		it("creates success history entry", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "unlock_doors",
					responseText: "Door unlocked",
				},
			});

			expect(result.historyEntry?.ok).toBe(true);
			expect(result.historyEntry?.response).toBe("Door unlocked");
		});

		it("handles missing responseText with placeholder", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "test_cmd",
				},
			});

			expect(result.displayMessage).toContain("(no response)");
		});

		it("uses controller responseText in history", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "status_read",
					responseText: "Status: OK",
				},
			});

			expect(result.historyEntry?.response).toBe("Status: OK");
		});
	});

	describe("command args handling", () => {
		it("includes trimmed args in command pending action", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				rawArgs: "  arg1 arg2  ",
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "ping arg1 arg2",
				},
			});

			const pendingAction = result.dispatchActions[0];
			expect(pendingAction.type).toBe("COMMAND_PENDING");
			if (pendingAction.type === "COMMAND_PENDING") {
				expect(pendingAction.payload.command).toBe("ping arg1 arg2");
			}
		});

		it("omits empty args from command string", () => {
			const result = buildCommandExecutionResult({
				...baseInput,
				rawArgs: "   ",
				canExecute: true,
				controllerResponse: {
					ok: true,
					command: "ping",
				},
			});

			const pendingAction = result.dispatchActions[0];
			expect(pendingAction.type).toBe("COMMAND_PENDING");
			if (pendingAction.type === "COMMAND_PENDING") {
				expect(pendingAction.payload.command).toBe("ping");
			}
		});
	});

	describe("timestamp handling", () => {
		it("uses provided startedAt timestamp", () => {
			const startedAt = 5000;
			const result = buildCommandExecutionResult({
				...baseInput,
				startedAt,
				canExecute: true,
				controllerResponse: { ok: true, command: "test" },
			});

			const pendingAction = result.dispatchActions[0];
			expect(pendingAction.type).toBe("COMMAND_PENDING");
			if (pendingAction.type === "COMMAND_PENDING") {
				expect(pendingAction.payload.startedAt).toBe(5000);
			}
		});

		it("uses current time for finishedAt in acked action", () => {
			const beforeCall = Date.now();
			const result = buildCommandExecutionResult({
				...baseInput,
				canExecute: true,
				controllerResponse: { ok: true, command: "test" },
			});
			const afterCall = Date.now();

			const ackedAction = result.dispatchActions[1];
			expect(ackedAction.type).toBe("COMMAND_ACKED");
			if (ackedAction.type === "COMMAND_ACKED") {
				expect(ackedAction.payload.finishedAt).toBeGreaterThanOrEqual(beforeCall);
				expect(ackedAction.payload.finishedAt).toBeLessThanOrEqual(afterCall + 10);
			}
		});
	});
});

describe("buildQuickActionResult", () => {
	const baseInput = {
		commandName: "ping" as const,
		lifecycleId: "qa-456",
		startedAt: 1000,
	};

	it("delegates to buildCommandExecutionResult with empty rawArgs", () => {
		const result = buildQuickActionResult({
			...baseInput,
			canExecute: true,
			controllerResponse: {
				ok: true,
				command: "ping",
				responseText: "Pong",
			},
		});

		expect(result.canExecute).toBe(true);
		expect(result.dispatchActions).toHaveLength(2);

		const pendingAction = result.dispatchActions[0];
		expect(pendingAction.type).toBe("COMMAND_PENDING");
		if (pendingAction.type === "COMMAND_PENDING") {
			expect(pendingAction.payload.command).toBe("ping");
		}
	});

	it("returns blocked state when quick action cannot execute", () => {
		const result = buildQuickActionResult({
			...baseInput,
			canExecute: false,
			blockReason: "Transport not ready",
			controllerResponse: { ok: true },
		});

		expect(result.canExecute).toBe(false);
		expect(result.blockReason).toBe("Transport not ready");
		expect(result.dispatchActions).toHaveLength(0);
	});
});
