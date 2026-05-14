/**
 * BoardStateContext — smoke tests
 */

jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {
		frames: [],
		messages: [],
		frameCount: 0,
		features: {},
		canHealth: {},
	},
	BUS_NAMES: {},
}));

import React from "react";
import { render } from "@testing-library/react-native";
import { Text } from "react-native";

const mockController = {
	readStatus: jest.fn(async () => ({ raw: {} })),
	activeTransportType: "http",
} as any;

import { BoardStateProvider, useBoardInstanceState } from "../../src/state/BoardStateContext";

function TestConsumer() {
	const { boardState } = useBoardInstanceState();
	return <Text testID="frameCount">{String(boardState.frameCount)}</Text>;
}

describe("BoardStateContext", () => {
	it("initial boardState has frameCount = 0", () => {
		const { getByTestId } = render(
			<BoardStateProvider controller={mockController}>
				<TestConsumer />
			</BoardStateProvider>,
		);
		expect(getByTestId("frameCount").props.children).toBe("0");
	});

	it("initial statusText is set", () => {
		function TextConsumer() {
			const { statusText } = useBoardInstanceState();
			return <Text testID="st">{statusText}</Text>;
		}
		const { getByTestId } = render(
			<BoardStateProvider controller={mockController}>
				<TextConsumer />
			</BoardStateProvider>,
		);
		expect(getByTestId("st").props.children).toBeTruthy();
	});
});
