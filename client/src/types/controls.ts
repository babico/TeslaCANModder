import type { BoardState, commands } from "@teslacanmodder/protocol";

export type ProtocolCommands = typeof commands;

export interface CommandExecutionResult {
	ok: boolean;
	command: string;
	responseText?: string;
	responseData?: unknown;
	boardState?: BoardState;
	error?: string;
}

export interface HardwareStatus {
	raw: unknown;
	fetchedAt: number;
	boardState?: BoardState;
}

export interface HardwareConnectionConfig {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
}
