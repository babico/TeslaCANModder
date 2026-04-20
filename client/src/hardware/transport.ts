// ---------------------------------------------------------------------------
// Core transport contract
// ---------------------------------------------------------------------------

export interface CommandTransport {
	/** Send a pre-built command string; resolves with parsed response. */
	send(command: string): Promise<unknown>;
	/** Fetch current board status; resolves with parsed status payload. */
	status(): Promise<unknown>;
	/** Human-readable transport type label, used for diagnostics / UI. */
	readonly transportType: TransportType;
}

// ---------------------------------------------------------------------------
// Transport type discriminant
// ---------------------------------------------------------------------------

export type TransportType = "http" | "ble" | "serial" | "bluetooth-serial" | "unsupported";

// ---------------------------------------------------------------------------
// Capability detection
// ---------------------------------------------------------------------------

/** Returns the best available transport type for the current runtime. */
export function detectAvailableTransportType(): TransportType {
	// Web Serial API (Chrome/Edge 89+, web or Electron)
	if (typeof navigator !== "undefined" && "serial" in navigator) {
		return "serial";
	}
	// React Native environment — BLE is expected to be available via an injected
	// IBlePeripheral adapter; we advertise "ble" availability here and let the
	// caller decide whether a device is in range.
	if (typeof navigator === "undefined") {
		// Non-browser (React Native / Node side)
		return "ble";
	}
	// Fallback: HTTP WiFi bridge is always available when a device URL is known
	return "http";
}

/** Returns true when a given transport type can be instantiated in this runtime. */
export function supportsTransport(type: TransportType): boolean {
	const available = detectAvailableTransportType();
	if (type === "http") return true; // HTTP is always constructable
	if (type === "bluetooth-serial") {
		return typeof navigator === "undefined" || available === "serial";
	}
	return available === type;
}

// ---------------------------------------------------------------------------
// Null/unsupported transport
// ---------------------------------------------------------------------------

/**
 * Sentinel transport used when no real transport is available.
 * All calls throw a descriptive error — callers should guard on
 * `supportsTransport` before constructing.
 */
export class UnsupportedTransport implements CommandTransport {
	readonly transportType: TransportType = "unsupported";

	send(_command: string): Promise<unknown> {
		return Promise.reject(new Error("No transport available: cannot send command"));
	}

	status(): Promise<unknown> {
		return Promise.reject(new Error("No transport available: cannot read status"));
	}
}

// ---------------------------------------------------------------------------
// HTTP transport
// ---------------------------------------------------------------------------

export interface HttpTransportConfig {
	baseUrl: string;
	commandPath: string;
	statusPath: string;
	/** Optional timeout for HTTP operations (default: 5000ms). */
	timeoutMs?: number;
}

function normalizeBaseUrl(url: string): string {
	return url.endsWith("/") ? url.slice(0, -1) : url;
}

async function parseResponseBody(response: Response): Promise<unknown> {
	const contentType = response.headers.get("content-type") || "";
	if (contentType.includes("application/json")) {
		return response.json();
	}

	return response.text();
}

export class HttpCommandTransport implements CommandTransport {
	readonly transportType: TransportType = "http";
	private readonly baseUrl: string;
	private readonly commandPath: string;
	private readonly statusPath: string;
	private readonly timeoutMs: number;

	constructor(config: HttpTransportConfig) {
		this.baseUrl = normalizeBaseUrl(config.baseUrl);
		this.commandPath = config.commandPath;
		this.statusPath = config.statusPath;
		this.timeoutMs = config.timeoutMs ?? 5000;
	}

	private async fetchWithTimeout(path: string, init?: RequestInit): Promise<Response> {
		const controller = new AbortController();
		const timeout = setTimeout(() => controller.abort(), this.timeoutMs);

		try {
			return await fetch(`${this.baseUrl}${path}`, {
				...init,
				signal: controller.signal,
			});
		} catch (error) {
			if (error instanceof Error && error.name === "AbortError") {
				throw new Error(
					`request timed out after ${this.timeoutMs}ms: ${this.baseUrl}${path}`,
				);
			}

			if (error instanceof Error) {
				const message = error.message || "network request failed";
				throw new Error(`network error contacting ${this.baseUrl}${path}: ${message}`);
			}

			throw new Error(`network error contacting ${this.baseUrl}${path}`);
		} finally {
			clearTimeout(timeout);
		}
	}

