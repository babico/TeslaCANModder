/**
 * BoardStateContext
 *
 * Board data, status text, status polling, and frame feed controls.
 * Extracted from BoardConnectionContext (Phase B refactor).
 */

import { createContext, useCallback, useContext, useRef, useState, type ReactNode } from "react";

import { initialBoardState, type BoardState } from "@teslacanmodder/protocol";

import { reduceBoardPayload } from "./board";
import { HardwareController } from "../hardware/controller";

// ── Types ─────────────────────────────────────────────────────────────────────

export interface BoardInstanceState {
	boardState: BoardState;
	statusText: string;
	lastResult: string;
}

export interface BoardInstanceActions {
	fetchStatus: (canFetchStatus: boolean) => Promise<void>;
	pauseFrameFeed: (paused: boolean) => void;
	clearFrames: () => void;
	applyBoardPayload: (raw: unknown) => void;
	setLastResult: (text: string) => void;
	setStatusText: (text: string) => void;
}

// ── Hooks ──────────────────────────────────────────────────────────────────────

export function useBoardInstanceState(): BoardInstanceState {
	const ctx = useContext(_BoardStateContext);
	if (!ctx) {
		throw new Error("useBoardInstanceState must be used inside <BoardStateProvider>");
	}
	return ctx.state;
}

export function useBoardInstanceActions(): BoardInstanceActions {
	const ctx = useContext(_BoardStateContext);
	if (!ctx) {
		throw new Error("useBoardInstanceActions must be used inside <BoardStateProvider>");
	}
	return ctx.actions;
}

// ── Context internals ──────────────────────────────────────────────────────────

interface BoardStateContextValue {
	state: BoardInstanceState;
	actions: BoardInstanceActions;
}

const _BoardStateContext = createContext<BoardStateContextValue | null>(null);

// ── Provider ───────────────────────────────────────────────────────────────────

export function BoardStateProvider({
	children,
	controller,
}: {
	children: ReactNode;
	controller: HardwareController;
}) {
	const [boardState, setBoardState] = useState<BoardState>(initialBoardState);
	const [frameFeedPaused, setFrameFeedPaused] = useState(false);
	const [statusText, setStatusText] = useState("No status fetched");
	const [lastResult, setLastResult] = useState("Ready");
	const nextMessageId = useRef(1);

	// ── Board payload reducer ─────────────────────────────────────────────────

	const applyBoardPayload = useCallback(
		(raw: unknown) => {
			setBoardState((current) => {
				const next = reduceBoardPayload(current, raw, () => nextMessageId.current++);
				if (!next) return current;
				if (frameFeedPaused) {
					return { ...next, frames: current.frames, frameCount: current.frameCount };
				}
				return next;
			});
		},
		[frameFeedPaused],
	);

	// ── Fetch status ────────────────────────────────────────────────────────────

	const fetchStatus = useCallback(
		async (canFetchStatus: boolean) => {
			if (!canFetchStatus) {
				setStatusText("Transport not ready for status fetch");
				return;
			}
			try {
				const status = await controller.readStatus();
				setStatusText(JSON.stringify(status, null, 2));
				if (status) applyBoardPayload(status);
			} catch (err) {
				setStatusText(`Error: ${String(err)}`);
			}
		},
		[controller, applyBoardPayload],
	);

	// ── Frame feed controls ─────────────────────────────────────────────────────

	const pauseFrameFeed = useCallback((paused: boolean) => {
		setFrameFeedPaused(paused);
	}, []);

	const clearFrames = useCallback(() => {
		setBoardState((current) => ({ ...current, frames: [], frameCount: 0 }));
	}, []);

	const value: BoardStateContextValue = {
		state: {
			boardState,
			statusText,
			lastResult,
		},
		actions: {
			fetchStatus,
			pauseFrameFeed,
			clearFrames,
			applyBoardPayload,
			setLastResult,
			setStatusText,
		},
	};

	return <_BoardStateContext.Provider value={value}>{children}</_BoardStateContext.Provider>;
}
