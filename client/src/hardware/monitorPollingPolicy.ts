export interface AutoPollPolicyInput {
  autoPoll: boolean;
  canFetchStatus: boolean;
  pollSeconds: number;
  blockReason?: string;
}

export type AutoPollPolicy =
  | { action: "idle" }
  | { action: "disable"; reason: string }
  | { action: "start"; everySeconds: number };

export interface MonitorTransportGateSnapshot {
  canExecuteCommands: boolean;
  canFetchStatus: boolean;
  commandBlockReason: string | null;
  statusBlockReason: string | null;
}

export function getStatusPollingBlockReason(
  canFetchStatus: boolean,
  blockReason?: string
): string | null {
  if (canFetchStatus) {
    return null;
  }
  return blockReason ?? "Selected transport is not ready for status polling.";
}

export function getCommandExecutionBlockReason(
  canExecute: boolean,
  blockReason?: string
): string | null {
  if (canExecute) {
    return null;
  }
  return blockReason ?? "Selected transport is not ready for command execution.";
}

export function buildMonitorTransportGateSnapshot(
  isTransportReady: boolean,
  blockReason?: string
): MonitorTransportGateSnapshot {
  return {
    canExecuteCommands: isTransportReady,
    canFetchStatus: isTransportReady,
    commandBlockReason: getCommandExecutionBlockReason(isTransportReady, blockReason),
    statusBlockReason: getStatusPollingBlockReason(isTransportReady, blockReason),
  };
}

export function getAutoPollPolicy(input: AutoPollPolicyInput): AutoPollPolicy {
  if (!input.autoPoll) {
    return { action: "idle" };
  }

  if (!input.canFetchStatus) {
    return {
      action: "disable",
      reason:
        input.blockReason ??
        "Auto poll disabled because selected transport is not active.",
    };
  }

  if (input.pollSeconds <= 0) {
    return { action: "idle" };
  }

  return { action: "start", everySeconds: input.pollSeconds };
}