	async send(command: string): Promise<unknown> {
		const response = await this.fetchWithTimeout(this.commandPath, {
			method: "POST",
			headers: {
				"Content-Type": "application/json",
			},
			body: JSON.stringify({ cmd: command }),
		});

		if (!response.ok) {
			const text = await response.text();
			throw new Error(`command failed (${response.status}): ${text}`);
		}

		return parseResponseBody(response);
	}

	async status(): Promise<unknown> {
		const response = await this.fetchWithTimeout(this.statusPath);
		if (!response.ok) {
			const text = await response.text();
			throw new Error(`status failed (${response.status}): ${text}`);
		}

		return parseResponseBody(response);
	}
}

// ---------------------------------------------------------------------------
// BLE transport
// ---------------------------------------------------------------------------

/**
 * Minimal BLE peripheral contract injected into BleCommandTransport.
 * Implement this using any BLE library (e.g. react-native-ble-plx,
 * expo-bluetooth, or a mock for testing).
 */
export interface IBlePeripheral {
	/**
	 * Write a UTF-8 string to the command characteristic.
	 * Returns the raw response bytes (or string) from the response characteristic.
	 */
	writeAndRead(command: string): Promise<string>;
	/** Read current board status from the notify/status characteristic. */
	readStatus(): Promise<string>;
	/** Whether the peripheral is currently connected. */
	readonly isConnected: boolean;
}

export interface BleTransportConfig {
	/** Optional timeout in ms for each BLE operation (default: 5000). */
	timeoutMs?: number;
}

function parseTextResponse(raw: string): unknown {
	const trimmed = raw.trim();
	try {
		return JSON.parse(trimmed);
	} catch {
		return trimmed;
	}
}

export class BleCommandTransport implements CommandTransport {
	readonly transportType: TransportType = "ble";
	private readonly peripheral: IBlePeripheral;
	private readonly timeoutMs: number;

	constructor(peripheral: IBlePeripheral, config: BleTransportConfig = {}) {
		this.peripheral = peripheral;
		this.timeoutMs = config.timeoutMs ?? 5000;
	}

	private withTimeout<T>(promise: Promise<T>): Promise<T> {
		return new Promise<T>((resolve, reject) => {
			const timer = setTimeout(
				() => reject(new Error("BLE operation timed out")),
				this.timeoutMs,
			);
			promise.then(
				(value) => {
					clearTimeout(timer);
					resolve(value);
				},
				(err) => {
					clearTimeout(timer);
					reject(err);
				},
			);
		});
	}

	async send(command: string): Promise<unknown> {
		if (!this.peripheral.isConnected) {
			throw new Error("BLE peripheral is not connected");
		}
		const raw = await this.withTimeout(this.peripheral.writeAndRead(command));
		return parseTextResponse(raw);
	}

	async status(): Promise<unknown> {
		if (!this.peripheral.isConnected) {
			throw new Error("BLE peripheral is not connected");
		}
		const raw = await this.withTimeout(this.peripheral.readStatus());
		return parseTextResponse(raw);
	}
}

// ---------------------------------------------------------------------------
// Serial transport (Web Serial API / Electron)
// ---------------------------------------------------------------------------

/**
 * Minimal serial port contract injected into SerialCommandTransport.
 * Implement using Web Serial API (`navigator.serial`) or Electron IPC.
 *
 * Callers are responsible for opening the port before passing it here.
 */
export interface ISerialPort {
	/**
	 * Write a line (command + newline) to the serial port and read
	 * the next available line response.
	 */
	writeLine(line: string): Promise<string>;
	/** Whether the port is currently open. */
	readonly isOpen: boolean;
}

export interface SerialTransportConfig {
	/** Delimiter to append to commands (default: "\n"). */
	lineEnding?: string;
	/** Optional timeout in ms for each serial operation (default: 3000). */
	timeoutMs?: number;
	/**
	 * The command string to send when polling status.
	 * Defaults to "status" — matches ESP32 firmware `status` command.
	 */
	statusCommand?: string;
}

export type BluetoothSerialTransportConfig = SerialTransportConfig;

