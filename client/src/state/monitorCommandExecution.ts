import type { CommandName } from "../hardware/controller";

/**
 * Represents a dispatch action for command lifecycle tracking.
 * Used to update command bus state during command execution.
 */
export type CommandDispatchAction =
  | {
      type: "COMMAND_PENDING";
      payload: {
        id: string;
        command: string;
        startedAt: number;
      };
    }
  | {
      type: "COMMAND_ACKED";
      payload: {
        id: string;
        command: string;
        finishedAt: number;
        detail: string;
      };
    }
  | {
      type: "COMMAND_FAILED";
      payload: {
        id: string;
        finishedAt: number;
        detail: string;
      };
    };

/**
 * Result of a command execution attempt.
 * Contains dispatch actions for lifecycle tracking, display messages, and history updates.
 */
export interface CommandExecutionResult {
  canExecute: boolean;
  blockReason?: string;
  dispatchActions: CommandDispatchAction[];
  displayMessage: string;
  historyEntry?: {
    command: string;
    ok: boolean;
    response: string;
  };
  shouldApplyBoardPayload: boolean;
}

/**
 * Input for resolving command execution readiness.
 * Checks if the command can execute based on transport and command gates.
 */
export interface ResolveCommandExecutionInput {
  transportCanExecute: boolean;
  transportBlockReason?: string;
  commandAvailable: boolean;
  commandBlockReason?: string;
}

/**
 * Resolves whether a command can execute and provides block reason if blocked.
 * Transport gate takes precedence over command gate.
 */
export function resolveCommandExecutionReadiness(
  input: ResolveCommandExecutionInput
): {
  canExecute: boolean;
  blockReason?: string;
} {
  if (!input.transportCanExecute) {
    return {
      canExecute: false,
      blockReason: input.transportBlockReason,
    };
  }

  if (!input.commandAvailable) {
    return {
      canExecute: false,
      blockReason: input.commandBlockReason,
    };
  }

  return {
    canExecute: true,
  };
}

/**
 * Input for building command execution result from command controller response.
 * Captures the full context of what happened during execution.
 */
export interface BuildCommandExecutionResultInput {
  canExecute: boolean;
  blockReason?: string;
  commandName: CommandName;
  rawArgs: string;
  lifecycleId: string;
  startedAt: number;
  controllerResponse: {
    ok: boolean;
    error?: string;
    command?: string;
    responseText?: string;
    responseData?: unknown;
  };
}

/**
 * Builds the complete execution result including dispatch actions and messages.
 * This pure function encapsulates the logic for determining what to do after a command runs.
 */
export function buildCommandExecutionResult(
  input: BuildCommandExecutionResultInput
): CommandExecutionResult {
  // Command cannot execute due to gate
  if (!input.canExecute) {
    return {
      canExecute: false,
      blockReason: input.blockReason,
      dispatchActions: [],
      displayMessage: input.blockReason ?? "Command blocked",
      historyEntry: undefined,
      shouldApplyBoardPayload: false,
    };
  }

  const now = Date.now();

  // Command execution failed
  if (!input.controllerResponse.ok) {
    const errorMessage = input.controllerResponse.error ?? "unknown error";
    return {
      canExecute: true,
      dispatchActions: [
        {
          type: "COMMAND_PENDING",
          payload: {
            id: input.lifecycleId,
            command: input.rawArgs.trim()
              ? `${input.commandName} ${input.rawArgs.trim()}`
              : input.commandName,
            startedAt: input.startedAt,
          },
        },
        {
          type: "COMMAND_FAILED",
          payload: {
            id: input.lifecycleId,
            finishedAt: now,
            detail: errorMessage,
          },
        },
      ],
      displayMessage: `Error: ${errorMessage}`,
      historyEntry: {
        command: input.commandName,
        ok: false,
        response: `Error: ${errorMessage}`,
      },
      shouldApplyBoardPayload: false,
    };
  }

  // Command succeeded
  const responseText = input.controllerResponse.responseText ?? "(no response)";
  return {
    canExecute: true,
    dispatchActions: [
      {
        type: "COMMAND_PENDING",
        payload: {
          id: input.lifecycleId,
          command: input.rawArgs.trim()
            ? `${input.commandName} ${input.rawArgs.trim()}`
            : input.commandName,
          startedAt: input.startedAt,
        },
      },
      {
        type: "COMMAND_ACKED",
        payload: {
          id: input.lifecycleId,
          command: input.controllerResponse.command ?? input.commandName,
          finishedAt: now,
          detail: responseText.slice(0, 120),
        },
      },
    ],
    displayMessage: `Sent: ${input.controllerResponse.command ?? input.commandName}\n${responseText}`,
    historyEntry: {
      command: input.controllerResponse.command ?? input.commandName,
      ok: true,
      response: responseText,
    },
    shouldApplyBoardPayload: input.controllerResponse.responseData !== undefined,
  };
}

/**
 * Input for building quick action execution result.
 * Quick actions are no-arg commands shown as tappable buttons.
 */
export interface BuildQuickActionResultInput {
  canExecute: boolean;
  blockReason?: string;
  commandName: CommandName;
  lifecycleId: string;
  startedAt: number;
  controllerResponse: {
    ok: boolean;
    error?: string;
    command?: string;
    responseText?: string;
    responseData?: unknown;
  };
}

/**
 * Builds execution result for quick action tap.
 * Applies same logic as regular command execution.
 */
export function buildQuickActionResult(
  input: BuildQuickActionResultInput
): CommandExecutionResult {
  return buildCommandExecutionResult({
    canExecute: input.canExecute,
    blockReason: input.blockReason,
    commandName: input.commandName,
    rawArgs: "",
    lifecycleId: input.lifecycleId,
    startedAt: input.startedAt,
    controllerResponse: input.controllerResponse,
  });
}
