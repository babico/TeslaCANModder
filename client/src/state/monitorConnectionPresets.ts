/**
 * Pure helpers for managing monitor transport presets and connection workflows.
 * Encapsulates logic for applying connection configs and managing preset state.
 */

import type { HardwareConnectionConfig } from "../types/controls";

/**
 * Represents a named connection preset that can be quickly applied.
 */
export interface Preset {
	name: string;
	connection: HardwareConnectionConfig;
}

/**
 * Input for building preset from a selection trigger.
 * Used to construct the connection config when a preset is applied.
 */
export interface ApplyPresetInput {
	preset: Preset;
}

/**
 * Result of applying a connection preset.
 * Contains the config to connect with and a confirmation message.
 */
export interface ApplyPresetResult {
	config: HardwareConnectionConfig;
	transportType: "http";
	confirmationMessage: string;
}

/**
 * Applies a preset connection configuration.
 * Returns the config and confirmation message for UI display.
 */
export function applyPreset(input: ApplyPresetInput): ApplyPresetResult {
	return {
		config: input.preset.connection,
		transportType: "http",
		confirmationMessage: `Preset applied: ${input.preset.name}`,
	};
}

export interface ConnectionState {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
}

export interface UpdateConnectionStateInput {
	current: ConnectionState;
	updates: Partial<HardwareConnectionConfig>;
}

/**
 * Updates connection state with new configuration values.
 * Returns new connection state and confirmation message.
 */
export function updateConnectionState(input: UpdateConnectionStateInput): ConnectionState {
	const { current, updates } = input;
	return {
		baseUrl: updates.baseUrl ?? current.baseUrl,
		commandPath: updates.commandPath ?? current.commandPath,
		statusPath: updates.statusPath ?? current.statusPath,
	};
}

export interface ConnectionValidationInput {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
}

/**
 * Validates connection configuration for required fields.
 * Returns true if all required fields are present and non-empty.
 */
export function validateConnectionConfig(input: ConnectionValidationInput): boolean {
	return (
		input.baseUrl.trim().length > 0 &&
		input.commandPath.trim().length > 0 &&
		input.statusPath.trim().length > 0
	);
}

export interface BuildApplyConnectionMessageInput {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
}

/**
 * Builds a user-facing message for connection application.
 * Indicates transport type and connection details.
 */
export function buildApplyConnectionMessage(input: BuildApplyConnectionMessageInput): string {
	return `Applied HTTP connection: ${input.baseUrl}${input.commandPath}`;
}

/**
 * Default/fallback preset for quick access to vehicle AP.
 */
export const DEFAULT_PRESET: Preset = {
	name: "Vehicle AP",
	connection: {
		baseUrl: "http://192.168.4.1",
		commandPath: "/api/command",
		statusPath: "/api/status",
	},
};

/**
 * Common lab development presets for testing.
 */
export const LAB_PRESETS: Preset[] = [
	DEFAULT_PRESET,
	{
		name: "Local Bridge",
		connection: {
			baseUrl: "http://localhost:8080",
			commandPath: "/api/command",
			statusPath: "/api/status",
		},
	},
	{
		name: "Lab Rig",
		connection: {
			baseUrl: "http://192.168.10.20",
			commandPath: "/api/command",
			statusPath: "/api/status",
		},
	},
];