/**
 * Bluetooth SPP/RFCOMM COM-port contract.
 * This keeps Bluetooth COM paths explicit even if they are line-based serial.
 */
export interface IBluetoothSerialPort {
	writeLine(line: string): Promise<string>;
	readonly isOpen: boolean;
}

export class SerialCommandTransport implements CommandTransport {
	readonly transportType: TransportType = "serial";
	private readonly port: ISerialPort;
	private readonly lineEnding: string;
	private readonly timeoutMs: number;
	private readonly statusCommand: string;

	constructor(port: ISerialPort, config: SerialTransportConfig = {}) {
		this.port = port;
		this.lineEnding = config.lineEnding ?? "\n";
		this.timeoutMs = config.timeoutMs ?? 3000;
		this.statusCommand = config.statusCommand ?? "status";
	}

	private withTimeout<T>(promise: Promise<T>): Promise<T> {
		return new Promise<T>((resolve, reject) => {
			const timer = setTimeout(
				() => reject(new Error("Serial operation timed out")),
				this.timeoutMs,
			);
			promise.then(
				(value) => {
					clearTimeout(timer);
					resolve(value);
				},
				(err) => {
					clearTimeout(timer);
					reject(err);
				},
			);
		});
	}

	async send(command: string): Promise<unknown> {
		if (!this.port.isOpen) throw new Error("Serial port is not open");
		const line = command.endsWith(this.lineEnding) ? command : command + this.lineEnding;
		const raw = await this.withTimeout(this.port.writeLine(line));
		return parseTextResponse(raw);
	}

	async status(): Promise<unknown> {
		return this.send(this.statusCommand);
	}
}

export class BluetoothSerialCommandTransport implements CommandTransport {
	readonly transportType: TransportType = "bluetooth-serial";
	private readonly port: IBluetoothSerialPort;
	private readonly lineEnding: string;
	private readonly timeoutMs: number;
	private readonly statusCommand: string;

	constructor(port: IBluetoothSerialPort, config: BluetoothSerialTransportConfig = {}) {
		this.port = port;
		this.lineEnding = config.lineEnding ?? "\n";
		this.timeoutMs = config.timeoutMs ?? 3000;
		this.statusCommand = config.statusCommand ?? "status";
	}

	private withTimeout<T>(promise: Promise<T>): Promise<T> {
		return new Promise<T>((resolve, reject) => {
			const timer = setTimeout(
				() => reject(new Error("Bluetooth serial operation timed out")),
				this.timeoutMs,
			);
			promise.then(
				(value) => {
					clearTimeout(timer);
					resolve(value);
				},
				(err) => {
					clearTimeout(timer);
					reject(err);
				},
			);
		});
	}

	async send(command: string): Promise<unknown> {
		if (!this.port.isOpen) throw new Error("Bluetooth serial port is not open");
		const line = command.endsWith(this.lineEnding) ? command : command + this.lineEnding;
		const raw = await this.withTimeout(this.port.writeLine(line));
		return parseTextResponse(raw);
	}

	async status(): Promise<unknown> {
		return this.send(this.statusCommand);
	}
}

// ---------------------------------------------------------------------------
// Transport factory
// ---------------------------------------------------------------------------

export type TransportFactoryOptions =
	| { type: "http"; config: HttpTransportConfig }
	| { type: "ble"; peripheral: IBlePeripheral; config?: BleTransportConfig }
	| { type: "serial"; port: ISerialPort; config?: SerialTransportConfig }
	| {
			type: "bluetooth-serial";
			port: IBluetoothSerialPort;
			config?: BluetoothSerialTransportConfig;
	  };

/**
 * Factory that constructs the appropriate transport given options.
 * Prefer `createTransport` over direct construction so callers can swap
 * transport types without modifying controller logic.
 */
export function createTransport(options: TransportFactoryOptions): CommandTransport {
	switch (options.type) {
		case "http":
			return new HttpCommandTransport(options.config);
		case "ble":
			return new BleCommandTransport(options.peripheral, options.config);
		case "serial":
			return new SerialCommandTransport(options.port, options.config);
		case "bluetooth-serial":
			return new BluetoothSerialCommandTransport(options.port, options.config);
	}
}
