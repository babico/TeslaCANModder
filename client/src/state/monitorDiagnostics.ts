import type { ConsoleMessage } from "@teslacanmodder/protocol";
import type { CommandLifecycleEntry } from "./commandBus";

export type DiagnosticsCategory = "all" | "command" | "board" | "snapshot" | "system";

export interface DiagnosticsEvent {
  id: string;
  ts: number;
  tsLabel: string;
  category: Exclude<DiagnosticsCategory, "all">;
  summary: string;
  detail: string;
  ok: boolean;
}

export interface DiagnosticsHistoryEntry {
  id: string;
  ts: number;
  command: string;
  ok: boolean;
  response: string;
}

export interface DiagnosticsFrameSnapshot {
  id: string;
  ts: number;
  frameCount: number;
  busFilter: string;
  frameFilter: string;
}

export interface BuildDiagnosticsEventsInput {
  commandLifecycle: CommandLifecycleEntry[];
  history: DiagnosticsHistoryEntry[];
  boardMessages: ConsoleMessage[];
  frameSnapshots: DiagnosticsFrameSnapshot[];
  formatTime: (epochMs: number) => string;
  nowMs?: number;
  maxEvents?: number;
}

export interface FilterDiagnosticsEventsInput {
  events: DiagnosticsEvent[];
  query: string;
  category: DiagnosticsCategory;
}

export function buildDiagnosticsEvents(input: BuildDiagnosticsEventsInput): DiagnosticsEvent[] {
  const nowMs = input.nowMs ?? Date.now();
  const maxEvents = input.maxEvents ?? 80;

  const commandLifecycleEvents: DiagnosticsEvent[] = input.commandLifecycle.map((entry) => {
    const done = entry.finishedAt !== undefined;
    const elapsed = done
      ? `${(entry.finishedAt! - entry.startedAt).toFixed(0)}ms`
      : "in-flight";

    return {
      id: `lifecycle-${entry.id}`,
      ts: entry.finishedAt ?? entry.startedAt,
      tsLabel: input.formatTime(entry.finishedAt ?? entry.startedAt),
      category: "command",
      summary: `${entry.command} · ${entry.status.toUpperCase()}`,
      detail: `${elapsed}${entry.detail ? ` · ${entry.detail}` : ""}`,
      ok: entry.status !== "failed",
    };
  });

  const commandHistoryEvents: DiagnosticsEvent[] = input.history.map((entry) => ({
    id: `history-${entry.id}`,
    ts: entry.ts,
    tsLabel: input.formatTime(entry.ts),
    category: "command",
    summary: `Result · ${entry.command}`,
    detail: entry.response,
    ok: entry.ok,
  }));

  const boardEvents: DiagnosticsEvent[] = input.boardMessages.map((message, idx) => ({
    id: `board-${message.id}-${idx}`,
    ts: nowMs - idx,
    tsLabel: message.ts,
    category: "board",
    summary: message.text,
    detail: message.type.toUpperCase(),
    ok: message.type !== "error",
  }));

  const snapshotEvents: DiagnosticsEvent[] = input.frameSnapshots.map((snapshot) => ({
    id: `snap-${snapshot.id}`,
    ts: snapshot.ts,
    tsLabel: input.formatTime(snapshot.ts),
    category: "snapshot",
    summary: `${snapshot.frameCount} frames captured`,
    detail: `bus=${snapshot.busFilter}${snapshot.frameFilter ? ` filter=${snapshot.frameFilter}` : ""}`,
    ok: true,
  }));

  return [
    ...commandLifecycleEvents,
    ...commandHistoryEvents,
    ...boardEvents,
    ...snapshotEvents,
  ]
    .sort((a, b) => b.ts - a.ts)
    .slice(0, maxEvents);
}

export function filterDiagnosticsEvents(
  input: FilterDiagnosticsEventsInput
): DiagnosticsEvent[] {
  const query = input.query.trim().toLowerCase();

  return input.events.filter((event) => {
    if (input.category !== "all" && event.category !== input.category) {
      return false;
    }

    if (!query) {
      return true;
    }

    const haystack = `${event.summary} ${event.detail} ${event.category}`.toLowerCase();
    return haystack.includes(query);
  });
}
