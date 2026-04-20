import type { TransportType } from "./transport";

export type MonitorTransportType = Exclude<TransportType, "unsupported">;

export interface MonitorTransportOption {
  id: MonitorTransportType;
  label: string;
  detail: string;
  applyLabel: string;
  supportsInlineConfig: boolean;
}

export interface MonitorTransportStatus {
  tone: "ready" | "pending";
  title: string;
  detail: string;
}

export interface MonitorTransportExecutionPolicy {
  ready: boolean;
  blockReason?: string;
}

function isSerialFamily(type: TransportType | MonitorTransportType): boolean {
  return type === "serial" || type === "bluetooth-serial";
}

export const MONITOR_TRANSPORT_OPTIONS: MonitorTransportOption[] = [
  {
    id: "http",
    label: "REST API",
    detail: "WiFi HTTP bridge using status and command endpoints.",
    applyLabel: "connectViaRestApi(...)",
    supportsInlineConfig: true,
  },
  {
    id: "ble",
    label: "BLE",
    detail: "Direct BLE link through a peripheral adapter.",
    applyLabel: "connectViaBle(...)",
    supportsInlineConfig: false,
  },
  {
    id: "serial",
    label: "Serial / COM",
    detail: "USB serial and OS-exposed Bluetooth COM ports use the same picker flow.",
    applyLabel: "connectViaComPort(...)",
    supportsInlineConfig: false,
  },
  {
    id: "bluetooth-serial",
    label: "Bluetooth COM",
    detail: "Legacy alias of the serial COM transport for Bluetooth SPP/RFCOMM adapters.",
    applyLabel: "connectViaBluetoothComPort(...)",
    supportsInlineConfig: false,
  },
];

export function getMonitorTransportOption(type: MonitorTransportType): MonitorTransportOption {
  return MONITOR_TRANSPORT_OPTIONS.find((option) => option.id === type) ?? MONITOR_TRANSPORT_OPTIONS[0];
}

export function isMonitorTransportReady(
  selected: MonitorTransportType,
  active: TransportType
): boolean {
  if (isSerialFamily(selected) && isSerialFamily(active)) {
    return true;
  }

  return selected === active;
}

export function getMonitorTransportExecutionPolicy(
  selected: MonitorTransportType,
  active: TransportType
): MonitorTransportExecutionPolicy {
  if (isMonitorTransportReady(selected, active)) {
    return { ready: true };
  }

  const option = getMonitorTransportOption(selected);
  return {
    ready: false,
    blockReason: `${option.label} is selected but not active yet. Wire runtime adapter via ${option.applyLabel} or switch back to the active ${active} transport before running commands.`,
  };
}

export function buildMonitorTransportStatus(
  selected: MonitorTransportType,
  active: TransportType,
  baseUrl: string
): MonitorTransportStatus {
  const option = getMonitorTransportOption(selected);

  if (selected === "http" && active === "http") {
    return {
      tone: "ready",
      title: "REST transport active",
      detail: `Monitor commands and status reads will use ${baseUrl}.`,
    };
  }

  if (selected === active) {
    return {
      tone: "ready",
      title: `${option.label} transport active`,
      detail: `Runtime adapter is expected to be connected through ${option.applyLabel}.`,
    };
  }

  return {
    tone: "pending",
    title: `${option.label} selected`,
    detail: `Waiting for runtime adapter wiring via ${option.applyLabel}. Active transport remains ${active}.`,
  };
}
