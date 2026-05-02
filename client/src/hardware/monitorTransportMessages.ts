import type { MonitorTransportOption, MonitorTransportType } from "./transportPresentation";

export function buildApplyTransportMessage(
	selectedTransportType: MonitorTransportType,
	selectedTransportOption: MonitorTransportOption,
	baseUrl: string,
	commandPath: string,
): string {
	if (selectedTransportType === "http") {
		return `REST transport updated: ${baseUrl}${commandPath}`;
	}

	return `${selectedTransportOption.label} selected. Connect a runtime adapter with ${selectedTransportOption.applyLabel} to activate this monitor transport.`;
}
