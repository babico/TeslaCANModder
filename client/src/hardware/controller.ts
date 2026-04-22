import { commands } from "@teslacanmodder/protocol";

import type {
	CommandExecutionResult,
	HardwareConnectionConfig,
	HardwareStatus,
} from "../types/controls";
import {
	BleCommandTransport,
	BluetoothSerialCommandTransport,
	CommandTransport,
	HttpCommandTransport,
	SerialCommandTransport,
	TransportType,
	UnsupportedTransport,
	createTransport,
	detectAvailableTransportType,
	supportsTransport,
	type BleTransportConfig,
	type BluetoothSerialTransportConfig,
	type IBlePeripheral,
	type IBluetoothSerialPort,
	type ISerialPort,
	type SerialTransportConfig,
	type TransportFactoryOptions,
} from "./transport";
import { coerceBoardStateSnapshot } from "../state/board";

export type CommandName = keyof typeof commands;

export interface CommandDescriptor {
	name: CommandName;
	argCount: number;
}

export const ALL_COMMANDS: CommandDescriptor[] = Object.entries(commands)
	.map(([name, fn]) => ({
		name: name as CommandName,
		argCount: fn.length,
	}))
	.sort((a, b) => a.name.localeCompare(b.name));

export const DEFAULT_CONNECTION: HardwareConnectionConfig = {
	baseUrl: "http://192.168.4.1",
	commandPath: "/api/command",
	statusPath: "/api/status",
};

function parseRawArg(raw: string): unknown {
	const trimmed = raw.trim();
	if (trimmed.length === 0) {
		return "";
	}

	if (trimmed === "true") return true;
	if (trimmed === "false") return false;
	if (/^-?\d+$/.test(trimmed)) return Number.parseInt(trimmed, 10);
	if (/^-?\d+\.\d+$/.test(trimmed)) return Number.parseFloat(trimmed);

	return trimmed;
}

function parseArgs(input: string, expectedCount: number): unknown[] {
	const trimmed = input.trim();
	if (expectedCount === 0) {
		return [];
	}

	if (trimmed.length === 0) {
		throw new Error(`this command needs ${expectedCount} argument(s)`);
	}

	try {
		const parsed = JSON.parse(trimmed);
		if (Array.isArray(parsed)) {
			if (parsed.length !== expectedCount) {
				throw new Error(`expected ${expectedCount} argument(s), got ${parsed.length}`);
			}
			return parsed;
		}
	} catch {
		// fall through to comma parser
	}

	const parsedParts = trimmed.split(",").map((part) => parseRawArg(part));
	if (parsedParts.length !== expectedCount) {
		throw new Error(`expected ${expectedCount} argument(s), got ${parsedParts.length}`);
	}

	return parsedParts;
}

export class HardwareController {
	private transport: CommandTransport;
	private inflight: Promise<unknown> | null = null;

	constructor(config: HardwareConnectionConfig = DEFAULT_CONNECTION) {
		this.transport = new HttpCommandTransport(config);
	}

	/**
	 * Replace the active transport with any `CommandTransport` implementation.
	 * Waits for any in-flight request to settle before swapping.
	 */
	async setTransport(transport: CommandTransport): Promise<void> {
		if (this.inflight) {
			try {
				await this.inflight;
			} catch {
				/* swallow */
			}
		}
		this.transport = transport;
	}

	/** Convenience: re-configure HTTP transport (preserves existing behavior). */
	setConnection(config: HardwareConnectionConfig): void {
		this.transport = new HttpCommandTransport(config);
	}

	/** Explicit REST API connection path. */
	connectViaRestApi(config: HardwareConnectionConfig): void {
		this.transport = new HttpCommandTransport(config);
	}

	/** Explicit BLE connection path. */
	connectViaBle(peripheral: IBlePeripheral, config?: BleTransportConfig): void {
		this.transport = new BleCommandTransport(peripheral, config);
	}

	/** Explicit COM port path (USB serial / virtual serial). */
	connectViaComPort(port: ISerialPort, config?: SerialTransportConfig): void {
		this.transport = new SerialCommandTransport(port, config);
	}

	/** Explicit Bluetooth SPP/RFCOMM COM port path. */
	connectViaBluetoothComPort(
		port: IBluetoothSerialPort,
		config?: BluetoothSerialTransportConfig,
	): void {
		this.transport = new BluetoothSerialCommandTransport(port, config);
	}

	/**
	 * Build and set the best available transport for the current runtime.
	 * Prefers HTTP (always available when a base URL is known). Pass
	 * `preferredType` to override the auto-detected order.
	 */
	setTransportFromFactory(options: TransportFactoryOptions): void {
		this.transport = createTransport(options);
	}

	/** Returns the type of the currently active transport. */
	get activeTransportType(): TransportType {
		return this.transport.transportType;
	}

	/** Returns the best available transport type for the current runtime. */
	static detectTransportType(): TransportType {
		return detectAvailableTransportType();
	}

	/** Returns true if the given transport type can be used in this runtime. */
	static supportsTransport(type: TransportType): boolean {
		return supportsTransport(type);
	}

	async disconnectTransport(): Promise<void> {
		if (this.inflight) {
			try {
				await this.inflight;
			} catch {
				// ignore request failures while tearing down the active transport
			} finally {
				this.inflight = null;
			}
		}

		try {
			await this.transport.close?.();
		} finally {
			this.transport = new UnsupportedTransport();
		}
	}

	async runCommand(name: CommandName, rawArgs = ""): Promise<CommandExecutionResult> {
		const descriptor = ALL_COMMANDS.find((entry) => entry.name === name);
		if (!descriptor) {
			return { ok: false, command: name, error: "unknown command" };
		}

		const fn = commands[name] as (...args: unknown[]) => string;

		try {
			const args = parseArgs(rawArgs, descriptor.argCount);
			const builtCommand = fn(...args);
			const sendPromise = this.transport.send(builtCommand);
			this.inflight = sendPromise;
			const response = await sendPromise;
			this.inflight = null;
			const responseText =
				typeof response === "string" ? response : JSON.stringify(response, null, 2);

			return {
				ok: true,
				command: builtCommand,
				responseText,
				responseData: response,
				boardState: coerceBoardStateSnapshot(response) ?? undefined,
			};
		} catch (error) {
			this.inflight = null;
			return {
				ok: false,
				command: name,
				error: error instanceof Error ? error.message : "unknown failure",
			};
		}
	}

	async runRawCommand(command: string): Promise<CommandExecutionResult> {
		const trimmed = command.trim();
		if (!trimmed) {
			return { ok: false, command: "", error: "command cannot be empty" };
		}

		try {
			const sendPromise = this.transport.send(trimmed);
			this.inflight = sendPromise;
			const response = await sendPromise;
			this.inflight = null;
			const responseText =
				typeof response === "string" ? response : JSON.stringify(response, null, 2);

			return {
				ok: true,
				command: trimmed,
				responseText,
				responseData: response,
				boardState: coerceBoardStateSnapshot(response) ?? undefined,
			};
		} catch (error) {
			this.inflight = null;
			return {
				ok: false,
				command: trimmed,
				error: error instanceof Error ? error.message : "unknown failure",
			};
		}
	}

	async readStatus(): Promise<HardwareStatus> {
		const raw = await this.transport.status();
		return {
			raw,
			fetchedAt: Date.now(),
			boardState: coerceBoardStateSnapshot(raw) ?? undefined,
		};
	}
}
