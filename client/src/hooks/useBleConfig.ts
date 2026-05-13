import { useState } from "react";

import type { PersistedDiagnosticsArchiveEntry } from "../state/monitorDiagnosticsPersistence";

export function useBleConfig({
	baseUrl,
	onLog,
	onSetLastResult,
}: {
	baseUrl: string;
	onLog: (
		category: PersistedDiagnosticsArchiveEntry["category"],
		summary: string,
		detail: string,
		ok?: boolean,
	) => void;
	onSetLastResult: (message: string) => void;
}) {
	const [bleDeviceName, setBleDeviceName] = useState("TeslaCANModder");
	const [bleConfigBusy, setBleConfigBusy] = useState(false);

	const refreshBleStatus = async () => {
		const trimmedBase = baseUrl.trim().replace(/\/+$/, "");
		if (!trimmedBase) {
			const message = "Base URL is required to read BLE status.";
			onLog("system", "BLE status failed", message, false);
			onSetLastResult(message);
			return;
		}

		setBleConfigBusy(true);
		try {
			const response = await fetch(`${trimmedBase}/api/status`);
			const raw = await response.text();
			let payload: Record<string, unknown> | null = null;
			try {
				payload = JSON.parse(raw) as Record<string, unknown>;
			} catch {
				payload = null;
			}

			if (!response.ok) {
				throw new Error(
					(payload?.error as string) || raw || `BLE status failed (${response.status})`,
				);
			}

			const ble = (payload?.ble ?? {}) as Record<string, unknown>;

			if (typeof ble.deviceName === "string") {
				setBleDeviceName(ble.deviceName);
			}

			const enabled = ble.enabled === true ? "on" : "off";
			const connected = ble.connected === true ? "yes" : "no";
			onLog("system", "BLE status fetched", `enabled=${enabled} connected=${connected}`);
			onSetLastResult(`BLE status loaded (enabled=${enabled}, connected=${connected}).`);
		} catch (error) {
			const message = error instanceof Error ? error.message : "Failed to read BLE status.";
			onLog("system", "BLE status failed", message, false);
			onSetLastResult(message);
		} finally {
			setBleConfigBusy(false);
		}
	};

	const applyBleDeviceName = async () => {
		const trimmedBase = baseUrl.trim().replace(/\/+$/, "");
		const nextName = bleDeviceName.trim();

		if (!trimmedBase) {
			const message = "Base URL is required to update BLE name.";
			onLog("system", "BLE name update failed", message, false);
			onSetLastResult(message);
			return;
		}

		if (!nextName || nextName.length > 32) {
			const message = "BLE name must be between 1 and 32 characters.";
			onLog("system", "BLE name update failed", message, false);
			onSetLastResult(message);
			return;
		}

		setBleConfigBusy(true);
		try {
			const response = await fetch(`${trimmedBase}/api/command`, {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify({ cmd: `ble:name:${nextName}` }),
			});

			const raw = await response.text();
			let payload: Record<string, unknown> | null = null;
			try {
				payload = JSON.parse(raw) as Record<string, unknown>;
			} catch {
				payload = null;
			}

			if (!response.ok) {
				throw new Error(
					(payload?.error as string) ||
						raw ||
						`BLE name update failed (${response.status})`,
				);
			}

			// /api/command returns an Ack; re-read /api/status to surface the updated name.
			try {
				const statusRes = await fetch(`${trimmedBase}/api/status`);
				if (statusRes.ok) {
					const sd = (await statusRes.json()) as Record<string, unknown>;
					const ble = (sd?.ble ?? {}) as Record<string, unknown>;
					if (typeof ble.deviceName === "string") {
						setBleDeviceName(ble.deviceName);
					}
				}
			} catch {
				// best-effort refresh; ignore failures
			}

			onLog("system", "BLE name updated", `name=${nextName}`);
			onSetLastResult(`BLE name updated to "${nextName}".`);
		} catch (error) {
			const message = error instanceof Error ? error.message : "Failed to update BLE name.";
			onLog("system", "BLE name update failed", message, false);
			onSetLastResult(message);
		} finally {
			setBleConfigBusy(false);
		}
	};

	return {
		bleDeviceName,
		setBleDeviceName,
		bleConfigBusy,
		refreshBleStatus,
		applyBleDeviceName,
	};
}
