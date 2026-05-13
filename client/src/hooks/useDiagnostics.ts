import { useEffect, useMemo, useState } from "react";

import {
	buildDiagnosticsEvents,
	filterDiagnosticsEvents,
	type DiagnosticsCategory,
	type DiagnosticsEvent,
} from "../state/monitorDiagnostics";
import {
	loadPersistedDiagnosticsState,
	savePersistedDiagnosticsState,
	type PersistedDiagnosticsArchiveEntry,
} from "../state/monitorDiagnosticsPersistence";

const BUS_FILTERS = ["all", "0", "1", "2"] as const;
type BusFilter = (typeof BUS_FILTERS)[number];

function fmtTime(epochMs: number): string {
	return new Date(epochMs).toLocaleTimeString();
}

export function useDiagnostics({
	commandLifecycle,
	history,
	boardMessages,
	frameSnapshots,
	setHistory,
	setFrameSnapshots,
}: {
	commandLifecycle: Record<string, unknown>;
	history: Array<{ id: string; ts: number; command: string; ok: boolean; response: string }>;
	boardMessages: Array<{ id: number; type: "info" | "error"; text: string; ts: string }>;
	frameSnapshots: Array<{
		id: string;
		ts: number;
		frameCount: number;
		busFilter: string;
		frameFilter: string;
	}>;
	setHistory: (
		value: Array<{ id: string; ts: number; command: string; ok: boolean; response: string }>,
	) => void;
	setFrameSnapshots: (
		value: Array<{
			id: string;
			ts: number;
			frameCount: number;
			busFilter: string;
			frameFilter: string;
		}>,
	) => void;
}) {
	const [diagnosticsQuery, setDiagnosticsQuery] = useState("");
	const [diagnosticsCategory, setDiagnosticsCategory] = useState<DiagnosticsCategory>("all");
	const [diagnosticsArchive, setDiagnosticsArchive] = useState<
		PersistedDiagnosticsArchiveEntry[]
	>([]);
	const [hasHydratedDiagnostics, setHasHydratedDiagnostics] = useState(false);

	useEffect(() => {
		let cancelled = false;

		const hydrateDiagnostics = async () => {
			const persisted = await loadPersistedDiagnosticsState();
			if (!persisted || cancelled) {
				setHasHydratedDiagnostics(true);
				return;
			}

			setHistory(persisted.history);
			setFrameSnapshots(
				persisted.frameSnapshots.map((snapshot) => ({
					...snapshot,
					busFilter: BUS_FILTERS.includes(snapshot.busFilter as BusFilter)
						? (snapshot.busFilter as BusFilter)
						: "all",
				})),
			);
			setDiagnosticsArchive(persisted.archive);
			setHasHydratedDiagnostics(true);
		};

		void hydrateDiagnostics();

		return () => {
			cancelled = true;
		};
	}, [setHistory, setFrameSnapshots]);

	useEffect(() => {
		if (!hasHydratedDiagnostics) {
			return;
		}

		void savePersistedDiagnosticsState({
			history,
			frameSnapshots,
			archive: diagnosticsArchive,
		});
	}, [hasHydratedDiagnostics, history, frameSnapshots, diagnosticsArchive]);

	const diagnosticsEvents = useMemo<DiagnosticsEvent[]>(
		() =>
			buildDiagnosticsEvents({
				commandLifecycle,
				history,
				boardMessages,
				frameSnapshots,
				formatTime: fmtTime,
			}),
		[commandLifecycle, history, boardMessages, frameSnapshots],
	);

	const archivedDiagnosticsEvents = useMemo<DiagnosticsEvent[]>(
		() =>
			diagnosticsArchive.map((entry) => ({
				id: `archive-${entry.id}`,
				ts: entry.ts,
				tsLabel: fmtTime(entry.ts),
				category: entry.category,
				summary: entry.summary,
				detail: entry.detail,
				ok: entry.ok,
			})),
		[diagnosticsArchive],
	);

	const visibleDiagnostics = useMemo(
		() =>
			filterDiagnosticsEvents({
				events: [...diagnosticsEvents, ...archivedDiagnosticsEvents],
				query: diagnosticsQuery,
				category: diagnosticsCategory,
			}),
		[diagnosticsEvents, archivedDiagnosticsEvents, diagnosticsQuery, diagnosticsCategory],
	);

	const pushDiagnosticsArchive = (
		category: PersistedDiagnosticsArchiveEntry["category"],
		summary: string,
		detail: string,
		ok = true,
	) => {
		setDiagnosticsArchive((current) =>
			[
				{
					id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
					ts: Date.now(),
					category,
					summary,
					detail,
					ok,
				},
				...current,
			].slice(0, 400),
		);
	};

	return {
		diagnosticsQuery,
		setDiagnosticsQuery,
		diagnosticsCategory,
		setDiagnosticsCategory,
		diagnosticsArchive,
		pushDiagnosticsArchive,
		hasHydratedDiagnostics,
		visibleDiagnostics,
	};
}
