export interface MonitorControlStateInput {
  selectedCommandAvailable: boolean;
  selectedCommandReason?: string | null;
  transportCanExecuteCommands: boolean;
  transportCanFetchStatus: boolean;
  transportCommandBlockReason?: string | null;
  transportStatusBlockReason?: string | null;
}

export interface MonitorControlState {
  canFetchStatus: boolean;
  canRunTransportCommand: boolean;
  canRunSelectedCommand: boolean;
  selectedCommandWarning: string | null;
  transportCommandWarning: string | null;
  autoPollWarning: string | null;
}

export interface QuickActionBlockReasonInput {
  commandAvailable: boolean;
  commandReason?: string | null;
  transportCanExecuteCommands: boolean;
  transportCommandBlockReason?: string | null;
}

export function resolveQuickActionBlockReason(
  input: QuickActionBlockReasonInput
): string | null {
  if (!input.commandAvailable) {
    return input.commandReason ?? "Command is currently unavailable.";
  }

  if (!input.transportCanExecuteCommands) {
    return (
      input.transportCommandBlockReason ??
      "Selected transport is not ready for command execution."
    );
  }

  return null;
}

export function buildMonitorControlState(
  input: MonitorControlStateInput
): MonitorControlState {
  const selectedCommandWarning =
    !input.selectedCommandAvailable && input.selectedCommandReason
      ? `⊘ ${input.selectedCommandReason}`
      : null;

  const transportCommandWarning = !input.transportCanExecuteCommands
    ? `⊘ ${
        input.transportCommandBlockReason ??
        "Selected transport is not ready for command execution."
      }`
    : null;

  const autoPollWarning = !input.transportCanFetchStatus
    ? `⊘ ${
        input.transportStatusBlockReason ??
        "Auto polling is available only when selected transport is active."
      }`
    : null;

  return {
    canFetchStatus: input.transportCanFetchStatus,
    canRunTransportCommand: input.transportCanExecuteCommands,
    canRunSelectedCommand:
      input.selectedCommandAvailable && input.transportCanExecuteCommands,
    selectedCommandWarning,
    transportCommandWarning,
    autoPollWarning,
  };
}
