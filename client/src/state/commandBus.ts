/**
 * Unified Message Bus — Command Lifecycle Tracking (B-04)
 *
 * Provides a pure reducer + action types for tracking command
 * pending / acked / failed lifecycle events in a shared, testable module.
 *
 * Usage:
 *   const [busState, dispatch] = useReducer(commandBusReducer, initialCommandBusState());
 */

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

export type CommandLifecycleStatus = "pending" | "acked" | "failed";

export interface CommandLifecycleEntry {
  /** Stable unique identifier for this command invocation. */
  id: string;
  /** Human-readable command label (e.g. "fsdOn" or "setSpeed 90"). */
  command: string;
  /** Current lifecycle phase. */
  status: CommandLifecycleStatus;
  /** Unix-ms timestamp when the command was dispatched. */
  startedAt: number;
  /** Unix-ms timestamp when the command settled (acked or failed). */
  finishedAt?: number;
  /** Short detail string — response snippet or error message. */
  detail?: string;
}

export interface CommandBusState {
  /** Active + recent command lifecycle entries (newest first). */
  entries: CommandLifecycleEntry[];
  /**
   * Total commands dispatched in this session.
   * Monotonically increasing; never reset on trim.
   */
  totalDispatched: number;
  /** Total acked commands in this session. */
  totalAcked: number;
  /** Total failed commands in this session. */
  totalFailed: number;
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

export type CommandBusAction =
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
        /** Final built command string returned by the transport. */
        command?: string;
        finishedAt: number;
        detail?: string;
      };
    }
  | {
      type: "COMMAND_FAILED";
      payload: {
        id: string;
        finishedAt: number;
        detail?: string;
      };
    }
  | {
      /** Remove all settled entries older than `olderThanMs` (default: purge all). */
      type: "PURGE_SETTLED";
      payload?: { olderThanMs?: number };
    };

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

/** Maximum number of entries kept in the state. Oldest entries are trimmed. */
export const COMMAND_BUS_MAX_ENTRIES = 40;

// ---------------------------------------------------------------------------
// Reducer
// ---------------------------------------------------------------------------

export function initialCommandBusState(): CommandBusState {
  return {
    entries: [],
    totalDispatched: 0,
    totalAcked: 0,
    totalFailed: 0,
  };
}

export function commandBusReducer(
  state: CommandBusState,
  action: CommandBusAction
): CommandBusState {
  switch (action.type) {
    case "COMMAND_PENDING": {
      const entry: CommandLifecycleEntry = {
        id: action.payload.id,
        command: action.payload.command,
        status: "pending",
        startedAt: action.payload.startedAt,
      };
      return {
        ...state,
        entries: [entry, ...state.entries].slice(0, COMMAND_BUS_MAX_ENTRIES),
        totalDispatched: state.totalDispatched + 1,
      };
    }

    case "COMMAND_ACKED": {
      return {
        ...state,
        entries: state.entries.map((e) =>
          e.id === action.payload.id
            ? {
                ...e,
                command: action.payload.command ?? e.command,
                status: "acked" as const,
                finishedAt: action.payload.finishedAt,
                detail: action.payload.detail,
              }
            : e
        ),
        totalAcked: state.totalAcked + 1,
      };
    }

    case "COMMAND_FAILED": {
      return {
        ...state,
        entries: state.entries.map((e) =>
          e.id === action.payload.id
            ? {
                ...e,
                status: "failed" as const,
                finishedAt: action.payload.finishedAt,
                detail: action.payload.detail,
              }
            : e
        ),
        totalFailed: state.totalFailed + 1,
      };
    }

    case "PURGE_SETTLED": {
      const cutoff = action.payload?.olderThanMs;
      const now = Date.now();
      return {
        ...state,
        entries: state.entries.filter((e) => {
          if (e.status === "pending") return true; // never purge in-flight
          if (cutoff == null) return false;         // purge all settled
          return e.finishedAt != null && now - e.finishedAt < cutoff;
        }),
      };
    }

    default:
      return state;
  }
}

// ---------------------------------------------------------------------------
// Selectors
// ---------------------------------------------------------------------------

/** Returns entries currently in "pending" state. */
export function selectPendingCommands(
  state: CommandBusState
): CommandLifecycleEntry[] {
  return state.entries.filter((e) => e.status === "pending");
}

/** Returns entries that have settled (acked or failed). */
export function selectSettledCommands(
  state: CommandBusState
): CommandLifecycleEntry[] {
  return state.entries.filter((e) => e.status !== "pending");
}

/** Returns the most recent entry, regardless of status. */
export function selectLastCommand(
  state: CommandBusState
): CommandLifecycleEntry | undefined {
  return state.entries[0];
}

/**
 * Returns aggregate success rate (0–1) for settled commands, or null
 * if no commands have settled yet.
 */
export function selectSuccessRate(state: CommandBusState): number | null {
  const settled = state.totalAcked + state.totalFailed;
  if (settled === 0) return null;
  return state.totalAcked / settled;
}

// ---------------------------------------------------------------------------
// ID generator
// ---------------------------------------------------------------------------

/** Generates a stable unique ID for a command invocation. */
export function generateCommandId(): string {
  return `${Date.now()}-${Math.random().toString(16).slice(2, 10)}`;
}
