/**
 * CommandContext — smoke tests
 */

jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {},
	BUS_NAMES: {},
}));

import React from "react";
import { render } from "@testing-library/react-native";
import { Text } from "react-native";

const mockController = {
	runCommand: jest.fn(async () => ({ ok: true, command: "ping", responseText: "pong" })),
	activeTransportType: "http",
} as any;

import {
	CommandProvider,
	useCommandState,
	useCommandActions,
} from "../../src/state/CommandContext";

function TestConsumer() {
	const { commandLifecycle: _historyLen, history } = useCommandState();
	return <Text testID="historyLen">{String(history.length)}</Text>;
}

describe("CommandContext", () => {
	it("initial history is empty", () => {
		const { getByTestId } = render(
			<CommandProvider
				controller={mockController}
				boardState={{} as any}
				selectedTransportType="http"
				applyBoardPayload={jest.fn()}
				setLastResult={jest.fn()}
			>
				<TestConsumer />
			</CommandProvider>,
		);
		expect(getByTestId("historyLen").props.children).toBe("0");
	});

	it("exposes runCommand action", () => {
		const actions: { current: any } = { current: null };
		function Probe() {
			const a = useCommandActions();
			actions.current = a;
			return null;
		}

		render(
			<CommandProvider
				controller={mockController}
				boardState={{} as any}
				selectedTransportType="http"
				applyBoardPayload={jest.fn()}
				setLastResult={jest.fn()}
			>
				<Probe />
			</CommandProvider>,
		);

		expect(typeof actions.current.runCommand).toBe("function");
	});
});
