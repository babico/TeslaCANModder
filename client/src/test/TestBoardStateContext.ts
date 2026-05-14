import { createContext } from "react";

export interface TestBoardStateCtx {
	boardState: Record<string, unknown>;
}

export const TestBoardStateContext = createContext<TestBoardStateCtx>({
	boardState: {},
});
